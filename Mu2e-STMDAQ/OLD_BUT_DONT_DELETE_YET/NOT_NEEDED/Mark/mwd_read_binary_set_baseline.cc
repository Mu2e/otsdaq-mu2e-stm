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
	std::string xml_path = EnvVars::expand("${STM_XML}");
	std::cout << "XML_PATH = " << xml_path << std::endl;
	Xml *xml_file = new Xml(xml_path);

	std::string filename(args[1]);
	// Open this Binary file and extract run and subrun from raw filename
	BinaryFile *bf = new BinaryFile();
	bf->open_input_file(filename);
	int raw_filesize_header_type = xml_file->int_value("stm.raw_filesize_header_type",1); // 1: new header, 0: old 
        bf->set_raw_file_version(raw_filesize_header_type); 

	int run = bf->getRunfromFilename();
	int subrun = bf->getSubRunfromFilename();
	std::cout << "filename " << filename << " run = " << run << " subrun = " << subrun << std::endl;


	MWD *mwd = new MWD(xml_file);
	std::cout << mwd->print();
	DBase *db = new DBase("127.0.0.1",5432,DBase::WRITE);

	std::vector<uint32_t> positions = bf->get_trigger_locations(false, true); // return array of file positions of internal triggers 
	//std::vector<uint32_t> positions = bf->get_trigger_locations(true, false); // return array of file positions of external triggers 
	if (positions.size() == 0){
	  std::cout << "No internal triggers in this file ..." << std::endl;
	}
	else {
	  std::vector<double> baseline_values_subrun_mean;
	  std::vector<double> baseline_values_subrun_rms;
	  for (unsigned int i = 0; i < positions.size(); i++) {
	    //std::cout << "Internal trigger offset in binary file = " << positions[i] << std::endl;
	    data* adc_data = bf->getData(positions[i]); // Get data for this trigger
	    mwd->mwd_algorithm(adc_data);
	    std::vector<double> baseline_values = mwd->calculate_baseline();
	    //std::cout << "Baseline mean = " << baseline_values.at(0) << " rms = " << baseline_values.at(1) << std::endl;
	    baseline_values_subrun_mean.push_back(baseline_values.at(0));
	    baseline_values_subrun_rms.push_back(baseline_values.at(1));

	    if (i < 3){
	      std::stringstream ss; 
	      ss << "claudia_run_" << run << "_lvalues_int_" << i << ".bin";
	      mwd->write_l_to_binary_file(ss.str());
	      ss.str(""); ss.clear();
	      ss << "claudia_run_" << run << "_adc_values_int_" << i << ".bin";
	      mwd->write_adc_to_binary_file(ss.str(),adc_data);
	    }
	  }
	  // Get average of mean and RMS arrays ie.baseline_values_subrun_mean, baseline_values_subrun_rms
	  double mean_baseline = stats::mean(baseline_values_subrun_mean);
	  double mean_rms_baseline = stats::mean(baseline_values_subrun_rms);
	  db->write_hpge_baseline(run,subrun, mean_baseline, mean_rms_baseline); // run,subrun,mean,rms
	}
	db->close();
	delete xml_file;
	delete mwd;
	delete bf;
}
