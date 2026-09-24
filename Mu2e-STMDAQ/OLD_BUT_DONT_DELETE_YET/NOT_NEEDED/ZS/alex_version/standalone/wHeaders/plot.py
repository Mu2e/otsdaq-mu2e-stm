import sys
import os
import math
import numpy as np
from numpy import * 

def meanVal(arr, n):
    sumVal = 0
    mean = 0
    for i in range(0,n):
        sumVal += (arr[i])
    mean = sumVal / n 
    return mean

def rmsVal(arr, n, mu):
    square = 0
    mean = 0
    root = 0

    #Calculate square
    for i in range(0,n):
        square += ((arr[i]-mu)**2)

    #Calculate sqrt 
    root = math.sqrt(square/n) 

    return root

# -------------------------------------
# Plot data and save
# -------------------------------------
print ("The data file is:",sys.argv[1])
data_file = sys.argv[1]
f_name = sys.argv[2]
outputdata=[]
# Open data file
with open(data_file, 'rb') as f:
    # Get data from file
    #outputdata = np.fromfile(f, dtype=np.int16, count=plotmax)
    outputdata = np.fromfile(f, dtype=np.int16)

# Output data length
output_data_length = len(outputdata)
print("Output data has length: ", output_data_length)

plotmax = output_data_length

plot_dir = "plots"
# Check whether the specified plot_dir exists or not
isExist = os.path.exists(plot_dir)
if not isExist:
   # Create a new directory because it does not exist
   os.makedirs(plot_dir)
   print(plot_dir,"directory has been created!")
else:
   print(plot_dir,"directory already exists.")

# Fit sine data
# from scipy import optimize
# def sine(x, a, b, c):
#     return a * np.sin(b * x + c)
# params, params_covariance = optimize.curve_fit(sine, time, output_data)
# print(params)
# freq = params[1]/(2*math.pi)
# print (freq)
    
# Import matplotlib
from matplotlib import pyplot as plt
# Calcuate time array
time = []
clock = 1/(300e6)
for i in range(output_data_length):
    time.append(float(i*clock))
    #time.append(i)

# Plot comparison data for only 2 repeated block RAMS
plt.figure(figsize=(10,8))
# Plot start only
mu = meanVal(outputdata[0:plotmax], plotmax)
rms = rmsVal(outputdata[0:plotmax], plotmax, mu)
print("Mean = %0.2f" % mu)
print("RMS = %0.2f" % rms)
plt.plot(time[0:plotmax],outputdata[0:plotmax], 'r-',label=('genData662keV_20kHz: baseline from ZS only'))
plt.legend(loc='best')
plt.ylabel('ADC counts', fontsize=12)
plt.tight_layout()
title = plot_dir + '/' + f_name + '.png'
plt.savefig(title)
