#include "STMDAQ-TestBeam/utils/BinaryFile.hh"
#include "STMDAQ-TestBeam/MWD_Analysis/MWD.h"
#include "STMDAQ-TestBeam/MWD_Analysis/data.hh"
#include "STMDAQ-TestBeam/MWD_Analysis/peaks.hh"

#include <string> 
#include <sstream>                                                                     
#include <vector>                                                                     

int main(int nargv, char** args){
  if (nargv != 2){
    std::cout << "ERROR: expecting one argument: the output binary filename" << std::endl;
    exit(-1);
  }

  std::string mwd_filename(args[1]);
  BinaryFile *bf_out = new BinaryFile();
  bf_out->open_output_file(mwd_filename,99,99);
  unsigned int num_triggers = 2;
  bf_out->write_int(num_triggers);
  int nadc_values = 0;
  for (unsigned int i = 0; i < num_triggers; i++) {
    peaks* peak_data = new peaks();
    uint32_t trigger_number = (uint32_t) i;
    uint16_t trigger_type = 72;
    int npeaks = 100*(i+1);	 
    std::vector<double> peak_heights;
    std::vector<double> peak_times;
    for (int i = 0 ; i < npeaks; i++){
      double ph = 10*i;
      double pt = 100*i;
      peak_heights.push_back(ph);
      peak_times.push_back(pt);
    }
    peak_data->npeaks = npeaks;
    peak_data->peak_heights = peak_heights;
    peak_data->peak_times = peak_times;
    peak_data->nadc_values = 150.0;
    peak_data->trigger_number = trigger_number;
    peak_data->trigger_type = trigger_type;
    bf_out->write_MWD_data(peak_data);
    delete peak_data;
  }
  bf_out->close_output_file();
  delete bf_out;
}
