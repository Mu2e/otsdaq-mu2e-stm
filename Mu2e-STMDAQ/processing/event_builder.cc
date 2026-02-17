// Event Builder code
#include "Mu2e-STMDAQ/processing/event_builder.hh"

// Constructor
EventBuilder::EventBuilder(const std::shared_ptr<AsyncLogger>& logger_,
                           const std::shared_ptr<STMdata>& stm_) :
  logger(logger_), stm(stm_) {}

// Build the event/EWT
size_t EventBuilder::build_event(std::shared_ptr<DataStruct>& buffer,
                                 size_t EWTidx, std::vector<int16_t>& event){

  // Check that the event vector is the right size
  if (event.size() < 3*stm->buffer_config.raw_len){
    if (logger) logger->log("EventBuilder::build_event: Error! Event size is < 3*stm->buffer_config.raw_len!",0);
    std::this_thread::sleep_for(std::chrono::seconds(1));
  }
  
  // Define a pointer to the raw data buffer
  auto& raw = buffer->raw;
  
  // Data length
  size_t data_len = 0;

  // Get this EWT
  EWT_info& ewt = buffer->EWTs[EWTidx];
  uint64_t EWT = ewt.EWT;

  // -------------
  // EWT HEADER
  // -------------
  
  // Insert the EWT header into the event
  std::copy(ewt.hdr.begin(), ewt.hdr.end(), event.begin() + data_len);

  // Increase data length
  data_len += sw_eHdr_info::len;
  
  // If this EWT's raw data has not been marked as bad
  if (!ewt.bad_data){
  
    // -------------
    // RAW DATA
    // -------------
    
    // If this EWT's raw data has not be prescaled
    if (!ewt.raw.prescale){
      // Start index of raw EWT data in buffer
      size_t start = ewt.raw.start;
      // Length of raw EWT data in buffer
      size_t len = ewt.raw.len;
      // Insert raw event data into the event
      std::copy(raw.begin() + start, raw.begin() + start + len,  event.begin() + data_len);
      // Increase data length
      data_len += len;
    }
    
    // -------------
    // ZS DATA
    // -------------
    // If this EWT's raw data has not be prescaled
    if (!ewt.zs.prescale){
      // Loop over all ZS regions
      for (int i = 0; i < ewt.zs.zs_regions.size(); i++){
        // Start index of ZS region in buffer
        size_t start = ewt.zs.zs_regions[i].start;
        // Length of ZS regionin buffer
        size_t len = ewt.zs.zs_regions[i].len;
        // Insert start and length into the event
        event[data_len] = static_cast<int16_t>(start); // start
        ++data_len; // Increase data length
        event[data_len] = static_cast<int16_t>(len); // length      
        ++data_len; // Increase data length
        // Insert raw event data into the event
        std::copy(raw.begin() + start, raw.begin() + start + len,  event.begin() + data_len);
        // Increase data length
        data_len += len;
      }
    }

    // -------------
    // PH DATA
    // -------------
    // Loop over all ZS regions
    for (int i = 0; i < ewt.ph.size(); i++){
      // Pulse time
      int16_t time = ewt.ph[i].time;
      // Pulse height
      int16_t height = ewt.ph[i].height;
      // Insert time and height into the event
      event[data_len] = time;
      ++data_len; // Increase data length
      event[data_len] = height;
      ++data_len; // Increase data length
    }
    
  }   
    
  return data_len;
  
}


