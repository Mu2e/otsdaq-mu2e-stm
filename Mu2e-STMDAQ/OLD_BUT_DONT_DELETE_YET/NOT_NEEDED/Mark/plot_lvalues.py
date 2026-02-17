import sys
import os
import numpy as np 
import matplotlib.pyplot as plt
import argparse

parser = argparse.ArgumentParser(description='')
parser.add_argument('--file', type=str, required=True, dest='filename', help='binary filename')
args = parser.parse_args()

filename = args.filename
dt = np.dtype('double') 
fptr = open(filename,"rb")
fsize = os.path.getsize(filename)
fptr.seek(0) 
b = fptr.read(fsize) 
values = np.frombuffer(b,dt)

print(values)

cutV = values[np.where(np.logical_and(values>=-1200, values<=1200))]

plt.ion()
fig, ax = plt.subplots()
ax.hist(cutV, bins=240, align='mid',color="orange")
ax.set_xlabel("l Values")
plt.yscale('log')

plt.show()
input('...')

