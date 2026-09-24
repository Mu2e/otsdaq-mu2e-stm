To run continuous tcp sending of sim, run setup.sh, build if needed, then run run_sim.sh with no parameters. 
To run a single burst, ./build/bin/run_eventSim.exe (number of events per burst) (length of event in ns) (prescale)
Printout 'connected to reciever' indicates it's actually found the port of the reciever.

Rate is controlled by # of events and sleep time in that script. 


TCP single stream is int_16s, event by event, with all three streams and headers combined as they are in the DAQ.  Sending is handled in the sendTCP module.
A lot of the header info is just placeholders, but the EWT is an incrementing int and the actual event info should all be accurate. 

------------------------------------------------

If you just want output bin files, turn the sendTCP bool off in run_eventSim.cc and turn on the relevant write streams.

------------------------------------------------

Other sim params are a bit scattered:

- ZS window length is set to 20 adcs somewhat arbitrarliy. Have now implemented overlaps to combine into one total ZS region.
- actual peak shape is currently the same for all, with amplitude -1000. # of peaks per event is random up to a user-set maximum in gen_eventData.cc, edit that for peak rate.  
