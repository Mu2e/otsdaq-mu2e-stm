#ifndef SEND_TCP_hh
#define SEND_TCP_hh

// STM data header
#include "Mu2e-STMDAQ/config/stm_data.hh"
// Ring buffer code
#include "Mu2e-STMDAQ/buffers/buffer_queue.hh"


class SendTCP {

private:

  // Store reference to the Config instance
  const Config& cfg;

  // STM data info
  const std::shared_ptr<STMdata>& stm;
  
  // The channel number
  const int CHAN;

  const int nevents;

  bool sendTCP;

public:

  // Constructor
  SendTCP(const Config& cfg_,
	     const std::shared_ptr<STMdata>& stm_,
	     const int CHAN_,
	     const int nevents_,
	     bool sendTCP_);
  
  // Destructor                                                          
  ~SendTCP() {
    std::cout << "SendTCP destructor called.\n";
  }


  void test();

  // Add headers to data
  void send_TCP(std::shared_ptr<std::vector<int16_t>>& buffer);
 
	  
  

};

#endif
