import sys
import os
import math
import numpy as np
from numpy import *     
from matplotlib import pyplot as plt
from scipy.optimize import curve_fit
import csv
from scipy.stats import norm

from analysis_funcs import *
from stm_headers import *

# PRINT OUTPUT
output = False

# Boolean to indicate whether we're check all data
allData = True
# If not using all data, maximum number of events
maxEvents = 4000

# -------------------------------------
# Main
# -------------------------------------

print ("The data file is:",sys.argv[1])
data_file = sys.argv[1]

# Open data file
with open(data_file, 'rb') as f:
    # Get data from file
    data = np.fromfile(f, dtype=np.int16)    
print("Input data has length: ", len(data))    

# Remove ".bin" from string
data_file = truncate_from_last(data_file[:-4],"/")
data_file = data_file[8:]

# file output_data data
headers,event_data,adc_data,event_info = getADCdata(data,output,allData,maxEvents)

# Pulse time average num
avg_num = 0;
# Pulse time average den
avg_den = 0
# Pulse seperation
pulse_sep = 0
# Old average value
prev_pulse_time = 0
# Number of pulses
pulse_num = 0
# Number of events
event_count = 0
# Pulse threshold
threshold  = 20000
# Pulse width
pulse_width_ns = 110*1e-9 # pulse width [110 ns]
pulse_width = int(pulse_width_ns*300e6)

pulse_data=[]

# Loop over all data
for i in range(len(adc_data)):
    if (i >= event_info[event_count][1]):
        event_count += 1
    EWT = event_info[event_count][0]
    # If the data is above the pulse threshold
    if (adc_data[i] > threshold):
        # Reset the average number...
        avg_num += i
        # Average pulse time den
        avg_den += 1
        # Else if not in a pulse region
    else:
        # If we've just come out of avergaing a pulse
        if (avg_num != 0):
            # Calculate the pulse time average
            avg_time = int(avg_num/avg_den)
            # Store current pulse location
            pulse_loc = avg_time                
            # Minimum pulse fit region
            pulse_min = int(pulse_loc)-pulse_width
            if (pulse_min < 0):
                pulse_min = 0                
            # Maximum pulse fit reiogn
            pulse_max = pulse_loc
            if (pulse_max+1 == len(adc_data)):
                pulse_max -= 1
            # x values to fit
            x = np.linspace(pulse_min/300e6, pulse_max/300e6, pulse_width)
            # y values to fit
            y = adc_data[pulse_min:pulse_max]
            # Fit rising edge
            rising_edge_fit,popt = fit_rising_edge(x,y,pulse_loc/300e6)
            # Save pulse time in this event
            pulse_data.append([EWT,pulse_loc,rising_edge_fit,popt])
            if (pulse_num % 1000 == 0):
                print(pulse_num,EWT,rising_edge_fit,pulse_loc/300e6)
            # Increment number of pulses
            pulse_num += 1
        # Set average variables to zero
        avg_num = 0
        avg_den = 0

            
    
# exit(0)

# # Loop over all data
# for i in range(len(event_data)):
#     # Get the event window tag
#     EWT = event_data[i][0]
#     # Get the event data
#     e_data = event_data[i][1]
#     # Get the event length
#     eLen = len(event_data[i][1])
#     # Initiliase a new array for the pulse avg time
#     event_data[i].append([])    
#     # Initiliase a new array for the rising pulse edge
#     event_data[i].append([])
#     # Initiliase a new array for the fit parameters
#     event_data[i].append([])
#     # Loop over the event length
#     for j in range(eLen):
#         # If the data is above the pulse threshold
#         if (e_data[j] > threshold):
#             # If the pulse started in the previous event
#             if (avg_den > 0 and j == 0):
#                 # Reset the average number...
#                 avg_num = 0
#                 # ... and sum negative value from the previous event
#                 for k in range(-avg_den,0):
#                     avg_num += k
#             # Average pulse time num
#             avg_num += j
#             # Average pulse time den
#             avg_den += 1
#         # Else if not in a pulse region
#         else:
#             # If we've just come out of avergaing a pulse
#             if (avg_num != 0):
#                 # Calculate the pulse time average
#                 avg_time = int(avg_num/avg_den)
#                 # Calculate the seperation between the pulses
#                 pulse_sep = avg_time-prev_pulse_time
#                 # If the previous pulse was in the previous event
#                 if (pulse_sep < 0):                    
#                     pulse_sep = avg_time + (len(event_data[i-1][1])  - prev_pulse_time)
#                 # Store previous pulse time
#                 prev_pulse_time = avg_time
#                 # Store current pulse location
#                 pulse_loc = avg_time
#                 # Define the width of the pulse fit region
#                 pulse_region = 51
#                 # Minimum pulse fit region
#                 pulse_min = int(pulse_loc-(pulse_region-1)/2)
#                 # Maximum pulse fit region
#                 pulse_max = int(pulse_loc+(pulse_region-1)/2)
#                 # x values to fit
#                 x = np.linspace(pulse_min/300e6, pulse_max/300e6, pulse_region)
#                 # y values to fit
#                 y = event_data[i][1][pulse_min:pulse_max+1]
#                 # # If the minimum pulse fit region is in the previous event
#                 if (pulse_min < 0):
#                     # Store the y data that is in the previous event
#                     prev_event = event_data[i-1][1][pulse_min:]
#                     # Store the y data that is in this event
#                     this_event = event_data[i][1][0:pulse_max+1]
#                     # New y values to fit
#                     y = np.concatenate((prev_event,this_event))
#                 # Fit rising edge
#                 rising_edge_fit,popt = fit_rising_edge(x,y,pulse_loc/300e6)
#                 # If the rising edge is larger than the found pulse time
#                 if (rising_edge_fit < 0):
#                     # Rising edge is in the previous event
#                     rising_edge_fit = len(event_data[i-1][1])/300e6 - rising_edge_fit
#                     # Save pulse time in previous event
#                     event_data[i-1][2].append(pulse_loc) # BUG IF NEGATIVE 
#                     event_data[i-1][3].append(rising_edge_fit)
#                     event_data[i-1][4].append(popt)
#                     # Print pulse information
#                     #print(pulse_num,i,EWT-1,avg_time,pulse_sep,pulse_sep/(300e6)*1e6,rising_edge_fit)

#                 else:
#                     # Pulse ended at the last bin of the previous event
#                     if (j == 0):
#                         # Print pulse information
#                         #print(pulse_num,i-1,EWT-1,avg_time,pulse_sep,pulse_sep/(300e6)*1e6,rising_edge_fit)
#                         # Save pulse time in previous event
#                         event_data[i-1][2].append(pulse_loc) # BUG IF NEGATIVE 
#                         event_data[i-1][3].append(rising_edge_fit)
#                         event_data[i-1][4].append(popt)
#                     else:
#                         # Print pulse information
#                         #print(pulse_num,i,EWT,avg_time,pulse_sep,pulse_sep/(300e6)*1e6,rising_edge_fit)
#                         # Save pulse time in this event
#                         event_data[i][2].append(pulse_loc)
#                         event_data[i][3].append(rising_edge_fit)
#                         event_data[i][4].append(popt)
                                              
#                 # Increment number of pulses
#                 pulse_num += 1
                
#             # Set average variables to zero
#             avg_num = 0
#             avg_den = 0

# Exit if no pulses found
if (pulse_num == 0):
    print("No pulses found. Exiting...")
    exit(0)
            
# -------------------------------------
# Plot data and save
# -------------------------------------

cwd = os.getcwd()
analysis_dir = str(cwd)+'/output'

# Check whether the specified plot_dir exists or not
isExist = os.path.exists(analysis_dir)
if not isExist:
   # Create a new directory because it does not exist
   os.makedirs(analysis_dir)
   print(analysis_dir,"directory has been created!")
else:
   print(analysis_dir,"directory already exists.")


plot_dir = analysis_dir+"/plots"
# Check whether the specified plot_dir exists or not
isExist = os.path.exists(plot_dir)
if not isExist:
   # Create a new directory because it does not exist
   os.makedirs(plot_dir)
   print(plot_dir,"directory has been created!")
else:
   print(plot_dir,"directory already exists.")


# Plot first event only
pulse_loc = int(pulse_data[0][1])
edge = pulse_data[0][2]
fit = pulse_data[0][3]
# Minimum pulse fit region
pulse_min = int(pulse_loc)-pulse_width
pulse_max = pulse_loc+1
x = np.linspace(pulse_min/300e6, pulse_max/300e6, pulse_width+1)
y = adc_data[pulse_min:pulse_max]

fig1 = plt.figure()
plt.title(f'First pulse')
plt.plot(x,y, 'o',label="Firmware output")
plt.plot(x, rising_edge(x, *fit), 'r-', label='Fit')
plt.axvline(x=edge, color='k', linestyle='--', label='Rising Edge')
plt.legend(loc='best')
plt.ylabel('ADC counts', fontsize=12)
plt.xlabel('Time [ns]', fontsize=12)
plt.tight_layout()
plt.savefig('%s/%s_first_pulse' % (plot_dir,data_file))

# Loop over pulses
pulses_per_event = 2
pulse_seps = []
f = open("%s/%s_pulse_times.txt"  % (analysis_dir,data_file), "w")
# Get first EWT
old_EWT = pulse_data[0][0]-1
# EWT counter
EWT_count = 0
# Loop over number of events
for i in range(len(pulse_data)):
    # Get the EWT
    EWT = pulse_data[i][0]
    if (i != len(pulse_data)-1 and old_EWT != EWT):
        pulse_1 = pulse_data[i+1][2]
        pulse_0 = pulse_data[i][2]
        pulse_sep = pulse_1 - pulse_0
        pulse_seps.append(pulse_sep*1e9)
        write_list = [EWT,pulse_0*1e9,pulse_sep*1e9]
        # Convert list elements to strings and join them with commas
        line = ",".join(str(x) for x in write_list)
        f.write(line+'\n')
        if (i % 1000 == 0): print(i,EWT,pulse_0*1e9,pulse_1*1e9,pulse_sep*1e9)
        old_EWT = EWT
f.close()


# Create a histogram of the seperations
hist, bins = np.histogram(pulse_seps, bins=30, density=True)

# Plot the histogram 
fig2 = plt.figure()
plt.hist(pulse_seps, bins=30, density=True, alpha=0.6, label='Histogram')

# Fit a normal distribution to the data
mu, std = norm.fit(pulse_seps)
# Create a range of x values for the fitted distribution
xmin, xmax = plt.xlim()
x = np.linspace(xmin, xmax, 100)
# Calculate the PDF of the fitted distribution
p = norm.pdf(x, mu, std)

# Plot the fit
plt.plot(x, p, 'k', linewidth=2)

# Label and save
plt.title(f'Separation between first and second pulse in every event\n Entries = %d. Fit results: mu = %.2f,  std = %.2f' % (len(pulse_seps),mu, std))
plt.xlabel('Time [ns]', fontsize=12)
plt.tight_layout()
plt.savefig('%s/%s_pulse_sep' % (plot_dir,data_file))
