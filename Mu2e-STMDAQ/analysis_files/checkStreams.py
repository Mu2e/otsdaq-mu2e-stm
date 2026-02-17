import numpy as np
import os
import sys


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
    print("Checking event: ", event)

    #totallen = headerlen + rawlen + zslen + phlen

    #checkedlen += totallen

    #check raw data matches in event and in raw stream
    for i in range(rawlen*2):
        if (data[i+headerlen+checkedlen] == dataraw[i]):
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
            continue
        else:
            for j in range(n_print_on_fail):
                print("ZS data | Event: ",j, "= ", data[j+checkedlen]," ZS stream: ", datazs[j])
            raise ValueError('ERROR: ZS data comparison failed')
        
    print("ZS data for this event: ok")
    checkedlen += zslen*2+(4*zsregions)


    checkedlen += phlen
    event += 1
    totalphlen += phlen


#check PH in streams is the right length (as exact values will be different)
if (totalphlen == len(dataph)):
     print("PH lengths for total data match")
else:
    print('Warning: PH lengths do not match, likely missing peaks!')



#print(dataph.hex(sep=' '))
