import sys
import os
import math
import numpy as np
from numpy import *     

from analysis_funcs import *

# STM IERC pulse width = 150 ns
pulse_width = 45 # 150 ns in clocks ticks

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

# -------------------------------------
# Function to get the event header
# -------------------------------------
def getEventHdr(header,output):

    nheader = header[4:]
    
    # Get 75 MHz event time
    TM75 = np.uint64(0)
    TM75_0 = (nheader[0]).astype(np.uint16) & 0xFFFF;
    TM75_1 = (nheader[1]).astype(np.uint16) & 0xFFFF;
    TM75_2 = (nheader[2]).astype(np.uint16) & 0xFFFF;
    TM75_3 = (nheader[3]).astype(np.uint16) & 0xFFFF;    
    TM75 = TM75_3 << 48 | TM75_2 << 32 | TM75_1 << 16 | TM75_0;    
    TM75 = TM75/(75e6)*1e9
    if (TM75 < 0):
        print (TM75_0,int(TM75_0) & 0xFFFF)
        print("%d = %d << 48 | %d << 32 | %d << 16 | %d" % (TM75,TM75_3,TM75_2,TM75_1,TM75_0))
        exit(0)
    if (output): print("75 Mhz Time = ",TM75)

    # Get event number
    EN = np.uint64(0)
    EN_0 = ((nheader[4]).astype(np.uint16)).astype(np.uint64)
    EN_1 = ((nheader[5]).astype(np.uint16)).astype(np.uint64)
    EN_2 = ((nheader[6]).astype(np.uint16)).astype(np.uint64)
    EN = EN_2 << 32 | EN_1 << 16 | EN_0;    
    if (output): print("Event Number = ",EN)

    # Get event window tag
    EWT = np.uint64(0)
    EWT_0 = ((nheader[7]).astype(np.uint16)).astype(np.uint64)
    EWT_1 = ((nheader[8]).astype(np.uint16)).astype(np.uint64)
    EWT_2 = ((nheader[9]).astype(np.uint16)).astype(np.uint64)
    EWT = EWT_2 << 32 | EWT_1 << 16 | EWT_0
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
        print("Instead it is %d" % check)
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
def getADCdata(data,output,allData,maxEvents):

    # Define total event counter
    event_count=0    
    # Define total data count
    data_count=0
    # Define data count
    adc_count=0
    
    # Event header array
    eHdr_data = []

    # EWT
    EWT = np.uint64(0)
    old_EWT = np.uint64(0)
    
    # ADC data
    adc_data = []
    event_loc = []

    # Event data
    e_data = []

    # Loop over all data
    while(data_count < len(data)):

        # Break if acquired maximum data
        if (allData == False and event_count == maxEvents):
            print("Reached max number of events = %d = %d ADC values." % (event_count,adc_count))
            print("Exiting getADCdata...") 
            return eHdr_data,e_data,adc_data,event_loc
            break
    
        # -----------------
        # Event header
        # -----------------        
        # Get event header
        eHdr = data[int(data_count):int(data_count)+int(E_HDR_LEN)]
        eHdr = getEventHdr(eHdr,output);
        
        # Get the EWT
        old_EWT = EWT
        EWT = eHdr[1]
        if (EWT-old_EWT != 1):
            for i in range(EWT-old_EWT-1):
                missed = old_EWT + i + 1
                print("ERROR!: missing EWT number %d" % missed)
        
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
        
        # Get event data
        new_event = np.array(data[int(data_count):int(data_count)+eventToWrite])
        
        # Store data by event
        e_data.append([EWT,new_event])

        # Store ADC data
        adc_data.extend(data[int(data_count):int(data_count)+eventToWrite].tolist())
        
        # Increase counters by event length
        data_count += int(eventToWrite)
        adc_count += int(eventToWrite)

        event_loc.append([EWT,adc_count])

        # Increment the event counter
        event_count += 1        
        
    # Return event header and adc data
    return eHdr_data,e_data,adc_data,event_loc
