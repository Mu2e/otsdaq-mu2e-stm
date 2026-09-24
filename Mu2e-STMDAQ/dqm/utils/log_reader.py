"""
log_reader.py  –  DAQ log file reader for DQM dash app

Resolves the log file to display via (in priority order):
  1. Environment variable DAQ_LOG_FILE  – explicit path to a single file
  2. Environment variable DAQ_LOG_DIR   – directory; picks the most-recently
                                          modified *.log (or *.txt) file in it
  3. Falls back to the current working directory as the search dir

Call `get_log_lines()` from a Dash callback / dcc.Interval tick.
"""

import os
import glob
import time
from pathlib import Path
from typing import Optional


# ---------------------------------------------------------------------------
# Configuration helpers
# ---------------------------------------------------------------------------

def _resolve_log_file() -> Optional[Path]:
    """
    Return the Path of the log file to tail, or None if nothing is found.
    Resolution order:
      1. DAQ_LOG_FILE  – explicit file path
      2. LOG_DIR   – directory; newest *.log 
      3. cwd           – same directory search as fallback
    """
    explicit = os.environ.get("DAQ_LOG_FILE", "").strip()
    if explicit:
        p = Path(explicit)
        if p.is_file():
            return p
        # Explicit path set but file doesn't exist yet – return it anyway so
        # the UI can show a "waiting for file" message rather than a stale one.
        return p

    search_dir = Path(os.environ.get("LOG_DIR", ".").strip())
    candidates = sorted(
        search_dir.glob("*.log"),
        key=lambda f: f.stat().st_mtime,
        reverse=True,
    )
    return candidates[0] if candidates else None


# ---------------------------------------------------------------------------
# Public API
# ---------------------------------------------------------------------------

def get_log_lines(
    max_lines: int = 500,
    tail_only: bool = True,
) -> dict:
    """
    Read the current log file and return a dict consumed by the Dash layout.

    Returns
    -------
    {
        "path":      str   – resolved file path (or empty string),
        "lines":     list  – list of plain-text log lines (newest last),
        "mtime":     float – file modification timestamp (0 if unavailable),
        "error":     str   – non-empty if something went wrong,
    }
    """
    result = {"path": "", "lines": [], "mtime": 0.0, "error": ""}

    log_path = _resolve_log_file()
    if log_path is None:
        result["error"] = (
            "No log file found.  Set DAQ_LOG_FILE or DAQ_LOG_DIR."
        )
        return result

    result["path"] = str(log_path)

    if not log_path.exists():
        result["error"] = f"Waiting for log file: {log_path}"
        return result

    try:
        result["mtime"] = log_path.stat().st_mtime
        with log_path.open("r", errors="replace") as fh:
            all_lines = fh.readlines()

        lines = all_lines[-max_lines:] if tail_only else all_lines
        # Strip trailing newlines but preserve empty lines so the layout
        # can render blank separators faithfully.
        result["lines"] = [ln.rstrip("\n") for ln in lines]
    except OSError as exc:
        result["error"] = f"Cannot read {log_path}: {exc}"

    return result


def format_mtime(mtime: float) -> str:
    """Human-readable last-modified string from a stat mtime float."""
    if mtime == 0.0:
        return "unknown"
    return time.strftime("%Y-%m-%d  %H:%M:%S", time.localtime(mtime))
