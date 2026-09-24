#!/usr/bin/env python3
"""
copy_and_patch_all.py

Copies ALL files from a source directory into the directory where this script lives,
preserving subdirectories. While copying:

1) For any .py file:
   - If it contains: manager = uhal.ConnectionManager("file://connections.xml")
     replace that line with a robust version that resolves connections.xml relative
     to the script location, and ensure 'import os' exists.

2) For dtc_sim_test_new.py specifically:
   - Replace specific hw.getNode(...).write(0x...) hard-coded register writes with
     values loaded from $STMDAQ_ROOT/config/xml/firmware.xml under:
         <fw><use_dtc_sim><tag>VALUE</tag>...</use_dtc_sim></fw>
   - Uses an explicit HW-key -> XML-tag mapping so XML tag names can be valid.
     Example: "10g_readout_chan1" -> "readout_chan1".
   - Ensures required imports exist.

Compatibility: Python 3.8+ (no PEP604 unions; uses typing.Optional/List/Dict).
"""

import argparse
import os
import re
import shutil
import sys
from typing import Dict, List, Optional, Tuple

# -----------------------------
# uHAL connections.xml patching
# -----------------------------

UHAL_LINE_RE = re.compile(
    r'^\s*manager\s*=\s*uhal\.ConnectionManager\(\s*([\'"])file://connections\.xml\1\s*\)\s*$'
)

UHAL_REPLACEMENT_LINES = [
    'here = os.path.dirname(os.path.abspath(__file__))',
    'xml = os.path.join(here, "connections.xml")',
    'manager = uhal.ConnectionManager("file://" + xml)',
]


# ---------------------------------
# dtc_sim_test_new.py special patch
# ---------------------------------

DTC_SIM_SCRIPT_NAME = "dtc_sim_test_new.py"

# HW keys: these are the suffixes of the hardware node names we patch
DTC_HW_KEYS: List[str] = [
    "teng_interpacket_pause",
    "ext_trig_timeout",
    "10g_readout_chan1",
    "10g_readout_chan2",
    "deltahb",
    "numberhb",
    "actual_adc_enable",
]

# Map HW key -> XML tag name under <fw><use_dtc_sim><xml_tag>...</xml_tag>
# This also handles invalid XML tag names that would start with a digit.
DTC_KEY_MAP: Dict[str, str] = {
    "teng_interpacket_pause": "teng_interpacket_pause",
    "ext_trig_timeout": "ext_trig_timeout",
    "10g_readout_chan1": "readout_chan1",
    "10g_readout_chan2": "readout_chan2",
    "deltahb": "deltahb",
    "numberhb": "numberhb",
    "actual_adc_enable": "actual_adc_enable",
}

# Match lines like:
#   hw.getNode("Buffers.Readout_regs.teng_interpacket_pause").write(0x40)
# with flexible whitespace, quotes, numeric literal, and optional trailing comment.
DTC_WRITE_RE = re.compile(
    r'^(?P<indent>\s*)hw\.getNode\(\s*(?P<q>[\'"])(?P<node>[^\'"]+)\2\s*\)\.write\(\s*'
    r'(?P<val>0x[0-9A-Fa-f]+|\d+)\s*\)\s*(?P<comment>#.*)?$'
)


# ------------
# Import helpers
# ------------

def ensure_import(lines: List[str], module: str, alias: Optional[str] = None) -> List[str]:
    """
    Ensure an import exists near the top of file in a safe location.

    - module="os" -> "import os"
    - module="xml.etree.ElementTree", alias="ET" -> "import xml.etree.ElementTree as ET"
    """
    if alias:
        # exact "import module as alias"
        import_re = re.compile(
            r'^\s*import\s+' + re.escape(module) + r'\s+as\s+' + re.escape(alias) + r'(\s|$)'
        )
        stmt = "import {} as {}\n".format(module, alias)
    else:
        import_re = re.compile(r'^\s*import\s+' + re.escape(module) + r'(\s|,|$)')
        stmt = "import {}\n".format(module)

    from_import_re = re.compile(r'^\s*from\s+' + re.escape(module) + r'\s+import\s+')

    for ln in lines:
        if import_re.match(ln) or from_import_re.match(ln):
            return lines

    i = 0

    # Shebang
    if i < len(lines) and lines[i].startswith("#!"):
        i += 1

    # Encoding cookie (PEP 263) in first two lines
    if i < len(lines) and re.search(r"coding[:=]\s*[-\w.]+", lines[i]):
        i += 1
    elif i + 1 < len(lines) and re.search(r"coding[:=]\s*[-\w.]+", lines[i + 1]):
        i += 2

    # Skip blank lines
    while i < len(lines) and lines[i].strip() == "":
        i += 1

    # Module docstring (best-effort)
    if i < len(lines) and re.match(r'^\s*(["\']{3})', lines[i]):
        quote = re.match(r'^\s*(["\']{3})', lines[i]).group(1)
        i += 1
        while i < len(lines) and quote not in lines[i]:
            i += 1
        if i < len(lines):
            i += 1
        while i < len(lines) and lines[i].strip() == "":
            i += 1

    # __future__ imports must remain at top
    while i < len(lines) and re.match(r'^\s*from\s+__future__\s+import\s+', lines[i]):
        i += 1

    return lines[:i] + [stmt] + lines[i:]


# -----------------------
# uHAL manager line patch
# -----------------------

def patch_uhal_manager(lines: List[str]) -> Tuple[List[str], bool]:
    changed = False
    new_lines: List[str] = []

    joined = "".join(lines)
    already_has_block = all(s in joined for s in UHAL_REPLACEMENT_LINES)

    for ln in lines:
        if UHAL_LINE_RE.match(ln) and not already_has_block:
            indent = re.match(r'^(\s*)', ln).group(1)
            for r in UHAL_REPLACEMENT_LINES:
                new_lines.append(indent + r + "\n")
            changed = True
        else:
            new_lines.append(ln)

    if changed:
        new_lines = ensure_import(new_lines, "os")

    return new_lines, changed


# --------------------------------------
# dtc_sim_test_new.py register patching
# --------------------------------------

def _dtc_xml_loader_block(indent: str) -> List[str]:
    """
    Code inserted into dtc_sim_test_new.py before the first patched write.

    Assumes firmware.xml structure:
      <fw>
        <use_dtc_sim>
          <teng_interpacket_pause>0x40</teng_interpacket_pause>
          ...
        </use_dtc_sim>
      </fw>

    Builds dtc_regs dict keyed by HW key (e.g. "10g_readout_chan1") using DTC_KEY_MAP.
    """
    block: List[str] = []

    block.append(indent + "# --- Load DTC sim register values from firmware.xml ---\n")
    block.append(indent + "stmdaq_root = os.environ.get(\"STMDAQ_ROOT\")\n")
    block.append(indent + "if not stmdaq_root:\n")
    block.append(indent + "    raise RuntimeError(\"STMDAQ_ROOT is not set in the environment\")\n")
    block.append(indent + "fw_xml = os.path.join(stmdaq_root, \"config\", \"xml\", \"firmware.xml\")\n")
    block.append(indent + "root = ET.parse(fw_xml).getroot()\n")
    block.append(indent + "\n")

    block.append(indent + "use = root.find(\".//use_dtc_sim\")\n")
    block.append(indent + "if use is None:\n")
    block.append(indent + "    raise KeyError(\"Could not find <use_dtc_sim> in {}\".format(fw_xml))\n")
    block.append(indent + "\n")

    block.append(indent + "def _fw_use_dtc_sim_value(xml_name: str) -> int:\n")
    block.append(indent + "    el = use.find(xml_name)\n")
    block.append(indent + "    if el is None or el.text is None or not el.text.strip():\n")
    block.append(indent + "        raise KeyError(\"Missing <{}> under <use_dtc_sim> in {}\".format(xml_name, fw_xml))\n")
    block.append(indent + "    return int(el.text.strip(), 0)\n")
    block.append(indent + "\n")

    block.append(indent + "DTC_KEY_MAP = {\n")
    for hw_key, xml_key in DTC_KEY_MAP.items():
        block.append(indent + "    \"{}\": \"{}\",\n".format(hw_key, xml_key))
    block.append(indent + "}\n\n")

    block.append(indent + "dtc_regs = {}\n")
    block.append(indent + "for hw_key, xml_key in DTC_KEY_MAP.items():\n")
    block.append(indent + "    dtc_regs[hw_key] = _fw_use_dtc_sim_value(xml_key)\n")
    block.append(indent + "# --- End firmware.xml load ---\n\n")

    return block


def patch_dtc_sim_registers(lines: List[str]) -> Tuple[List[str], bool]:
    """
    For dtc_sim_test_new.py only:
      - Replace specific hw.getNode(...<key>).write(<hardcoded>) with write(dtc_regs["key"])
      - Insert firmware.xml loader block once before the first replaced line
      - Ensure required imports exist at top: os and xml.etree.ElementTree as ET
    """
    out: List[str] = []
    changed = False
    inserted_loader = False

    for ln in lines:
        m = DTC_WRITE_RE.match(ln)
        if m:
            node = m.group("node")
            indent = m.group("indent")
            comment = m.group("comment") or ""

            key = node.split(".")[-1]  # last suffix
            if key in DTC_HW_KEYS:
                if not inserted_loader:
                    out = ensure_import(out, "os")
                    out = ensure_import(out, "xml.etree.ElementTree", alias="ET")
                    out.extend(_dtc_xml_loader_block(indent))
                    inserted_loader = True

                new_line = indent + 'hw.getNode("{}").write(dtc_regs["{}"])'.format(node, key)
                if comment:
                    new_line += " " + comment.strip()
                out.append(new_line + "\n")
                changed = True
                continue

        out.append(ln)

    return out, changed


def ensure_finished_true_at_end(lines: List[str]) -> List[str]:
    """
    Ensure the file ends with:
        finished = True
    (with a trailing newline).
    """
    # Strip trailing whitespace-only lines
    while lines and lines[-1].strip() == "":
        lines.pop()

    # If already present as the last meaningful line, do nothing
    if lines and lines[-1].strip() == "finished = True":
        return lines + ["\n"]

    return lines + ["\n", "finished = True\n"]

# -------------------------
# File copying / processing
# -------------------------

def patch_python_file(src: str, dst: str) -> Tuple[bool, bool]:
    """
    Patch python files:
      - uHAL manager line patch (connections.xml)
      - dtc_sim_test_new.py register patch (special)
    Returns (changed_anything, changed_dtc_special)
    """
    with open(src, "r", encoding="utf-8", errors="replace") as f:
        lines = f.readlines()

    changed_any = False
    changed_dtc = False

    lines, ch = patch_uhal_manager(lines)
    changed_any = changed_any or ch

    if os.path.basename(src) == DTC_SIM_SCRIPT_NAME:
        lines, ch2 = patch_dtc_sim_registers(lines)
        lines = ensure_finished_true_at_end(lines)
        changed_dtc = ch2
        changed_any = True  # we *did* modify the file        
        
    os.makedirs(os.path.dirname(dst), exist_ok=True)
    with open(dst, "w", encoding="utf-8", newline="") as f:
        f.writelines(lines)

    return changed_any, changed_dtc


def main() -> int:
    ap = argparse.ArgumentParser(
        description="Copy ALL files from a source directory into this script's directory. Patch Python files for uHAL and dtc_sim_test_new.py."
    )
    ap.add_argument("source_dir", help="Directory containing the provided scripts (all files copied recursively).")
    ap.add_argument("--dry-run", action="store_true", help="Show what would be copied/patched without writing files.")
    args = ap.parse_args()

    src_root = os.path.abspath(os.path.expanduser(args.source_dir))
    if not os.path.isdir(src_root):
        print("ERROR: Not a directory: {}".format(src_root), file=sys.stderr)
        return 2

    dest_root = os.path.dirname(os.path.abspath(__file__))

    total_files = 0
    py_patched = 0
    dtc_patched = 0

    for dirpath, _, filenames in os.walk(src_root):
        for name in filenames:
            total_files += 1
            src = os.path.join(dirpath, name)
            rel = os.path.relpath(src, src_root)
            dst = os.path.join(dest_root, rel)

            if args.dry_run:
                if name.endswith(".py"):
                    with open(src, "r", encoding="utf-8", errors="replace") as f:
                        lines = f.readlines()

                    _, uhal_would = patch_uhal_manager(lines)

                    dtc_would = False
                    if os.path.basename(src) == DTC_SIM_SCRIPT_NAME:
                        _, dtc_would = patch_dtc_sim_registers(lines)

                    flags: List[str] = []
                    if uhal_would:
                        flags.append("UHAL")
                    if dtc_would:
                        flags.append("DTC")

                    if flags:
                        print("PATCH[{}] : {}".format(",".join(flags), rel))
                    else:
                        print("COPY        : {}".format(rel))
                else:
                    print("COPY        : {}".format(rel))
                continue

            # Real run: write outputs
            if name.endswith(".py"):
                changed_any, changed_dtc = patch_python_file(src, dst)
                if changed_any:
                    py_patched += 1
                if changed_dtc:
                    dtc_patched += 1

                if changed_any and changed_dtc:
                    print("PATCH[PY,DTC]: {}".format(rel))
                elif changed_any:
                    print("PATCH[PY]    : {}".format(rel))
                else:
                    os.makedirs(os.path.dirname(dst), exist_ok=True)
                    shutil.copy2(src, dst)
                    print("COPY        : {}".format(rel))
            else:
                os.makedirs(os.path.dirname(dst), exist_ok=True)
                shutil.copy2(src, dst)
                print("COPY        : {}".format(rel))

    print("\nDone.")
    print("  Total files copied: {}".format(total_files))
    print("  Python files patched: {}".format(py_patched))
    print("  dtc_sim_test_new.py patched: {}".format(dtc_patched))
    print("Destination: {}".format(dest_root))
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
