#ifndef FORM_EVENTS_hh
#define FORM_EVENTS_hh

// Async Logger code
#include "Mu2e-STMDAQ/utils/async_logger.hh"
// STM data header
#include "Mu2e-STMDAQ/config/stm_data.hh"
// Operations base header
#include "Mu2e-STMDAQ/processing/operations_base.hh"

class FormEvents : public OperationMap {

private:

  // Async Logger
  const std::shared_ptr<AsyncLogger>& logger;

  // STM data info
  const std::shared_ptr<STMdata>& stm;

  uint64_t tot_adc_count = 0;
  
  // First event?
  bool first_event = true;

  // Has a signal buffer call ended on a complete event
  bool end_on_complete = false;

  // Have we detected a null heartbeat?
  bool null_hb = false;
  
  // Incrementing packet counter
  size_t packet_count = 0;
  
  // Incrementing EWT counter
  size_t EWT_count = 0;
  
  // Event counter
  size_t current_event_count = 0;

  // Current EWT
  size_t current_EWT = 0;

  // Current event length (ADC data only)
  size_t current_event_len = 0;

  // The current event header
  sw_event_header eHdr_to_copy;

  // The current event data to copy
  std::vector<int16_t> event_to_copy;

  // Max number of events in a buffer
  size_t max_event_num;
  
public:

  // Constructor
  FormEvents(const std::shared_ptr<AsyncLogger>& logger_,
	     const std::shared_ptr<STMdata>& stm_);
  
  // Destructor                                                          
  ~FormEvents() {
    // Log to user
    logger->log("FormEvents formed " + std::to_string(packet_count) +
                " UDP packets into " + std::to_string(EWT_count) +
                " events containing " + std::to_string(tot_adc_count) +
                " ADC values.",1);
    std::cout << "FormEvents destructor called.\n";
  }

  // Get events from packet data
  void get_events(std::shared_ptr<DataStruct>& buffer);

  // Setup new event
  void new_event(int16_t* data_ptr, size_t hdr_start_loc,
                             uint64_t EWT, size_t event_len);
  
  // Store complete event
  int store_event(std::shared_ptr<DataStruct>& buffer, size_t& adc_count);

  // Insert header for missing EWTs
  void insert_missing_ewts(std::shared_ptr<DataStruct>& buffer);
  
};

#endif
