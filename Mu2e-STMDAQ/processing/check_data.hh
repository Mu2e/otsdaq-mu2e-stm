#ifndef CHECK_DATA_hh
#define CHECK_DATA_hh

// Async Logger code
#include "Mu2e-STMDAQ/utils/async_logger.hh"
// STM data header
#include "Mu2e-STMDAQ/config/stm_data.hh"
// Operations base header
#include "Mu2e-STMDAQ/processing/operations_base.hh"

class CheckData : public OperationMap {

private:

  // Async Logger
  const std::shared_ptr<AsyncLogger>& logger;

  // STM data info
  const std::shared_ptr<STMdata>& stm;
  
  // The last packet number
  uint32_t last_packet = 0;

  // The last EWT
  uint64_t last_EWT = 0;

  // Booleans to check for 0xDEADBEEF
  bool deadbeef = false;

  // Boolean whether this is the first packet of the data run
  bool firstPacket = true;

  // Boolean whether this is the start of a new buffer
  bool new_buffer = true;
  
  // Checked packet counter
  uint64_t checked_packet_count = 0;
  
  // A dropped packet counter
  uint64_t dropped_packet_count = 0;

  // Checked EWT counter
  uint64_t checked_EWT_count = 0;
  
  // A lost EWT counter
  uint64_t lost_EWT_count = 0;

  // Boolean to signal null heartbeat (when event mode = 0)
  bool is_null_hb = false;

  // Allow for EWT starting from 0
  bool ewt_offset_known = false;
  bool ewt_is_evnum_minus_one = false;
  
public:

  // Constructor
  CheckData(const std::shared_ptr<AsyncLogger>& logger_,
	    const std::shared_ptr<STMdata>& stm_);

  // Destructor                                                          
  ~CheckData() {
    // Log message or warning depending on lost data
    int log_type = 1;
    // If we have dropped packets --> warning
    if (dropped_packet_count > 0 or lost_EWT_count > 0) log_type = 2;
    // Log to user
    logger->log("CheckData: Checked " +
                std::to_string(checked_packet_count) +
                " UDP packets and detected " +
                std::to_string(dropped_packet_count) +
                " dropped packets. And checked " +
                std::to_string(checked_EWT_count) +
                " EWTs and detected " +
                std::to_string(lost_EWT_count) +
                " lost EWTs.",log_type);
    std::cout << "CheckData destructor called.\n";
  }
  
  // Process packet data
  void check_packets(std::shared_ptr<DataStruct>& buffer);
  
  // Check the packet header end
  void check_pHdr(uint32_t packet_num, int16_t pHdr_end);
  
  // Check for correct header format
  void check_eHdrs(std::shared_ptr<DataStruct>& buffer, uint64_t packet_start);

  // Check for dropped packets 
  bool check_dropped_packets(std::shared_ptr<DataStruct>& buffer, uint32_t packet_num, uint32_t last_packet);

  // Check firmware event header 
  uint64_t check_eHdr(std::shared_ptr<DataStruct>& buffer, uint64_t hdr_index);
  
};

#endif
