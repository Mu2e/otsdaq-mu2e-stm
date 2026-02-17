// Template module code
#include "Mu2e-STMDAQ/processing/prescale_raw.hh"
#include <random>

// Constructor
PrescaleRaw::PrescaleRaw(Config& cfg_,
			   const std::shared_ptr<AsyncLogger>& logger_,
			   const std::shared_ptr<STMdata>& stm_,
			   const int CHAN_) :
  cfg(cfg_), logger(logger_), stm(stm_), CHAN(CHAN_) {

  // Get the right channel sub-string
  std::string channel;
  if (CHAN == 0){
    channel = "stm.prescale.hpge.";
  }
  else if (CHAN == 1){
    channel = "stm.prescale.labr.";
  }

  // Instantiate any class variables in the constructor  
  raw_ps_size = cfg.getValue<double>(channel+"raw_prescale");
  logger->log("Prescale: Prescaling raw by " + std::to_string(raw_ps_size), 1);
  
  // Initialise function map for OperationManager
  functionMap["random_prescale"] = [this](std::shared_ptr<DataStruct>& buffer) {
    random_prescale(buffer);
  };
  functionMap["peak_based_prescale"] = [this](std::shared_ptr<DataStruct>& buffer) {
    peak_based_prescale(buffer);
  };
}

// First template function activated by operation manager
// and implemented by thread manager
void PrescaleRaw::random_prescale(std::shared_ptr<DataStruct>& buffer){
  
  // Get number of events
  size_t n_evts = buffer->raw_header_num;

  // Setup random number generator
  std::random_device rand;
  std::mt19937 gen(rand());
  std::uniform_real_distribution<double> unif_dist(0.0, 1.0);

  // If we are not prescaling set all prescale bools to 1
  if (raw_ps_size == 1) {
    std::fill(buffer->raw_ps_bool.begin(),buffer->raw_ps_bool.end(),1);
    return;
  }	

  // Loop over events and save if random number less than cut
  int ps_cut = 1/raw_ps_size;
  for (size_t iE=0; iE<n_evts; iE++) {
    if ( unif_dist(gen) < ps_cut ) buffer->raw_ps_bool[iE] = 1;
  }

}

void PrescaleRaw::peak_based_prescale(std::shared_ptr<DataStruct>& buffer){
  
  // Do something to buffer
  
}

