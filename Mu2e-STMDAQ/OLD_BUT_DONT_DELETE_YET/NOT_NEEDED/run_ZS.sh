source setup.sh
NAME=$1
if [ ${NAME} == "claudia" ]
then 
    cp $STMDAQ_ROOT/ZS/${NAME}_version/CallZS/CallZS.cc $STMDAQ_ROOT/ZS
elif [ $NAME == "alex" ]
then
    cp $STMDAQ_ROOT/ZS/${NAME}_version/CallZS.cc $STMDAQ_ROOT/ZS
elif [ $NAME == "george" ]
then
    cp $STMDAQ_ROOT/ZS/${NAME}_version/CallZS.cc $STMDAQ_ROOT/ZS
else
    echo "Name \"$NAME\" not recognised!"
    return
fi
cp $STMDAQ_ROOT/ZS/${NAME}_version/ZS.cc $STMDAQ_ROOT/ZS
cp $STMDAQ_ROOT/ZS/${NAME}_version/ZS.hh $STMDAQ_ROOT/ZS
make
time ./build/bin/CallZS.exe /data1/run00109.new.bin_00
cp output.bin output_$NAME.bin
cp output.dat output_$NAME.dat
if [ $NAME == "alex" ]
then
    if [ -f "output_claudia.bin" ]; then
	echo "Checking for any differences with Claudia's output files..."
	echo "--"
	diff output_alex.bin output_claudia.bin
	#diff output_alex.bin output_claudia.bin
	echo "--"
	echo "If nothing between lines above, success!"	
    else
	echo "DIFF ERROR: Can't find Claudia's output files. Run code as \"claudia\" first and then re-run as \"alex\"."
    fi
fi
if [ $NAME == "george" ]
then
    if [ -f "output_claudia.bin" ]; then
	echo "Checking for any differences with Claudia's output files..."
	echo "--"
	diff output_george.bin output_claudia.bin
	#diff output_alex.bin output_claudia.bin
	echo "--"
	echo "If nothing between lines above, success!"	
    else
	echo "DIFF ERROR: Can't find Claudia's output files. Run code as \"claudia\" first and then re-run as \"george\"."
    fi
fi
