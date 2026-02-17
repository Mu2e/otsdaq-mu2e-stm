import sys
import os
import numpy as np 
import matplotlib.pyplot as plt
import argparse
import struct

def unpack_header():
	header = struct.unpack('I', file.read(4))[0]  # uint32_t
	trig_num = struct.unpack('I', file.read(4))[0] # uint32_t
	trig_type = struct.unpack('H', file.read(2))[0] # uint16_t
	npeaks = struct.unpack('i', file.read(4))[0] # int
	nadc = struct.unpack('i', file.read(4))[0] # uint32_t
	print("%s trig_num = %d, trig_type = %d, npeaks = %d, nadc = %d" % (hex(header), trig_num, trig_type, npeaks, nadc))
	return trig_num, trig_type, npeaks, nadc

parser = argparse.ArgumentParser(description='')
parser.add_argument('--file', type=str, required=True, dest='filename', help='binary mwd filename')
args = parser.parse_args()
filename = args.filename

fsize = os.path.getsize(filename)
print("Filesize = %d" % (fsize))

file = open(filename,"rb")
num_triggers = struct.unpack('i', file.read(4))[0]
print("Number of triggers in MWD file = %d" % (num_triggers))

dt = np.dtype(np.double) 
offset = 4
peaks = np.array([])
times = np.array([])
num_peaks = np.array([0,0])
num_trigs = np.array([0,0])
num_adc = np.array([0,0])
num_peaks_gt1000 = np.array([0,0])
while (offset < fsize):
	file.seek(offset, os.SEEK_SET)
	trig_num, trig_type, npeaks, nadc = unpack_header()
	offset = offset + 18
	num_trigs[trig_type] = num_trigs[trig_type] + 1
	if (npeaks > 0):
		num_peaks[trig_type] = num_peaks[trig_type] + npeaks
		num_adc[trig_type] = num_adc[trig_type] + nadc
		file.seek(offset, os.SEEK_SET)
		pp = np.fromfile(file, dtype=dt, count=npeaks)
		peaks = np.append(peaks,pp)
		ngt100 = np.where(pp < -1000)[0]
		num_peaks_gt1000[trig_type] = num_peaks_gt1000[trig_type] + len(ngt100)
		offset = offset + npeaks*dt.itemsize 
		file.seek(offset, os.SEEK_SET)
		times = np.append(times,np.fromfile(file, dtype=dt, count=npeaks))
		offset = offset + npeaks*dt.itemsize

plt.ion()
mode = ['external','internal']
for i in range(2):
	total_adc_time = num_adc[i]*(1.0/320.0e6)
	print(total_adc_time)
	rate = num_peaks_gt1000[i]/total_adc_time
	print("# %s triggers = %d, # peaks = %d, [<1000 = %d], # adc values = %d, rate = %.3f" % (mode[i], num_trigs[i], num_peaks[i], num_peaks_gt1000[i], num_adc[i], rate))
fig, ax = plt.subplots()
ax.scatter(times, peaks, color='darkorange',s=10.0) 
ax.set_xlabel("Time")
ax.set_ylabel("ADC Counts")
plt.savefig(filename+'.png')
plt.show()
input('...')
plt.close()





