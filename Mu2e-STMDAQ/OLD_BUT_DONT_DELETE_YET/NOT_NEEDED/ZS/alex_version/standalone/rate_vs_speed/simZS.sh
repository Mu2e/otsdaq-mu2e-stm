for filename in /data1/claudia_simulation_binary_files/662keV_0.32mV/*genData*; do
    echo $filename
    for ((i=0; i<10; i++)); do
	./ZS_alex.exe $filename >> 662keV_0.32mV.txt
    done
done
