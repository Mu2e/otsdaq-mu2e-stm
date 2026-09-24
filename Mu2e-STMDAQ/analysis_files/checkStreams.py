import numpy as np
import os
import sys
from matplotlib import pyplot as plt



def twos_comp(val, bits):
    """compute the 2's complement of int value val"""
    if (val & (1 << (bits - 1))) != 0: # if sign bit is set e.g., 8bit: 128-255
        val = val - (1 << bits)        # compute negative value
    return val                         # return positive value as is

# Check arguments
if len(sys.argv) != 5:
    print("Usage: python checkStreams.py <events_file> <raw_file> <zs_file> <ph_file>")
    sys.exit(1)

# Get arguments
events_file = sys.argv[1]
raw_file = sys.argv[2]
zs_file = sys.argv[3]
ph_file = sys.argv[4]

with open(events_file, 'rb') as f:
    data = f.read()

with open(raw_file, 'rb') as fraw:
    dataraw = fraw.read()

with open(zs_file, 'rb') as fzs:
    datazs = fzs.read()

with open(ph_file, 'rb') as fph:
    dataph = fph.read()


#data = data*5
#dataraw = dataraw*5
#datazs = datazs*5
#dataph = dataph*5



'''
with open('/data/dominika/mu2estm_HPGe_events_2026-02-10_10-14-42_subrun0.bin', 'rb') as f:
    data = f.read()

with open('/data/dominika/mu2estm_HPGe_raw_2026-02-10_10-14-42_subrun0.bin', 'rb') as fraw:
    dataraw = fraw.read()

with open('/data/dominika/mu2estm_HPGe_zs_2026-02-10_10-14-42_subrun0.bin', 'rb') as fzs:
    datazs = fzs.read()

with open('/data/dominika/mu2estm_HPGe_ph_2026-02-10_10-14-42_subrun0.bin', 'rb') as fph:
    dataph = fph.read()

with open('data.bin', 'rb') as f: 
    data = f.read() 

with open('data_raw.bin', 'rb') as fraw:
    dataraw = fraw.read()

with open('data_ZS.bin', 'rb') as fzs:
    datazs = fzs.read()

with open('data_PH.bin', 'rb') as fph:
    dataph = fph.read()
'''

n_print_on_fail = 5

rawlen = 0
zslen = 0
zsregions = 0
phlen = 0
totalphlen = 0
headerlen = 42
checkedlen = 0
event = 0;
zsdata = []


#check the events file has at least one cafe header
if (data[0] == 254 and data[1] == 202):
    print("Found first 0xCAFE")

else:
    raise ValueError('ERROR: no headers in this file')

while (checkedlen < len(data)):

    #get info from the first header 
    npeaks = (data[checkedlen+39]<<8) | data[checkedlen+38]
    phlen = npeaks*4
    zslen = (data[checkedlen+37]<<8) | data[checkedlen+36]
    zsregions = (data[checkedlen+35]<<8) | data[checkedlen+34]
    rawlen = (data[checkedlen+33]<<8) | data[checkedlen+32]
    zsdata = []
    rawdata = []
    phdata = []

    print("Checking event: ", event)
 #   print(data[checkedlen+1])

    #totallen = headerlen + rawlen + zslen + phlen

    #checkedlen += totallen

    #check raw data matches in event and in raw stream
    for i in range(rawlen*2):
        if (data[i+headerlen+checkedlen] == dataraw[i]):
            rawdata.append(data[i+headerlen+checkedlen])
            if i < 100:
                print(dataraw[i])
            continue
        else:
            for j in range(n_print_on_fail):
                print("Raw data | Event: ",j, "= ", data[j+headerlen+checkedlen]," Raw stream: ", dataraw[j])
            raise ValueError('ERROR: raw data comparison failed')

    print("Raw data for this event: ok")
    checkedlen += headerlen+rawlen*2

    #check zs and zs headers in streams
    for i in range(zslen*2+(4*zsregions)):
        #print(i+headerlen+rawlen
        if (data[i+checkedlen] == datazs[i]):
            zsdata.append(datazs[i])
            continue
        else:
            for j in range(n_print_on_fail):
                print("ZS data | Event: ",j, "= ", data[j+checkedlen]," ZS stream: ", datazs[j])
            raise ValueError('ERROR: ZS data comparison failed')
        
    print("ZS data for this event: ok")
    checkedlen += zslen*2+(4*zsregions)


    for i in range(phlen):        
        phdata.append(data[i+checkedlen])
            



    checkedlen += phlen
    event += 1
    totalphlen += phlen

#    print(rawlen*2)
#    print(zslen*2+(4*zsregions))
#    print(phlen)
#    print(checkedlen)
#    print(len(data))

    #plot all the ZS regions with raw/PH overlaid
    #print(zsdata)
    zsparsed = []
    rawparsed = []
    phparsed = []
    time = []
    time2 = []
    zstotallen = 0
    bonuslen = 20

    for i in range(zsregions):
        starttime = (zsdata[1+zstotallen]<<8) | zsdata[0+zstotallen]
        length = (zsdata[3+zstotallen]<<8) | zsdata[2+zstotallen]
        #print(starttime)
        #print(length)
        plotname = str(starttime)+".png"

        for j in range(length):
            val = twos_comp((zsdata[2*j+5+zstotallen]<<8) | zsdata[2*j+4+zstotallen],16)
           # val_raw = twos_comp((rawdata[starttime*2+1+2*j-headerlen]<<8) | rawdata[starttime*2+2*j-headerlen],16)

            zsparsed.append(val)
            #print(val_raw)
           # rawparsed.append(val_raw)
            time.append(starttime+j)

    

        for j in range(length+bonuslen*2):
            #val = twos_comp((zsdata[2*j+5+zstotallen]<<8) | zsdata[2*j+4+zstotallen],16)
            val_raw = twos_comp((rawdata[(starttime-bonuslen)*2+1+2*j-headerlen]<<8) | rawdata[(starttime-bonuslen)*2+2*j-headerlen],16)

           # zsparsed.append(val)
            #print(val_raw)
            rawparsed.append(val_raw)
            time2.append(starttime-bonuslen+j)

        for j in range(int(phlen/2)):
            val_ph = twos_comp((phdata[2*j+1]<<8) | phdata[2*j],16)
            phparsed.append(val_ph)





        #print(zsparsed)
        #print(phparsed)
        #print(len(rawdata))
        #print(len(dataraw))

        plt.plot(time2,rawparsed)
        plt.plot(time,zsparsed)
    
        for j in range(int(phlen/4)):
            pos = phparsed[2*j]
            peakh = -1*phparsed[2*j+1]
            if (pos >= time2[0] or pos < time2[-1]):
                plt.plot([pos],[peakh],marker = "X", markersize = 3, color="k")




        plt.xlim(time2[0],time2[-1])
        plt.savefig(plotname)
        plt.close()

        zsparsed = []
        time = []
        time2 = []
        rawparsed = []
        phparsed = []
        zstotallen += length*2+4




'''
    for i in range(int(len(zsdata)/2)):
        val = twos_comp((zsdata[2*i+1]<<8) | zsdata[2*i],16)
        
        #read the zs header
        if i < lastlen:
            laststart = val
        else:
      
            print(val)
#        if i = lastlen:
#            lastlen = val
#            print(val)
   
'''
        

        


#check PH in streams is the right length (as exact values will be different)
if (totalphlen == len(dataph)):
     print("PH lengths for total data match")
else:
    print('Warning: PH lengths do not match, likely missing peaks!')










#print(dataph.hex(sep=' '))
