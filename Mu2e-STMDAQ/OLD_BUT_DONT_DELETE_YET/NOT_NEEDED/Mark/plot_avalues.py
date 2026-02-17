import sys
import os
import numpy as np 
import matplotlib.pyplot as plt
import argparse

parser = argparse.ArgumentParser(description='')
parser.add_argument('--file', type=str, required=True, dest='filename', help='binary filename')
args = parser.parse_args()

filename = args.filename
dt = np.dtype('<i2') 
fptr = open(filename,"rb")
fsize = os.path.getsize(filename)
fptr.seek(0) 
b = fptr.read(fsize) 
values = np.frombuffer(b,dt)

cutV = values[np.where(np.logical_and(values>=-5000, values<=5000))]

print(values)

plt.ion()
ll = len(cutV)
tt = np.linspace(0.0,ll*2.7,ll)
print("Length of values: ",ll)

fig, ax = plt.subplots()

ax.set_xlabel("ADC Values")
ax.set_xlabel("Time ns")
ax.scatter(tt,cutV,color='darkorange',s=2.0)
plt.savefig(filename+'.png')

plt.show()
input('...')

