#!/usr/bin/env bash

file1="/data/alexkesh/mu2estm_HPGe_zs_2026-01-19_04-01-35_subrun0.bin"
file2="/home/mu2estm/alexkesh/Mu2e-STMDAQ/testing/data.bin"
n=100   # number of differences to print

python3 - <<EOF
import numpy as np

f1 = np.fromfile("$file1", dtype=np.int16)
f2 = np.fromfile("$file2", dtype=np.int16)

N = min(len(f1), len(f2))

print(f"{len(f1)}, {len(f2)}") 

count = 0
for i in range(N):
    if f1[i] != f2[i]:
        print(f"{i}, {N}, {f1[i]}, {f2[i]}, {f2[i]-f1[i]}")
        count += 1
        if count >= $n:
            break

if count == 0:
    print("Files are identical in the first", N, "entries.")
EOF
