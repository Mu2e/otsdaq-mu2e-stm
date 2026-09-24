#ifndef EVENT_BUILDER_hh
#define EVENT_BUILDER_hh

// Async Logger code
#include "Mu2e-STMDAQ/utils/async_logger.hh"
// STM data header
#include "Mu2e-STMDAQ/config/stm_data.hh"
// Data struct code
#include "Mu2e-STMDAQ/buffers/data_struct.hh"


class EventBuilder {

private:

  // Async Logger
  const std::shared_ptr<AsyncLogger>& logger;

  // STM data info
  const std::shared_ptr<STMdata>& stm;

public:
  
  // Constructor
  EventBuilder(const std::shared_ptr<AsyncLogger>& logger_,
                const std::shared_ptr<STMdata>& stm_);
  
  // Destructor                                                          
  ~EventBuilder() {
    std::cout << "EventBuilder destructor called.\n";
  }

  // Build the event/EWT
  size_t build_event(std::shared_ptr<DataStruct>& buffer,
                     size_t EWT,
                     std::vector<int16_t>& event);

};

#endif
