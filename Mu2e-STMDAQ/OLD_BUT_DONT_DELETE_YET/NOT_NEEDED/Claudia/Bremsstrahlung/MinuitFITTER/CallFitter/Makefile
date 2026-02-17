ifndef STMDAQ_ROOT
$(error STMDAQ_ROOT not set, must source setup.sh)
endif

include $(STMDAQ_ROOT)/Makefile.inc

WARNING = -Wall -Wformat=2 -Wno-format-nonliteral -Wno-strict-aliasing -Wuninitialized -Wno-unused-function -Wextra -Wformat=2 -Wno-unused-parameter -Wno-missing-field-initializers -Wno-pointer-arith -Wno-unused-variable -Wno-unused-local-typedefs -Wno-parentheses -Wno-deprecated-declarations -Wno-conversion-null -Wno-unused-but-set-variable -Wno-type-limits -Wno-unused-but-set-parameter -Wno-catch-value -fPIC -Wno-deprecated

#CCOPTS = -O3 
CCOPTS = -Wall -Wno-unused-variable -ggdb -g3 -DUSE_GRAPHICS # turn on debugging information and graphics
#CCOPTS = -Wall -Wno-unused-variable -ggdb -g3 # turn on debugging information and no graphics 
#DEPOPTS = -MT $@ -MMD -MP -MF $(BUILD_DIR)/dep/$*.d

#MAIN_CC = FitterROOT.cc
#MAIN_CC = ReadRootFile_Fit.cc
#MAIN_CC = AddData_LaBrStatistics.cc
#MAIN_CC = CheckFitMethods.cc
#MAIN_CC = CheckFitMethods_GenData.cc
#MAIN_CC = Analyse_RecoTrueSignal.cc
#MAIN_CC = BremsstrahlungRooFit.cc
#MAIN_CC = SampleBackground_Fit.cc
#MAIN_CC = FitFromHistogram.cc
#MAIN_CC = FitFromHistogram_jobs.cc
#MAIN_CC = PlotSampledBackground.cc
#MAIN_CC = PlotDiffExp.cc
#MAIN_CC = SampleCalibration.cc
MAIN_CC = FitFromHistogram_simhisto.cc

MAIN_OBJ = $(patsubst %.cc, $(BUILD_DIR)/obj/%.o, $(MAIN_CC))
MAIN_EXE = $(patsubst %.cc, $(BUILD_DIR)/bin/%.exe, $(MAIN_CC))
INCLUDE_CC = -I$(BOOST_INC) -I$(PQXX_INC) -I$(STM_INC) -I$(ROOT_INC)
DEPFILES = $(patsubst %.cc, $(BUILD_DIR)/dep/%.d, $(MAIN_CC))

#.PHONY: all clean
#.PHONY: all 

all: $(MAIN_EXE)

clean:
	rm $(MAIN_OBJ) $(DEPFILES)

$(BUILD_DIR)/obj/%.o: %.cc Makefile $(BUILD_DIR)/dep/%.d | $(BUILD_DIR)/dep
	echo "<**Compiling**>" $<
	g++ $(CCOPTS) $(WARNING) $(DEPOPTS) $(INCLUDE_CC) -c $< -o  $@

$(BUILD_DIR)/bin/%.exe: $(BUILD_DIR)/obj/%.o
	echo "<**Linking**>"  $(notdir $@)
#	echo "g++ $(CCOPTS) $^ -o $@ $(STM_LIBS) $(IPBUS_LIBS) $(PQXX_LIBS) $(ROOT_LIBS) -lMinuit -lRooFit -lRooFitCore -L$(STMDAQ_ROOT)/build/lib -lMinuitFitter"
#	g++ $(CCOPTS) $^ -o $@ $(STM_LIBS) $(IPBUS_LIBS) $(PQXX_LIBS) $(ROOT_LIBS) -lMinuit -lRooFit -lRooFitCore -L$(STMDAQ_ROOT)/build/lib -lMinuitFitter
	echo "g++ $(CCOPTS) $^ -o $@ $(ROOT_LIBS) -lMinuit -lRooFit -lRooFitCore -L$(STMDAQ_ROOT)/build/lib -lMinuitFitter"
	g++ $(CCOPTS) $^ -o $@ $(ROOT_LIBS) -lMinuit -lRooFit -lRooFitCore -L$(STMDAQ_ROOT)/build/lib -lMinuitFitter -lUtils $(BOOST_LIBS)

$(DEPFILES):
include $(wildcard $(DEPFILES))
