#include "STMDAQ-TestBeam/utils/BinaryFile.hh"
#include "STMDAQ-TestBeam/utils/dbase.hh"
#include "STMDAQ-TestBeam/utils/xml.hh"
#include "STMDAQ-TestBeam/utils/EnvVars.hh"
#include "STMDAQ-TestBeam/MWD_Analysis/MWD.h"
#include "STMDAQ-TestBeam/MWD_Analysis/data.hh"
#include "STMDAQ-TestBeam/MWD_Analysis/stats.hh"

#include <string> 
#include <sstream>                                                                     
#include <vector>                                                                     

int main(int nargv, char** args){
	if (nargv != 2){
		std::cout << "ERROR: expecting one argument: the input filename" << std::endl;
		exit(-1);
	}
	std::string filename(args[1]);

	std::string xml_path = EnvVars::expand("${STM_XML}");
	std::cout << "XML_PATH = " << xml_path << std::endl;
	Xml *xml_file = new Xml(xml_path);
	// Get the clock frequencies to compute time offsets
	int trigger_time_clock = xml_file->int_value("stm.trigger_time_clock",13); // in MHz 
	int adc_offset_time_clock = xml_file->int_value("stm.adc_offset_time_clock",125); // in MHz 
	std::string mwd_filename = xml_file->value("stm.mwd_binary_filename"); 
	

	// Open this Binary file and extract run and subrun from raw filename
	BinaryFile *bf_in = new BinaryFile();
	bf_in->open_input_file(filename);
	int raw_filesize_header_type = xml_file->int_value("stm.raw_filesize_header_type",1); // 1: new header, 0: old 
        bf_in->set_raw_file_version(raw_filesize_header_type); 
	int run = bf_in->getRunfromFilename();
	int subrun = bf_in->getSubRunfromFilename();
	std::cout << "filename " << filename << " run = " << run << " subrun = " << subrun << std::endl;


	MWD *mwd = new MWD(xml_file);
	std::cout << mwd->print();
	DBase *db = new DBase("127.0.0.1",5432,DBase::READ);
	std::map<std::string, float> results = db->read_hpge_baseline(run,subrun);
	if (results.size() == 2) {
	  std::cout << "Mean for this run, subrun = " << results["mean"] << " and rms = " << results["rms"] << std::endl;
	}
	else if (results.size() == 0) {
	  std::cout << "*** No baseline values for this run, subrun" << std::endl;
	  exit(-1);
	}
	else {
	  std::cout << "*** Expected two values but got : " << results.size() << std::endl;
	  exit(-1);
	}

	std::vector<uint32_t> positions = bf_in->get_trigger_locations(true, true); // return array of file positions of external & internal triggers 
	if (positions.size() == 0){
	  std::cout << "No triggers in this file ..." << std::endl;
	}
	else {

	  // Open MWD output binary file ....
	  BinaryFile *bf_out = new BinaryFile();
	  bf_out->open_output_file(mwd_filename,run,subrun);
	  int number_of_triggers = positions.size();
	  bf_out->write_int(number_of_triggers);
	  std::cout << " Number of triggers being analysed = " << positions.size() << std::endl;
	  for (unsigned int i = 0; i < positions.size(); i++) {

	    std::cout << "Trigger offset in binary file = " << positions[i] << std::endl;
	    data* adc_data = bf_in->getData(positions[i]); // Get data for this trigger
	    uint32_t nadc_values = adc_data->nadc;
	    mwd->mwd_algorithm(adc_data);

	    double mean_baseline = (double) results["mean"];
	    double rms_baseline = (double) results["rms"];
	    int16_t* header = bf_in->getTriggerHeader(positions[i]);

	    bf_in->printi16a(header, 16);
	   

	    double time_offset = ((double) bf_in->getTriggerTime(header))/((double) trigger_time_clock) +
	      ((double) bf_in->getADC0Time(header))/((double) adc_offset_time_clock) ; // in us.
	    std::cout << "time_offset(trigger+ADC0) in us = " << time_offset << std::endl;
	    peaks* peak_data = mwd->find_peaks(mean_baseline, rms_baseline, time_offset);

	    peak_data->trigger_number = bf_in->getTriggerNumber(header);
	    peak_data->trigger_type = bf_in->getTriggerType(header);
	    peak_data->nadc_values = nadc_values;

	    bf_out->write_MWD_data(peak_data);
	    delete peak_data;
	  }
	  bf_out->close_output_file();
	  delete bf_out;
	}
	bf_in->close_input_file();
	db->close();
	delete xml_file;
	delete mwd;
	delete bf_in;
}
