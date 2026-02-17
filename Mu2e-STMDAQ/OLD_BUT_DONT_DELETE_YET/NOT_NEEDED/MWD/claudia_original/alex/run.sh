source ../../setup.sh
./run_MWD.exe SupData662keV_80kHz.bin 400 200 10000 50000
echo "Checking for any differences with Claudia's output files..."
echo "--"
diff peaks.txt peaks_compare.txt
echo "--"
echo "If nothing between lines above, success!"
