import sys
import os
import numpy as np
from numpy import * 

# PRINT OUTPUT
output = False

# Packet size [bytes]
PACKET_SIZE = 8198
# Packet length [no of int16_t entries]
PACKET_LEN = PACKET_SIZE / 2

# Fixed packet header length
P_HDR_LEN = 3
# Fixed event header length
E_HDR_LEN = 20;
# Event header variable names
E_HDR_NAMES = ["Channel",
              "40_MHz_Time",
              "75_MHz_Time",
              "Event_Number",
              "Event_Window_Tag_DTC",
              "Event_Mode_DTC",
              "Delivery_Ring_RF_Marker_DTC",
              "Event_Start_Offset",
              "Event_Length"]

# Boolean to indicate whether we're check all data
allData = True
# Block RAM size (portion of data file read in) [bytes]
RAM_SIZE = 65536 * 32 / 8
# Block RAM length [no of int16_t entries]
RAM_LEN = RAM_SIZE / 2
# Number of block RAMs to check (if not allData)
RAM_NUM = 2

# -------------------------------------
# Function to get the packet header
# -------------------------------------
def getPacketHdr(header):
    # Get the packet number
    pNum0 = header[0] & 0xFFFF
    pNum1 = header[1] & 0xFFFF
    packetNum = pNum1 << 16 | pNum0;
    if (output): print ("Packet number = ",packetNum)
    # Checksum
    if (header[2] != -18):
        print("Packet Header Error")
        exit(0)
    # Return the packet number
    return packetNum

# -------------------------------------
# Function to get the event header
# -------------------------------------
def getEventHdr(header):

    # Make unsigned
    header = header & 0xFFFF

    # Get the channel number
    Channel = header[0] & 0xFF;
    if (output): print("Channel = ",Channel)

    # Get 40 MHz event time
    TM40_0 = (header[0] >> 8) & 0xF;
    TM40_1 = header[1];
    TM40_2 = header[2];
    TM40_3 = header[3];
    TM40 = TM40_3 << 48 | TM40_2 << 32 | TM40_1 << 16 | TM40_0;    
    if (output): print("40 Mhz Time = ",TM40)

    # Get 75 MHz event time
    TM75_0 = header[4];
    TM75_1 = header[5];
    TM75_2 = header[6];
    TM75_3 = header[7];    
    TM75 = TM75_3 << 48 | TM75_2 << 32 | TM75_1 << 16 | TM75_0;    
    if (output): print("75 Mhz Time = ",TM75)

    # Get event number
    EN_0 = header[8];
    EN_1 = header[9];
    EN_2 = header[10];
    EN = EN_2 << 32 | EN_1 << 16 | EN_0;    
    if (output): print("Event Number = ",EN)

    # Get event window tag
    EWT_0 = header[11];
    EWT_1 = header[12];
    EWT_2 = header[13];
    EWT = EWT_2 << 32 | EWT_1 << 16 | EWT_0;    
    if (output): print("Event Window Tag = ",EWT)

    # Get event mode
    EM_0 = header[14];
    EM_1 = header[15];
    EM_2 = header[16] & 0xFF;
    EM = EM_2 << 32 | EM_1 << 16 | EM_0;    
    if (output): print("Event Mode = ",EM)

    # Get Delivery Ring RF Marker TDC
    DRM = header[16] >> 8 & 0xF;
    if (output): print("Delivery Ring RF Marker TDC = ",DRM)

    # Get event start offset
    ESO = header[17];
    if (output): print("Event Start Offset = ",ESO)

    # Get event length
    EL = header[18];
    if (output): print("Event Length = ",EL)

    # Check header ends with 0xBEEF
    if(header[19] != 0xBEEF):
        print("ERROR: Event header does not end with 0xBEEF!")
        print("Event Number = ",EN)
        print(header)
        exit(0)

    return Channel,TM40,TM75,EWT,EN,EM,DRM,ESO,EL

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

    # THIS SHOULDN"T BE HERE - DELETE LATER    
    splitPacket = False
    splitEvent = 0

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

        # Reset packet counter to zero
        packet_count = 0
        
        # -----------------
        # Packet header
        # -----------------
        
        # Get packet header
        pHdr = data[int(data_count):int(data_count)+int(P_HDR_LEN)]
        # Get the new packet number
        new_pNum = getPacketHdr(pHdr)
        # Check the packet number has incremented by 1 only
        if (new_pNum > 0 and (new_pNum - old_pNum) != 1):
            print("Error: Missing packet number!!")
            print("Previous packet number = ", old_pNum)
            print("New packet number = ", new_pNum)
            exit(0)
        # Store as the old packet number
        old_pNum = new_pNum
        # Increase counters by packet header length
        data_count += P_HDR_LEN
        packet_count += P_HDR_LEN
        
        # Loop over data in packet
        while(packet_count < PACKET_LEN):
            
            # -----------------
            # Event header
            # -----------------        
            
            # Get event header
            eHdr = data[int(data_count):int(data_count)+int(E_HDR_LEN)]
            eHdr = getEventHdr(eHdr);
            # Store event header in the event header array
            eHdr_data.append(eHdr)
            # Get the new event number
            new_eNum = eHdr[3]
            # Check the event number has not increased by more than 1
            if (new_eNum > 0 and (new_eNum - old_eNum) > 1):
                print("Error: Missing event number!!")
                exit(0)
            # Store as the old event number
            old_eNum = new_eNum
            # Increase counters by event header length
            data_count += E_HDR_LEN
            packet_count += E_HDR_LEN
            
            # -----------------
            # ADC data
            # -----------------        
            
            # Get event length
            eLen = eHdr[8]
            
            # Get the amount of data left in the packet
            leftInPacket = int(PACKET_LEN - packet_count)

            # Get the amount of data left in the packet
            eventToWrite = int(eLen)

            # Check if we're about to reach the data_max
            if (adc_count + eLen >= data_max):
                # Get the amount of remaining data to write
                eventToWrite = int(data_max - data_count)
                reached_max = True

            # THIS SHOULDN"T BE HERE - DELETE LATER
            if (splitPacket):
                eventToWrite = int(splitEvent)
                splitPacket = False
                splitEvent = 0
                
            # THIS SHOULDN"T BE HERE - DELETE LATER
            if (leftInPacket < eventToWrite):
                splitEvent = int(eLen - leftInPacket)
                eventToWrite = int(PACKET_LEN - packet_count)
                splitPacket = True
                
            # Get ADC data
            adc_data.extend(list(data[int(data_count):int(data_count)+eventToWrite]))
            
            # Increase counters by event length
            data_count += int(eventToWrite)
            adc_count += int(eventToWrite)
            packet_count += int(eventToWrite)
            
            # Break loop if data_max reached
            if (reached_max) :
                break

            # Check for 0xDEADBEEF (end of packet)
            isDeadBeef = True
            while(isDeadBeef and (int(data_count) != data_max)):
                beef = data[int(data_count)] & 0xFFFF
                dead = data[int(data_count)+1] & 0xFFFF
                deadbeef = dead << 16 | beef;
                # If 0xDEADBEEF found...
                if(deadbeef == 0xDEADBEEF):
                    # Increase counters by length of 0xDEADBEEF (2)
                    data_count += 2
                    packet_count += 2
                    # Else if not 0xDEADBEEF...
                else:
                    isDeadBeef = False

    # Return ADC data array
    return adc_data,eHdr_data

# -------------------------------------
# Main
# -------------------------------------

print ("The input data file is:",sys.argv[1])
input_data_file = sys.argv[1]
print ("The output data file is:",sys.argv[2])
output_data_file = sys.argv[2]

# Open input file
#with open('channel0_mockdata.bin', 'rb') as f:
with open(input_data_file, 'rb') as f:
    input_data = np.fromfile(f, dtype=np.int16, count=-1)

# Truncate input data to 1 block RAM (what Erdem repeats)
input_data_trunc = input_data[0:int(RAM_LEN)]

# Saving array as a file
toWrite = np.array(input_data_trunc)
toWrite.astype('int16').tofile('input_data_only256kb.bin')

# Open data file
#with open('channel0_subrun0.bin', 'rb') as f:
with open(output_data_file, 'rb') as f:
    # Get data from file
    data = np.fromfile(f, dtype=np.int16, count=-1)

# Truncate data to 1 block RAM (what Erdem repeats)
data_trunc = data[0:int(131072)]

# Saving array as a file
toWrite = np.array(data_trunc)
toWrite.astype('int16').tofile('fw_data_only256kb.bin')

# Input data  length
input_data_length = RAM_LEN

# Get output_data data
output_data = getADCdata(data)
output_headers = output_data[1]
output_data = output_data[0]

# Truncate output data to 1 block RAM (what Erdem repeats)
output_data_trunc = output_data[0:int(RAM_LEN)]

# Saving array as a file
toWrite = np.array(output_data_trunc)
toWrite.astype('int16').tofile('output_data_only256kb.bin')

# Output data length
output_data_length = len(output_data)
print("Output data has length: ", output_data_length)

# Calculate projected multiples of input data
input_multiple = output_data_length / input_data_length
print("256kb of input data should be repeated ", input_multiple, " times")

# Create array of multipled input data to compare
input_compare = [] # Initialise array
entries = 0 # Initialise no of entries for full multiples of input data

# Loop over the number of full multiples of input data
for i in range(int(input_multiple)):
    # Append the comparison array with the full input data array
    input_compare.extend(input_data_trunc[0:int(input_data_length)])
    # Increase the number of entries corresponding to a full multiple
    entries += int(input_data_length)
        
# Get the remainder of entries that remains to be added
remainder = output_data_length - entries
entries += remainder
# Append that portion of the input data
input_compare.extend(input_data_trunc[0:remainder])
                          
# Input data length
print("Multipled input data has length: ", entries, "/", len(input_compare)) 

#only 256kb of the input file is read in (65536 bytes * 32 bits)

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

# Calcuate time array
time = []
clock = 1/(300e6)
for i in range(output_data_length):
    #time.append(float(i*clock))
    time.append(i)

# Import matplotlib
from matplotlib import pyplot as plt
# Plot comparison data
plt.plot(time,output_data, 'r-',label="Firmware output")
plt.plot(time,input_compare, 'b-',label="Data file input (256kb repeated)")
if (len(time) <= len(input_data)):
    plt.plot(time,input_data[0:len(time)], 'g--',label="Data file input (raw)")
else:
    plt.plot(time[0:len(input_data)],input_data, 'g--',label="Data file input (raw)")
plt.legend(loc='best')
plt.ylabel('ADC counts', fontsize=12)
plt.tight_layout()
plt.savefig('%s/FW_output_compare.pdf' % plot_dir)

# Plot comparison data for only 2 repeated block RAMS
plt.figure()
# Find the length of 2 blocks RAMS
twoRAMS = int(2*RAM_LEN)
plt.plot(time[0:twoRAMS],output_data[0:twoRAMS], 'r-',label="Firmware output")
plt.plot(time[0:twoRAMS],input_compare[0:twoRAMS], 'b-',label="Data file input (256kb repeated)")
plt.plot(time[0:twoRAMS],input_data[0:twoRAMS], 'g--',label="Data file input (raw)")
plt.legend(loc='best')
plt.ylabel('ADC counts', fontsize=12)
plt.tight_layout()
plt.savefig('%s/FW_output_compare_2RAMsonly.pdf' % plot_dir)

# Plot shifted comparison data for only 2 repeated block RAMS
plt.figure()
# Create subplot
plt.subplot(2, 1, 1)
# Find the anchor (the INT16T_MAX value)
INT16T_MAX = 32767 
anchor = [i for i, n in enumerate(input_data) if n == INT16T_MAX][1]
# Find and print the shift in counts to match to the anchor
shift = anchor - output_data.index(INT16T_MAX)
print("The output data is shifted by ",shift," counts to match the input data...")
# Plot data
plt.plot(time[0+shift:twoRAMS],output_data[0:twoRAMS-shift], 'r-',label="Firmware output (shifted by %d counts)" % shift)
plt.plot(time[0:twoRAMS],input_compare[0:twoRAMS], 'g--',label="Data file input (raw)")
plt.legend(loc='best')
plt.ylabel('ADC counts', fontsize=12)
# Plot difference as a subplot
plt.subplot(2, 1, 2)
# Find the shift length
shift_length = twoRAMS-shift
# Find the difference between output and input
diff = []
for i in range(shift_length):
    out = output_data[i]
    inp = input_compare[i+shift]
    diff.append(out-inp)
# Plot data
plt.plot(time[0+shift:twoRAMS],diff, 'k-',label="Output - input")
plt.legend(loc='best')
plt.ylabel('Difference', fontsize=12)
plt.xlim([0,time[twoRAMS]])
plt.tight_layout()
plt.savefig('%s/FW_output_compare_2RAMsonly_shifted.pdf' % plot_dir)
    

# Plot headers
output_headers = np.array(output_headers).T.tolist() # Transpose array
entries = np.arange(0, len(output_headers[0]),1)
for i in range(len(E_HDR_NAMES)):
    fig1, ax1 = plt.subplots()
    ax1.plot(entries,output_headers[i],'bo',label=E_HDR_NAMES[i])
    ax1.legend(loc='best')
    plt.xlabel('Triggers', fontsize=12)
    plt.tight_layout()
    plt.savefig('%s/FW_output_%s.pdf' % (plot_dir,E_HDR_NAMES[i]))

# Plot converted trigger time on same plot
clock40 = 1/(40e6)
clock75 = 1/(75e6)
time40 = [item * clock40 for item in output_headers[1]]
time75 = [item * clock75 for item in output_headers[2]]
fig1, ax1 = plt.subplots()
ax1.plot(entries,time40,'bo',label=E_HDR_NAMES[1])
ax1.plot(entries,time75,'ro',label=E_HDR_NAMES[2])
ax1.legend(loc='best')
plt.xlabel('Triggers', fontsize=12)
plt.ylabel('Trigger Time (s)', fontsize=12)
plt.tight_layout()
plt.savefig('%s/FW_output_clocks.pdf' % plot_dir)

