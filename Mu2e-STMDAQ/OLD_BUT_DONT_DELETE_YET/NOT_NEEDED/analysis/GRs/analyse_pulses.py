import sys
import os
import math
import numpy as np
from numpy import *     
from matplotlib import pyplot as plt
from scipy.optimize import curve_fit
import csv
from scipy.stats import norm

# STM IERC pulse width = 150 ns
pulse_width = 45 # 150 ns in clocks ticks

# Truncate edge of string
def truncate_from_last(string, char):
    last_index = string.rfind(char)
    if last_index != -1:
        return string[last_index+1:]
    return string

# Twos compliment
def twos_comp(n, w):
    if n & (1 << (w - 1)): n = n - (1 << w)
    return n

# Rising edge function
def rising_edge(t, offset,amplitude, rise_time, t0):
    return offset + amplitude * (1 - np.exp(-(t - t0) / rise_time)) * (t >= t0)

# Fit the rising edge
def fit_rising_edge(x,y,pulse_time):

    # Function guess values
    offset_guess = min(y) # offset
    amp_guess = max(y) # amplitude
    center_guess = pulse_time # pulse centre
    #width_guess = 45 # 150 ns in clocks ticks
    width_guess = 150*1e-9 # pulse width [150 ns]
    t0_guess = center_guess - width_guess/2 # rising edge t0
    rise_time_guess = 6*1e-9 # rising edge rise time
    
    # Find the middle of the pulse (same as middle of data)
    middle = int(len(x)/2)
    # x values to middle 
    fit_x = x[0:middle]
    # y values to middle 
    fit_y = y[0:middle]

    # Initial guesses for the paramater
    p0 = [offset_guess,amp_guess, rise_time_guess, t0_guess]

    # Fit the rising edge
    popt, _ = curve_fit(rising_edge, fit_x , fit_y, p0)
    offset,amplitude, rise_time, t0 = popt

    # Calculate rising edge
    rising_edge_fit = (t0+rise_time)

    # Return rising edge and fit parameters
    return rising_edge_fit,popt

# Convert clock ticks to time in seconds
def to_time(tick):
    return tick/300e6
    
# PRINT OUTPUT
output = False

# Packet size [bytes]
PACKET_SIZE = 8198+2
# Packet length [no of int16_t entries]
PACKET_LEN = PACKET_SIZE / 2

# Fixed packet header length
P_HDR_LEN = 4
# Fixed event header length
E_HDR_LEN = 32;
# Event header variable names
E_HDR_NAMES = ["75_MHz_Time",
               "Event_Number",
               "Event_Window_Tag_DTC",
               "Event_Mode_DTC",
               "Delivery_Ring_RF_Marker_DTC",
               "Event_Start_Offset",
               "Event_Length_PKT",
               "BEEF",
               "SUBRUN_NUM",
               "ZSflag",
               "Prescale_Value",
               "Run_Num"
               "Channel",
               "40_MHz_Time"]

E_HDR_PLACEHOLDER = [0xCAFE,0xBBBB,0xAAAA,0xCAFE],

# Boolean to indicate whether we're check all data
allData = True
# Block RAM size (portion of data file read in) [bytes]
RAM_SIZE = 65536 * 32 / 8
# Block RAM length [no of int16_t entries]
RAM_LEN = RAM_SIZE / 2
# Number of block RAMs to check (if not allData)
RAM_NUM = 200


# -------------------------------------
# Function to get the event header
# -------------------------------------
def getEventHdr(header):

    nheader = header[4:]
    
    # Get 75 MHz event time
    TM75_0 = int(nheader[0]) & 0xFFFF;
    TM75_1 = int(nheader[1]) & 0xFFFF;
    TM75_2 = int(nheader[2]) & 0xFFFF;
    TM75_3 = int(nheader[3]) & 0xFFFF;    
    TM75 = TM75_3 << 48 | TM75_2 << 32 | TM75_1 << 16 | TM75_0;    
    TM75 = TM75/(75e6)*1e9
    if (TM75 < 0):
        print (TM75_0,int(TM75_0) & 0xFFFF)
        print("%d = %d << 48 | %d << 32 | %d << 16 | %d" % (TM75,TM75_3,TM75_2,TM75_1,TM75_0))
        exit(0)
    if (output): print("75 Mhz Time = ",TM75)

    # Get event number
    EN_0 = nheader[4];
    EN_1 = nheader[5];
    EN_2 = nheader[6];
    EN = EN_2 << 32 | EN_1 << 16 | EN_0;    
    if (output): print("Event Number = ",EN)

    # Get event window tag
    EWT_0 = nheader[7];
    EWT_1 = nheader[8];
    EWT_2 = nheader[9];
    EWT = EWT_2 << 32 | EWT_1 << 16 | EWT_0;    
    if (output): print("Event Window Tag = ",EWT)

    # Get event mode
    EM_0 = nheader[10];
    EM_1 = nheader[11];
    EM_2 = nheader[12] & 0xFF;
    EM = EM_2 << 32 | EM_1 << 16 | EM_0;    
    if (output): print("Event Mode = ",EM)

    # Get Delivery Ring RF Marker TDC
    DRM = nheader[12] >> 8 & 0xF;
    if (output): print("Delivery Ring RF Marker TDC = ",DRM)

    # Get event start offset
    ESO = nheader[13];
    if (output): print("Event Start Offset = ",ESO)

    # Get event length from packet
    EIP = nheader[14];
    if (output): print("Event Length = ",EIP)

    check = nheader[15]
    beef = twos_comp(0xBEEF,16)
    #concheck = check.as_type(np.int16)
    # Check index 15 of nheader is 0xBEEF
    if(check != beef):
        print("ERROR: Event number %d nheader at index 15 is not = 0xBEEF!" % EN)
        np.set_printoptions(formatter={'int':lambda x:hex(int(x))})
        #print(check)
        #print(nheader)
        exit(0)

    # Subrun Number
    SR_0 = nheader[16];
    SR_1 = nheader[17];
    SR = SR_1 << 16 | SR_0;
    if (output): print("Subrun Number = ",SR);

    # ZS flag
    ZS = nheader[18] >> 8 & 0xF;
    if (output): print("ZS flag = ",ZS);
    # Prescale Value
    PS = nheader[18] >> 12 & 0xF;
    if (output): print("Prescale Value = ",PS);

    # Total event length
    EL = nheader[19];
    if (output): print("Total Event Length = ",EL);

    # Run Number
    RN_0 = nheader[20];
    RN_1 = nheader[21];
    RN_2 = nheader[22];
    RN_3 = nheader[23];    
    RN = RN_3 << 48 | RN_2 << 32 | RN_1 << 16 | RN_0;    
    if (output): print("Run Number = ",RN)

    # Get the channel number
    Channel = nheader[24] & 0xFF;
    if (output): print("Channel = ",Channel)

    # Get 40 MHz event time
    TM40_0 = int((nheader[24] >> 8) & 0xF) & 0xFFFF
    TM40_1 = int(nheader[25]) & 0xFFFF
    TM40_2 = int(nheader[26]) & 0xFFFF
    TM40_3 = int(nheader[27]) & 0xFFFF
    TM40 = TM40_3 << 40 | TM40_2 << 24 | TM40_1 << 8 | TM40_0;
    TM40 = TM40/(40e6)*1e9
    if (output): print("40 Mhz Time = ",TM40)

    # Return event header variables
    return TM75,EWT,EN,EM,DRM,ESO,EIP,SR,ZS,PS,EL,RN,Channel,TM40;

# -------------------------------------
# Function to get ADC data
# -------------------------------------
def getADCdata(data):

    # Define total data count
    data_count=0
    # Define data count
    adc_count=0
    
    # Event header array
    eHdr_data = []

    # Event data
    e_data = []

    # Boolean to signal we've reached the maximum data to read
    reached_max  = False
    # Get end of data to loop over
    data_max = 0
    if (allData):
        data_max = len(data)
    else:
        data_max = RAM_NUM * RAM_LEN

    # Loop over all data
    while(data_count < len(data)):

        # Break if acquired maximum data
        if (reached_max):
            break
    
        # -----------------
        # Event header
        # -----------------        
        # Get event header
        eHdr = data[int(data_count):int(data_count)+int(E_HDR_LEN)]
        eHdr = getEventHdr(eHdr);
        # Get the EWT
        EWT = eHdr[1]
        # Store event header in the event header array
        eHdr_data.append(eHdr)
        # Increase counters by event header length
        data_count += E_HDR_LEN
        
        # -----------------
        # ADC data
        # -----------------        

        # Get event length
        eLen = eHdr[10]
        
        # Get the amount of data left in the packet
        eventToWrite = int(eLen)
        
        # Check if we're about to reach the data_max
        if (int(adc_count) + int(eLen) >= data_max):
            # Get the amount of remaining data to write
            eventToWrite = int(data_max - data_count)
            reached_max = True

        # Get event data
        new_event = np.array(data[int(data_count):int(data_count)+eventToWrite])

        # Store data by event
        e_data.append([EWT,new_event])

        # Increase counters by event length
        data_count += int(eventToWrite)
        adc_count += int(eventToWrite)

        # Break loop if data_max reached
        if (reached_max) :
            break

    # Return event header and adc data
    return eHdr_data,e_data

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
print(data_file)

# Get output_data data
output_data = getADCdata(data)
output_headers = output_data[0]
event_data = output_data[1]
#output_data = output_data[0]

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

# Loop over all data
for i in range(len(event_data)):
    # Get the event window tag
    EWT = event_data[i][0]
    # Get the event data
    e_data = event_data[i][1]
    # Get the event length
    eLen = len(event_data[i][1])
    # Initiliase a new array for the pulse avg time
    event_data[i].append([])    
    # Initiliase a new array for the rising pulse edge
    event_data[i].append([])
    # Initiliase a new array for the fit parameters
    event_data[i].append([])
    # Loop over the event length
    for j in range(eLen):
        # If the data is above the pulse threshold
        if (e_data[j] > threshold):
            # If the pulse started in the previous event
            if (avg_den > 0 and j == 0):
                # Reset the average number...
                avg_num = 0
                # ... and sum negative value from the previous event
                for k in range(-avg_den,0):
                    avg_num += k
            # Average pulse time num
            avg_num += j
            # Average pulse time den
            avg_den += 1
        # Else if not in a pulse region
        else:
            # If we've just come out of avergaing a pulse
            if (avg_num != 0):
                # Calculate the pulse time average
                avg_time = int(avg_num/avg_den)
                # Calculate the seperation between the pulses
                pulse_sep = avg_time-prev_pulse_time
                # If the previous pulse was in the previous event
                if (pulse_sep < 0):                    
                    pulse_sep = avg_time + (len(event_data[i-1][1])  - prev_pulse_time)
                # Store previous pulse time
                prev_pulse_time = avg_time
                # Store current pulse location
                pulse_loc = avg_time
                # Define the width of the pulse fit region
                pulse_region = 51
                # Minimum pulse fit region
                pulse_min = int(pulse_loc-(pulse_region-1)/2)
                # Maximum pulse fit reiogn
                pulse_max = int(pulse_loc+(pulse_region-1)/2)
                # x values to fit
                x = np.linspace(pulse_min/300e6, pulse_max/300e6, pulse_region)
                # y values to fit
                y = event_data[i][1][pulse_min:pulse_max+1]
                # # If the minimum pulse fit region is in the previous event
                if (pulse_min < 0):
                    # Store the y data that is in the previous event
                    prev_event = event_data[i-1][1][pulse_min:]
                    # Store the y data that is in this event
                    this_event = event_data[i][1][0:pulse_max+1]
                    # New y values to fit
                    y = np.concatenate((prev_event,this_event))
                # Fit rising edge
                rising_edge_fit,popt = fit_rising_edge(x,y,pulse_loc/300e6)
                # If the rising edge is larger than the found pulse time
                if (rising_edge_fit < 0):
                    # Rising edge is in the previous event
                    rising_edge_fit = len(event_data[i-1][1])/300e6 - rising_edge_fit                
                    EWT -= 1
                    # Save pulse time in previous event
                    event_data[i-1][2].append(pulse_loc) # BUG IF NEGATIVE 
                    event_data[i-1][3].append(rising_edge_fit)
                    event_data[i-1][4].append(popt)
                else:
                    # Save pulse time in this event
                    event_data[i][2].append(pulse_loc)
                    event_data[i][3].append(rising_edge_fit)
                    event_data[i][4].append(popt)

                # Print pulse information
                print(pulse_num,i,EWT,avg_time,pulse_sep,pulse_sep/(300e6)*1e6,rising_edge_fit)


                # Increment number of pulses
                pulse_num += 1

                
            # Set average variables to zero
            avg_num = 0
            avg_den = 0

# Exit if no pulses found
if (pulse_num == 0):
    print("No pulses found. Exiting...")
    exit(0)
            
# -------------------------------------
# Plot data and save
# -------------------------------------

analysis_dir = '/scratch/mu2e/mu2estm_stm_Mu2eDAQ/STMDAQ/data/Oct24_GR3/analysis/'

plot_dir = analysis_dir+"plots"
# Check whether the specified plot_dir exists or not
isExist = os.path.exists(plot_dir)
if not isExist:
   # Create a new directory because it does not exist
   os.makedirs(plot_dir)
   print(plot_dir,"directory has been created!")
else:
   print(plot_dir,"directory already exists.")

# Plot first event only
pulse_loc = int(event_data[0][2][0])
edge = event_data[0][3][0]
fit = event_data[0][4][0]
pulse_region = 51
pulse_min = int(pulse_loc-(pulse_region-1)/2)
pulse_max = int(pulse_loc+(pulse_region-1)/2)
x = np.linspace(pulse_min/300e6, pulse_max/300e6, pulse_region)
y = event_data[0][1][pulse_min:pulse_max+1]

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
pulse_region = 51
pulses_per_event = 2
pulse_seps = []
f = open("%s/%s_pulse_times.txt"  % (analysis_dir,data_file), "w")
# Loop over number of events
for i in range(len(event_data)):
    # Get the EWT
    EWT = event_data[i][0]
    if (pulses_per_event <= len(event_data[i][3])):
        pulse_1 = event_data[i][3][1]
        pulse_0 = event_data[i][3][0]
        pulse_sep = pulse_1 - pulse_0
        pulse_seps.append(pulse_sep*1e9)
        write_list = [EWT,pulse_0*1e9,pulse_sep*1e9]
        # Convert list elements to strings and join them with commas
        line = ",".join(str(x) for x in write_list)
        f.write(line+'\n')
        print(EWT,pulse_0*1e9,pulse_sep*1e9)
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
