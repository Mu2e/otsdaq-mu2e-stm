import sys
import os
import math
import numpy as np
from numpy import * 

def twos_comp(n, w):
    if n & (1 << (w - 1)): n = n - (1 << w)
    return n

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
RAM_NUM = 64

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
# Function to get the event header
# -------------------------------------
def getEventHdr(header):

    nheader = header[4:]
    
    # Get 75 MHz event time
    TM75_0 = nheader[0];
    TM75_1 = nheader[1];
    TM75_2 = nheader[2];
    TM75_3 = nheader[3];    
    TM75 = TM75_3 << 48 | TM75_2 << 32 | TM75_1 << 16 | TM75_0;    
    TM75 = TM75/(75e6)*1e9
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
    TM40_0 = (nheader[24] >> 8) & 0xF;
    TM40_1 = nheader[25];
    TM40_2 = nheader[26];
    TM40_3 = nheader[27];
    TM40 = TM40_3 << 48 | TM40_2 << 32 | TM40_1 << 16 | TM40_0;   
    TM40 = TM40/(40e6)*1e9
    if (output): print("40 Mhz Time = ",TM40)

    print("Event number = %d, EWT = %d, ADC clock = %d, DTC clock = %d, ESO = %d, EIP = %d, Event length = %d" % (EN,EWT,TM75,TM40,ESO,EIP,EL))
    
    # Check placeholder values for nheader are correct
     #for i in range(20,E_HDR_LEN):
       #  if(nheader[i] != E_HDR_PLACEHOLDER[i-20]):
            # print("ERROR: Event nheader placeholder not %s as expected!" % E_HDR_PLACEHOLDER[i-20])
            # print (nheader[i],E_HDR_PLACEHOLDER[i-20])
            # print("Event Number = ",EN)
            #print(nheader)
            #exit(0)            
        
    return TM75,EWT,EN,EM,DRM,ESO,EIP,SR,ZS,PS,EL,RN,Channel,TM40;

# -------------------------------------
# Function to get ADC data
# -------------------------------------
def getADCdata(data):

    # Define total data count
    data_count=0
    # Define data count
    adc_count=0
    # Define packet length counter
    packet_count=0
    # Old packet number
    old_pNum = 0
    # New packet number
    new_pNum = 0
    # Old event number
    old_eNum = 0
    # New event number
    new_eNum = 0
    
    # Data array
    adc_data=[]

    # Event header array
    eHdr_data = []

    # Array of pulse data
    pulse_count = 0
    pulse_times = []

    # THIS SHOULDN"T BE HERE - DELETE LATER    
    splitPacket = False
    splitEvent = 0

    firstPacket = True

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
        DTCtime = eHdr[13]
        ADCtime = eHdr[0]
        # Store event header in the event header array
        eHdr_data.append(eHdr)
        # Get the new event number
        new_eNum = eHdr[2]
        # Check the event number has not increased by more than 1
        #if (new_eNum > 0 and (new_eNum - old_eNum) > 1):
        # print("Error: Missing event number!!")
        # exit(0)
        # Store as the old event number
        old_eNum = new_eNum
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

        # Get ADC data
        adc_data.extend(list(data[int(data_count):int(data_count)+eventToWrite]))
    
        # Loop over event data
        for i in range(int(data_count),int(data_count)+eventToWrite):
            # If the data is above the pulse threshold
            if(data[i] > 20000):
                #print((i-int(data_count))/(300e6)*1e9,data[i])
                # Get the number of counts since the start of the event
                count_since_event_start = i-int(data_count)
                # Get the number of counts since the start of the run
                count_since_adc_start = i-int(data_count) + adc_count
                
                # Get the time since the start of the event
                time_since_event_start = count_since_event_start/(300e6)*1e9
                # Get the pulse time from total number of ADC counts
                pulse_time_ADCcounts = count_since_adc_start/(300e6)*1e9
                # Get the pulse time since the DTC event timestamp
                pulse_time_DTCstamp = DTCtime + time_since_event_start
                # Get the pulse time since the ADC event timestamp
                pulse_time_ADCstamp = ADCtime + time_since_event_start
                
                # Get variables to calculate pulse separations
                pulse_diff_ADCcounts = 0
                pulse_diff_DTCstamp = 0
                pulse_diff_ADCstamp = 0
                pulse_freq_ADCcounts = 0
                pulse_freq_DTCstamp = 0
                pulse_freq_ADCstamp = 0
                # If this is not the first pule we've found
                if (pulse_count > 0):
                    # Get the previous pulse time from ADC counts
                    prev_pulse_ADCcounts = pulse_times[pulse_count-1][0]
                    # Get the pulse time difference from ADC counts
                    pulse_diff_ADCcounts = pulse_time_ADCcounts - prev_pulse_ADCcounts                    
                    # Get the pulse frequency from ADC counts
                    pulse_freq_ADCcounts = 1/(pulse_diff_ADCcounts*1e-9)*1e-3

                    # Get the previous pulse time from the DTC timestamp
                    prev_pulse_DTCstamp = pulse_times[pulse_count-1][3]
                    # Get the pulse time difference from DTC timestamp
                    pulse_diff_DTCstamp = pulse_time_DTCstamp - prev_pulse_DTCstamp
                    # Get the pulse frequency from DTC counts
                    pulse_freq_DTCstamp = 1/(pulse_diff_DTCstamp*1e-9)*1e-3
                    #print(DTCtime,time_since_event_start,pulse_diff_DTCstamp,pulse_time_DTCstamp,prev_pulse_DTCstamp,pulse_freq_DTCstamp)

                    # Get the previous pulse time from the ADC timestamp
                    prev_pulse_ADCstamp = pulse_times[pulse_count-1][6]
                    # Get the pulse time difference from ADC timestamp
                    pulse_diff_ADCstamp = pulse_time_ADCstamp - prev_pulse_ADCstamp
                    # Get the pulse frequency from ADC counts
                    pulse_freq_ADCstamp = 1/(pulse_diff_ADCstamp*1e-9)*1e-3

                # Store pulse data
                pulse_data = [pulse_time_ADCcounts,
                              pulse_diff_ADCcounts,
                              pulse_freq_ADCcounts,
                              pulse_time_DTCstamp,
                              pulse_diff_DTCstamp,
                              pulse_freq_DTCstamp,
                              pulse_time_ADCstamp,
                              pulse_diff_ADCstamp,
                              pulse_freq_ADCstamp]
               
                pulse_times.append(pulse_data)
                pulse_count += 1
 
                # Break so only print start of pulse
                break

        # Increase counters by event length
        data_count += int(eventToWrite)
        adc_count += int(eventToWrite)

        # Break loop if data_max reached
        if (reached_max) :
            break

    # Return ADC data array
    return adc_data,eHdr_data

# -------------------------------------
# Main
# -------------------------------------

print ("The data file is:",sys.argv[1])
data_file = sys.argv[1]
det = sys.argv[2]
detector = str(det)

# Open data file
with open(data_file, 'rb') as f:
    # Get data from file
    data = np.fromfile(f, dtype=np.int16)

input_data_length = len(data)
print("Input data has length: ", input_data_length)    
# Get output_data data
output_data = getADCdata(data)
output_headers = output_data[1]
output_data = output_data[0]

# Saving array as a file
toWrite = np.array(output_data)
adcfile = 'adc_only_files/'+sys.argv[1][10:-1]+'n'
toWrite.astype('int16').tofile(adcfile)

# Output data length
output_data_length = len(output_data)
print("Output data has length: ", output_data_length)

#exit(0)

# -------------------------------------
# Plot data and save
# -------------------------------------

plot_dir = "plots"
# Check whether the specified plot_dir exists or not
isExist = os.path.exists(plot_dir)
if not isExist:
   # Create a new directory because it does not exist
   os.makedirs(plot_dir)
   print(plot_dir,"directory has been created!")
else:
   print(plot_dir,"directory already exists.")

plot_dir = "plots/"+sys.argv[1][48:-4]
# Check whether the specified plot_dir exists or not
isExist = os.path.exists(plot_dir)
if not isExist:
   # Create a new directory because it does not exist
   os.makedirs(plot_dir)
   print(plot_dir,"directory has been created!")
else:
   print(plot_dir,"directory already exists.")

   
# Calcuate time array
time = []
clock = 1/(300e6)
for i in range(output_data_length):
    time.append(float(i*clock))
    #time.append(i)

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


# Plot comparison data for only 2 repeated block RAMS
plt.figure()
# Plot start only

plotmax = 5000

# Calculate and print mean and RMS of data in plotmax range
mu = meanVal(output_data[0:plotmax], plotmax)
rms = rmsVal(output_data[0:plotmax], plotmax, mu)
print("Mean = %0.2f" % mu)
print("RMS = %0.2f" % rms)

plt.plot(time[0:plotmax],output_data[0:plotmax], 'r-',label="Firmware output")
plt.legend(loc='best')
plt.ylabel('ADC counts', fontsize=12)
plt.tight_layout()
plt.savefig('%s/FW_output_startonly_%s' % (plot_dir,detector))
 
# Plot headers
output_headers = np.array(output_headers).T.tolist() # Transpose array
entries = np.arange(0, len(output_headers[0]),1)
for i in range(len(E_HDR_NAMES)):
    fig1, ax1 = plt.subplots()
    ax1.plot(entries,output_headers[i],'bo',label=E_HDR_NAMES[i])
    ax1.legend(loc='best')
    plt.xlabel('Triggers', fontsize=12)
    plt.tight_layout()
    plt.savefig('%s/FW_output_%s_%s.pdf' % (plot_dir,E_HDR_NAMES[i],detector))
    print(output_headers[i])
    
# Plot converted trigger time on same plot
clock200 = 1/(200e6)
clock75 = 1/(75e6)
time40 = [item * clock200 for item in output_headers[1]]
time75 = [item * clock75 for item in output_headers[2]]
fig1, ax1 = plt.subplots()
ax1.plot(entries,time40,'bo',label=E_HDR_NAMES[1])
ax1.plot(entries,time75,'ro',label=E_HDR_NAMES[2])
ax1.legend(loc='best')
plt.xlabel('Triggers', fontsize=12)
plt.ylabel('Trigger Time (s)', fontsize=12)
plt.tight_layout()
plt.savefig('%s/FW_output_clocks_%s.pdf' % (plot_dir,detector))

