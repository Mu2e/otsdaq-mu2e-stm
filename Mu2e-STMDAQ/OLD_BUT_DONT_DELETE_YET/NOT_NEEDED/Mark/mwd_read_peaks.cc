#include "STMDAQ-TestBeam/utils/BinaryFile.hh"
#include "STMDAQ-TestBeam/MWD_Analysis/MWD.h"
#include "STMDAQ-TestBeam/MWD_Analysis/peaks.hh"

#include <string> 
#include <sstream>                                                                     
#include <vector>                                                                     

int main(int nargv, char** args){
	if (nargv != 2){
		std::cout << "ERROR: expecting one argument: the input filename" << std::endl;
		exit(-1);
	}
	std::string filename(args[1]);
	// Open this Binary file and extract run and subrun from raw filename
	BinaryFile *bf_in = new BinaryFile();
	bf_in->open_input_file(filename);
	int run = bf_in->getRunfromFilename();
	int subrun = bf_in->getSubRunfromFilename();
	int num_triggers = bf_in->read_int();
	std::cout << "filename " << filename << " run = " << run << " subrun = " << subrun << " # triggers = " << num_triggers << std::endl;

	for (int i = 0; i < num_triggers; ++i){
	  peaks* peak_data = bf_in->read_MWD_data();
	  std::cout << "Trigger number = " << peak_data->trigger_number << " Trigger Type =  " << peak_data->trigger_type << " # peaks = " << peak_data->npeaks << " # ADC values = " << peak_data->nadc_values << std::endl;
	  if (peak_data->npeaks > 0) { 
	    std::cout << " First peak : energy = " << peak_data->peak_heights.at(0) << std::endl;
	    std::cout << " First peak : times = " << peak_data->peak_times.at(0) << std::endl;
	  }
	  delete peak_data;
	}
	bf_in->close_input_file();
	delete bf_in;
}
