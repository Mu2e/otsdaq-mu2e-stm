------------------------------------------------TESTING run00109.new.bin_00------------------------------------------------
-------> No Suppressed Data

run00109.new.bin_00

Number of elements in raw file: 536870912

RUN MWD ALGORITHM TO NO SUPPRESSED: /work/cgarcia/STMDAQ-TestBeam/build/bin/test_new.exe run00109.new.bin_00

Get energy peaks in: run00109.new_energypeaks_bin_00.root

Number of peaks: 2244 (many of them are noise)

Number of counts in the raw-data photopeak between -1200 and -1100: 267 

-------> Gradient Code: /work/cgarcia/STMDAQ-TestBeam/Claudia/Suppression_Algorithm/gradient_peaksuppression.cc

Compile: /work/cgarcia/STMDAQ-TestBeam/build/bin/gradient_peaksuppression.exe run00109.new.bin_00

Get: Number of elements in suppressed file: 12988974

Get the file with the suppressed data: suppresseddata_gradient.bin

RUN MWD ALGORITHM TO SUPPRESSED: /work/cgarcia/STMDAQ-TestBeam/build/bin/test_new.exe suppresseddata_gradient.bin 

Get energy peaks in: energypeakssuppressed_gradient.root

Number of triggers/peaks found: 1403

Number of counts in the raw-data photopeak between -1200 and -1100: 266

-------> Old Code: /work/cgarcia/STMDAQ-TestBeam/Claudia/Suppression_Algorithm/nonpeaksuppression_new.cc

Compile: /work/cgarcia/STMDAQ-TestBeam/build/bin/nonpeaksuppression_new.exe run00109.new.bin_00

Get: Number of elements in suppressed file: 92775583

Get the file with the suppressed data: run00109_suppressedsignal_bin_00.bin

RUN MWD ALGORITHM TO SUPPRESSED: /work/cgarcia/STMDAQ-TestBeam/build/bin/test_new.exe run00109_suppressedsignal_bin_00.bin

Get energy peaks in: energypeakssuppressed_oldcode.root

Number of triggers/peaks found: 2085

Number of counts in the sup-data photopeak between -1200 and -1100: 267

PLOT WITH: plotPeaksRawSup.C

------------------------------------------------TESTING run00109.bin------------------------------------------------
-------> No Suppressed Data

/work/cgarcia/DATA/MWD_Analysis/RUN109/run00109.bin

-------> Gradient Code: /work/cgarcia/STMDAQ-TestBeam/Claudia/Suppression_Algorithm/gradient_peaksuppression.cc

Compile: /work/cgarcia/STMDAQ-TestBeam/build/bin/gradient_peaksuppression.exe /work/cgarcia/DATA/MWD_Analysis/RUN109/run00109.bin 

Get: Number of elements in suppressed file: 

Get the file with the suppressed data: suppresseddata_all109run_gradient.bin

RUN MWD ALGORITHM TO NO SUPPRESSED: /work/cgarcia/STMDAQ-TestBeam/build/bin/test_new.exe suppresseddata_all109run_gradient.bin

Get energy peaks in: energypeakssuppressed_all109run_gradient.root

I DON'T UNDERSTAND WHY I GET A BAD RESULT WHEN RUNNING ALL THE BINARY FILE (run00109.bin) BUT THEN I GET A GOOD RESULT WHEN I RUN ALL THE FILE IN SPLITTED JOBS.

------------------------------------------------TESTING run00109.bin------------------------------------------------
-------> No Suppressed Data

/work/cgarcia/DATA/MWD_Analysis/RUN109/run00109.new.bin_00.......run00109.new.bin_19
