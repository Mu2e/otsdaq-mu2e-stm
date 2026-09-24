import sys
import BinaryFile as bf
import numpy as np 
import matplotlib.pyplot as plt
import argparse
import os
import datetime

def run_subrun_from_filename(filename):
        ic = filename.index('__run')
        run = int (filename[ic+5:ic+12])
        ic = filename.index('_subrun')
        subrun = int (filename[ic+7:ic+12])
        return run,subrun

def splitWord(word, debug = False):

        if debug:
                print(f"splitting word {word} of type {type(word)}")
        
        if word < 0: 
                word = word + 2**16
        
        fWord1 = word >> 12 
        fWord2 = word >> 8
        fWord3 = word
        
        if debug:
                print(f"shifted words {fWord1}, {fWord2}, {fWord3}")

        fWord1 = fWord1 & 0xF
        fWord2 = fWord2 & 0xF
        fWord3 = fWord3 & 0xFF

        #if debug:
        #        print(f"hexed words {fWord1}, {fWord2}, {fWord3}")
        #
        #fWord1 = int(fWord1)
        #fWord2 = int(fWord2)
        #fWord3 = int(fWord3)

        if debug:
                print(f"signed int words {fWord1}, {fWord2}, {fWord3}")

        #is this needed?
        for w in [fWord1, fWord2, fWord3]:
                if (w < 0):
                        w = w + 2**32

        if debug:
                print(f"unsigned int words {fWord1}, {fWord2}, {fWord3}")

        return fWord1, fWord2, fWord3

def combineWords(words, shifts, debug = False):
        
        if (debug): 
                print("combining ", words, "with shifts", shifts)

        i = 0
        for word, shift in zip(words,shifts):
                if word < 0: 
                        word = word + 2**16

                if (debug): 
                        print(f"adding 2**16 if negative, word: {word}")
                        print(f"shifting {word} by {shift} bits: {word << shift}")

                word = word << shift
                words[i] = word
                i+=1
   
        nwords = len(words)

        #if words > 1 OR them to combine
        combWord = words[0]
        for i in range(1, nwords):
                combWord = combWord | words[i]

        if (debug):
                print(f"signed combined word: {combWord} ")

        if (combWord < 0):
                if (nwords == 4):
                        combWord = combWord + 2**64
                elif (nwords == 2):
                        combWord = combWord + 2**32

        if (debug):
                print(f"unsigned combined word: {combWord}")
                
        return combWord

parser = argparse.ArgumentParser(description='')
parser.add_argument('--file', type=str, required=True, dest='filename', help='binary filename')
args = parser.parse_args()
filename = args.filename

# Get run and subrun from filename so as to determine which header format to use
print(filename)
run, subrun = run_subrun_from_filename(filename)
print("Run = %d, Subrun = %d" % (run,subrun))

#make backwards compatible (added header in after run 9)
if (run < 9):
        headerVersion = 1
else:
        headerVersion = 2

print("Using header version = %d" % (headerVersion))

trigNumsToPlot = [12045, 12073, 12087]#range(10)#range(100)#[22,23,24,25,122,123,124,125,601,602,603,604]

bff = bf.BinaryFile(filename)  

#load in dictionary which reads header file and gets format
import readBFHeader as bfh
bfhDict = bfh.getHeaderDict("sw_tHdr")
bfsDict = bfh.getHeaderDict("sw_sHdr")

print("\nsw trigger dict:", bfhDict)
print("\nsw slice dict:", bfsDict)

#useful to plot a simple counter
triggerIndex = []

#all of these are plotted against trigger num
plotDict = {
        "datasize" : []
        , "deadbeef" : []
        , "sNum" : []
        , "TrigNum" : []
        #, "MdChTp" : []
        , "TrigTime" : []
        , "ADCoffset" : []
        , "mode" : []
        , "channel" : []
        , "trigType" : []
        , "droppedPackets" : []
        , "unixTime" : []

}
slicePlotDict = {
        "slice_sNum" : [] # these won't work for multiple slices per trigger
        , "slice_sSize" : []
        , "slice_trigNum" : []
        , "slice_time" : []
}
#for i,v in bfhDict.items():
#        plotDict[i] = []

thSize = bfhDict["Size"]
thLength = bfhDict["Len"]

if headerVersion < 2:
        print ("reducing Header size by hand")
        thSize = thSize - 8
        thLength = thLength - 4

shSize = bfsDict["Size"]
shLength = bfsDict["Len"]

print("trigger header size: ", thSize)
print("slice header size: ", shSize)
#debug = False
debug = True

if (debug):
        print(bfhDict)

#set initial position to read from the start
pos = 0
iTrig = 0

#loop until you've reached end of file
startTime = 3650829445914
endTime = 0
numTrigs = [0,0];

while pos < bff.fsize:
        #if iTrig > 1:
        #        break
        bfhWords = {}

        if (debug):
                print ("\n\niTrig:", iTrig)
        #read header and populate dict

        for i,v in bfhDict.items():
                if i == "Size" or i == "Len":
                        continue
        
                #2 is the length in bits of a 16 bit word
                word = bff.read_data(pos + 2*v, 1)[0]
                bfhWords[i] = int(word)#int(str(word), 16)

        if (debug) :
                for i,v in bfhWords.items():
                        print('%s %d'%(i,v), hex(v & 0xFFFF) )

        #combine words
        dataSize = combineWords([bfhWords["dataSize1"], bfhWords["dataSize2"]], [0,16], debug )
        deadbeef = combineWords([bfhWords["0"], bfhWords["1"]], [0,16], debug )
        sNum = combineWords([bfhWords["sNum1"], bfhWords["sNum2"]], [0,16], debug )
        trigNum = combineWords([bfhWords["TrigNum1"], bfhWords["TrigNum2"]], [0,16], debug )
        trigTime = combineWords([bfhWords["TrigTime1"], bfhWords["TrigTime2"], bfhWords["TrigTime3"], bfhWords["TrigTime4"]], [0,16,32,48], debug )
        adcOffset = combineWords([bfhWords["ADCoffset1"], bfhWords["ADCoffset2"]], [0,16], debug )
        unixTime = 0
        if headerVersion > 1:

                uT1 = np.int16(bfhWords["unixTime1"])
                uT2 = np.int16(bfhWords["unixTime2"])
                uT3 = np.int16(bfhWords["unixTime3"])
                uT4 = np.int16(bfhWords["unixTime4"])

                unixTime = combineWords([bfhWords["unixTime1"], bfhWords["unixTime2"], bfhWords["unixTime3"], bfhWords["unixTime4"]], [0,16,32,48], debug )
                #unixTime = combineWords([uT1, uT2, uT3, uT4], [0,16,32,48], debug )

        adcClock = (320.0520833313*1E6) #Hz
        trigTime_s = trigTime * 1/(13E6)

        #put adcoffset in correct units...
        adcOffset_s = adcOffset * 8E-9 #125MHz

        #single words
        droppedPackets = combineWords([bfhWords["DroppedPackets"]], [0], debug )
        mode, channel, trigType = splitWord(bfhWords["MdChTp"], debug )

        plotDict["datasize"].append(dataSize)
        plotDict["deadbeef"].append(deadbeef)
        plotDict["sNum"].append(sNum)
        plotDict["TrigNum"].append(trigNum)
        plotDict["TrigTime"].append(trigTime_s)
        plotDict["ADCoffset"].append(adcOffset)

        plotDict["mode"].append(mode)
        plotDict["channel"].append(channel)
        plotDict["trigType"].append(trigType)
        plotDict["droppedPackets"].append(droppedPackets)
        plotDict["unixTime"].append(unixTime)

        if (unixTime < startTime):
                startTime = unixTime
        if (unixTime > endTime):
                endTime = unixTime
        print("TrigType, Mode = %d, %d" % (trigType, mode))
        numTrigs[mode] = numTrigs[mode] + 1

        print("Trignum, unixTime = %d, %d" % (trigNum, unixTime))
        #only plot the ADC values for certain triggers- mode determines internal or external triggers
        plotSlice = False

        #print(f"checking if {trigNum} is in {trigNumsToPlot} and {mode} is 1")
        print (trigNum)
        if trigNum in trigNumsToPlot: # and mode == 1:
                plotSlice = True
                print("PLOTTING ADC values for trigger", trigNum, "with trig mode", mode)

        #read slice header: start from pos (position of trigger header start, + trigger header size)
        slicepos = pos + thSize

        if debug:
                print("starting read from position", slicepos, "for", trigNum)

        sliceADC = []
        sliceTime = []
        for slice in range(sNum):
                bfsWords = {}
                for i,v in bfsDict.items():
                        if i == "Size" or i == "Len":
                                continue
        
                        #2 is the length in bits of a 16 bit word
                        word = bff.read_data(slicepos + 2*v, 1)[0]
                        bfsWords[i] = int(word)#int(str(word), 16)

                if (plotSlice and debug) :
                        for i,v in bfsWords.items():
                                print('%s %d'%(i,v), hex(v & 0xFFFF) )

                #combine words
                sliceNum = combineWords([bfsWords["sNum1"], bfsWords["sNum2"]], [0,16], debug )
                sSize = combineWords([bfsWords["sSize1"], bfsWords["sSize2"]], [0,16], debug )
                adcTime = combineWords([bfsWords["ADCtime1"], bfsWords["ADCtime2"], bfsWords["ADCtime3"], bfsWords["ADCtime4"]], [0,16,32,48], debug )

                print("adcTime:", adcTime)

                #get adcTime in right units
                adcTime_s = trigTime_s + adcOffset_s + (adcTime* 76.92E-9)

                print ("\n\ntrigTime:", trigTime_s, "adcOffset:", adcOffset_s, "adcTime", adcTime_s, "mode",mode ,"\n")

                #for plotting later: this will not work when there is more than one slice, as it is plotted against trigger num
                slicePlotDict["slice_sNum"].append(sliceNum)
                slicePlotDict["slice_sSize"].append(sSize)
                slicePlotDict["slice_trigNum"].append(trigNum) #could be many entries for a single trigNum
                slicePlotDict["slice_time"].append(adcTime_s) 

                #plot ADC data (move along header size in bytes)
                datapos = slicepos + shSize

                if plotSlice:
                        if debug:
                                print("sliceNum:", sliceNum,"sSize:", sSize, "adcTime:", adcTime, "type", type(adcTime))
                                print("getting data from position", datapos, "for trigger", trigNum, "and slice", sNum, "size", sSize, "dataSize:", dataSize)
                                
                        #divide by 2 to get number of int 16
                        data = bff.read_data(datapos, sSize/2) #HACK
                        clockTick = 1.0/(adcClock); # seconds
                        dataLength = len(data)

                        dataTime = np.zeros(dataLength)#, dtype = np.uint64)
                        for i,v in enumerate(data):
                                dataTime[i] = adcTime_s + i*clockTick
                                #dataTime[i] = (i*clockTick)
                        
                                #add to slice data
                                sliceADC.append(v)
                                sliceTime.append(dataTime[i]*1E6) #add to the array in us
                                
                                #print("adctime in seconds", adcTime_s, "dataTime[0]", dataTime[0])
                                #exit()

                                if i < 2 and debug:
                                        print(f"TIME CHECK dataTime: {dataTime[i]} {type(dataTime[i])} sliceTime: {sliceTime[i]} {type(sliceTime[i])} adcOffset:{adcTime} i*clockTick: {i*clockTick}")
                #set position to itself header size plus slice size
                slicepos += shSize + sSize
        
        #end loop over slices
        if plotSlice :
                if debug:
                        print("array size", len(sliceTime))
                        print("last 10 values:")
                        for j in range(10):
                                print("dataTime:", sliceTime[-j], "ADC:", sliceADC[-j], "type:", type(sliceADC[-j]))

                fig, ax = plt.subplots()
                
                ax.set_xlabel("ADC Time [us]")
                ax.set_ylabel("ADC Value")

                ax.scatter(np.array(sliceTime), np.array(sliceADC))
                
                
                toplim = sliceTime[-1]
                if (len(sliceTime) > 2E2):
                        toplim = sliceTime[200];
                print("setting xlimits from", sliceTime[0], "to", toplim, "total length:", toplim - sliceTime[0])
                ax.set_xlim(sliceTime[0], toplim)                

                #ax.set_ylim(-500, 5000)

                fname = os.path.basename(filename) 
                fname_png = fname.replace(".bin", "_ADCfor" + str(trigNum) + ".png")
                plt.savefig(fname_png)
                plt.close()

        #now move on to next trigger
        pos += thSize + dataSize
        triggerIndex.append(iTrig)
        iTrig+=1


print_sql = 1
s1 = datetime.datetime.fromtimestamp(startTime/1000.0).strftime('%Y-%m-%d %H:%M:%S')
s2 = datetime.datetime.fromtimestamp(endTime/1000.0).strftime('%Y-%m-%d %H:%M:%S')
print("Time of first trigger = %s, Time of last trigger = %s\n\n" %(s1, s2))
if (print_sql == 1):
        fname = os.path.basename(filename)
        print("insert into runinfo (run,start_time,end_time,comment) values (%d,'%s','%s','LaBr data: same DAQ as HPGe run=28');" % (run,s1,s2))
#        print("update runinfo set start_time = '%s' where run = %d;" % (s1,run))
#        print("update runinfo set end_time = '%s' where run = %d;" % (s2,run))
        print("insert into run_subrun_times (run,subrun,start_time,end_time,num_int_trigs,num_ext_trigs,file_name,file_format) values (%d,%d,'%s','%s',%d,%d,'%s',%d);" % (run,subrun,s1,s2,numTrigs[0],numTrigs[1],fname,headerVersion))
        sys.exit()

print("\nPLOTTING trigger header info...")

plt.ion()
fig, ax = plt.subplots()

ax.set_xlabel("Trigger Time")
ax.set_ylabel("Trigger Index")
ax.scatter(plotDict["TrigTime"],triggerIndex)

# Save to png
fname = os.path.basename(filename)
fname_png = fname.replace(".bin", "TriggerTimeAndIndex.png")
plt.savefig(fname_png)

#plot all of them vs TrigNum
for i,v in plotDict.items():
        fig, ax = plt.subplots()
        ax.set_xlabel("Trigger Number")
        ax.set_ylabel(i)
        ax.scatter(plotDict["TrigNum"][:50],v[:50])
        
        fname = os.path.basename(filename) 
        fname_png = fname.replace(".bin", "_" + i + ".png")
        plt.savefig(fname_png)

#plot slice vs trig num (filled per slice)
for i,v in slicePlotDict.items():
        fig, ax = plt.subplots()
        ax.set_xlabel("Trigger Number")
        ax.set_ylabel(i)
        ax.scatter(slicePlotDict["slice_trigNum"],v)
        
        fname = os.path.basename(filename) 
        fname_png = fname.replace(".bin", "_" + i + ".png")
        plt.savefig(fname_png)

bff.close_file()
input("....")
