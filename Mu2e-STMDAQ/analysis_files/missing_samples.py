import os
import glob
import re
import sys
import struct
from collections import Counter

def get_subrun_number(filename):
    """Extract subrun number from filename."""
    match = re.search(r"_subrun(\d+)\.bin$", filename)
    return int(match.group(1)) if match else -1

def estimate_total_samples(filenames):
    """Estimate total int16 samples from total byte size."""
    total_bytes = sum(os.path.getsize(f) for f in filenames)
    return total_bytes // 2  # each int16 is 2 bytes

def check_continuity_streamed(filenames):
    last_val = None
    global_index = 0
    discontinuities = []
    expected_total = estimate_total_samples(filenames)

    print("→ Checking files in order (by subrun):")
    for fname in filenames:
        print(f"   subrun {get_subrun_number(fname):2d} → {fname}")
    print()
    print(f"[INFO] Estimated total samples: {expected_total}")
    print()

    for fname in filenames:
        with open(fname, "rb") as f:
            while True:
                bytes_read = f.read(2)
                if not bytes_read:
                    break
                val = struct.unpack('<h', bytes_read)[0]
                val_u16 = val if val >= 0 else val + 65536

                if last_val is not None:
                    expected = (last_val + 1) % 65536
                    if val_u16 != expected:
                        discontinuities.append((global_index, expected, val_u16, fname))
                        print(f"[DISCONTINUITY] idx {global_index:>7}: expected {expected:>5}, got {val_u16:>5} in {fname}")

                last_val = val_u16
                global_index += 1

    print("\n==============================")
    print(f"[SUMMARY] Checked {global_index} samples across {len(filenames)} file(s)")
    print(f"[SUMMARY] Total discontinuities: {len(discontinuities)}")

    if discontinuities:
        # Analyze spacing between discontinuities
        gaps = [discontinuities[i+1][0] - discontinuities[i][0] for i in range(len(discontinuities)-1)]
        gap_counts = Counter(gaps)

        print("\n[DISCONTINUITY SPACING]")
        for gap, count in sorted(gap_counts.items()):
            print(f"  Gap of {gap:>5} samples occurred {count} time(s)")

        # Optional: print first few entries for reference
        print("\n[EXAMPLES]")
        for i, (idx, exp, got, fname) in enumerate(discontinuities[:5]):
            print(f"  idx {idx}: expected {exp}, got {got} in {fname}")
        if len(discontinuities) > 5:
            print("  ...")

def main():
    import argparse
    parser = argparse.ArgumentParser(description="Check int16 counter continuity in binary files.")
    parser.add_argument("path", help="Path to file or directory containing subrun files.")
    args = parser.parse_args()

    # Handle single file or directory
    if os.path.isfile(args.path):
        filenames = [args.path]
    elif os.path.isdir(args.path):
        pattern = os.path.join(args.path, "mu2estm_hpge_raw_2025-07-17_11-*.bin")
        filenames = glob.glob(pattern)
    else:
        print(f"[ERROR] Path not found: {args.path}")
        return

    filenames = sorted(filenames, key=get_subrun_number)

    if not filenames:
        print(f"[ERROR] No matching files found.")
        return

    check_continuity_streamed(filenames)

if __name__ == "__main__":
    main()
