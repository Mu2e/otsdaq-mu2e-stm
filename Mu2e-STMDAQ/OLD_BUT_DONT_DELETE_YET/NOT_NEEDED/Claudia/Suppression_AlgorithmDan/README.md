-------> No Suppressed Data

run00109.new.bin_00

Number of elements in raw file: 536870912

RUN MWD ALGORITHM TO NO SUPPRESSED: /work/cgarcia/STMDAQ-TestBeam/build/bin/test_new.exe run00109.new.bin_00

Get energy peaks in: run00109.new_energypeaks_bin_00.root

Number of peaks: 2244 (many of them are noise)

Number of counts in the raw-data photopeak: 267 

-------> Dan's Code: /work/cgarcia/STMDAQ-TestBeam/Claudia/Suppression_AlgorithmDan/nonpeaksuppression_newDan.cc

Compile: /work/cgarcia/STMDAQ-TestBeam/build/bin/nonpeaksuppression_newDan.exe run00109.new.bin_00

Get: Number of elements in suppressed file: 181488000

Get the file with the suppressed data: run00109_suppressedsignal_bin_00Dan.bin

RUN MWD ALGORITHM TO SUPPRESSED: /work/cgarcia/STMDAQ-TestBeam/build/bin/test_new.exe run00109_suppressedsignal_bin_00Dan.bin

Get energy peaks in: energypeakssuppressed_Dan.root

Number of triggers/peaks found: 1917

Number of counts in the sup-data photopeak between -1200 and -1100: 257
