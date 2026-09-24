while [ 1 ]
do
echo | ./build/bin/run_eventSim.exe 1 10000 1
sleep 0.2s
done

# syntax is (number of events per burst) (length of event in ns) (prescale)
# sampling matches data so 100ns = 30 raw adcs 
# prescale does nothing for now 
