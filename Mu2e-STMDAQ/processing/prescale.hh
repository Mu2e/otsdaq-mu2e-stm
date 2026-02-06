#ifndef PRESCALE_hh
#define PRESCALE_hh

// Async Logger code
#include "Mu2e-STMDAQ/utils/async_logger.hh"
// STM data header
#include "Mu2e-STMDAQ/config/stm_data.hh"
// Operations base header
#include "Mu2e-STMDAQ/processing/operations_base.hh"
// Fast random number generator
#include "Mu2e-STMDAQ/utils/FastRNG.hh"

// Prescale class
class Prescale : public OperationMap {

private:

  // Async Logger
  const std::shared_ptr<AsyncLogger>& logger;

  // STM data info
  const std::shared_ptr<STMdata>& stm;
  
  // Fast random number generator
  FastRNG rng;

  // Prescale config
  const prescale_info p_cfg;

  // Spill struct (state tracking)
  struct spill {
    uint64_t count = 0;           // shared per spill
    uint64_t selection_start[2];  // [datatype] 0=raw,1=zs
  };

  // Two spill states to track (on/off-spill)
  spill sp[2]; // [spill] 0=off,1=on
    
  // Raw prescale flag
  const int RAW = 0;
  // ZS prescale flag
  const int ZS = 1;
  
  // Header base values
  uint16_t hdr_on_base{0};
  uint16_t hdr_off_base{0};
  
  // Check for prime number
  bool is_prime(long long n) {
    if (n <= 1) return false;
    if (n <= 3) return true;
    if (n % 2 == 0 || n % 3 == 0) return false;
    for (long long i = 5; i * i <= n; i += 6) {
      if (n % i == 0 || n % (i + 2) == 0)
        return false;
    }
    return true;
  }

  // Pack prescale header with values
  void pack_hdr(bool data_type, bool ps, uint16_t& h) {
    if (data_type == RAW) h |= (static_cast<uint16_t>(ps) & 0x1) << 15;
    if (data_type == ZS) h |= (static_cast<uint16_t>(ps)  & 0x1) << 7;      
  }
  
public:
  
  // Constructor
  Prescale(const std::shared_ptr<AsyncLogger>& logger_,
                const std::shared_ptr<STMdata>& stm_);
  
  // Destructor                                                          
  ~Prescale() {
    std::cout << "Prescale destructor called.\n";
  }

  // Prescale data
  void prescale_data(std::shared_ptr<DataStruct>& buffer);

  // Select prescale data
  void select_data(std::shared_ptr<DataStruct>& buffer, const int data_flag);
  
};

#endif
