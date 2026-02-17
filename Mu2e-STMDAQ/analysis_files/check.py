import os
import glob
import re
import sys
import struct

def get_subrun_number(filename):
    """Extract subrun number from filename."""
    match = re.search(r"_subrun(\d+)\.bin$", filename)
    return int(match.group(1)) if match else -1

def check_continuity_streamed(filenames):
    last_val = None

    print("→ Checking files in order (by subrun):")
    for fname in filenames:
        print(f"   subrun {get_subrun_number(fname):2d} → {fname}")
    print()

    for fname in filenames:
        with open(fname, "rb") as f:
            while True:
                bytes_read = f.read(2)
                if not bytes_read:
                    break
                val = struct.unpack('<h', bytes_read)[0]  # little-endian int16

                if last_val is not None:
                    expected = (last_val + 1) % 65536
                    # Interpret val as unsigned for comparison
                    val_unsigned = val if val >= 0 else val + 65536
                    if val_unsigned != expected:
                        print(f"[ERROR] Discontinuity in {fname}")
                        print(f"        Expected {expected}, got {val_unsigned}")
                        return
                last_val = val if val >= 0 else val + 65536

    print("[SUCCESS] Counter is continuous across all files.")

if __name__ == "__main__":
    directory = sys.argv[1] if len(sys.argv) > 1 else "."
    pattern = os.path.join(directory, "mu2estm_hpge_raw_2025-07-17_11-*.bin")
    files = glob.glob(pattern)
    files = sorted(files, key=get_subrun_number)

    if not files:
        print(f"[ERROR] No matching files found in {directory}")
    else:
        check_continuity_streamed(files)
