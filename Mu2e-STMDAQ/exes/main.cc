// STM Frontend Header
#include "Mu2e-STMDAQ/frontend/stm_frontend.hh"

// Main
int main() {  

  // Create and configure STM Frontend
  std::shared_ptr<STMfrontend> stmFE = std::make_shared<STMfrontend>();
  
  // Start STMDAQ
  stmFE->start_stmdaq();
  stmFE->wait();
  
  return 0;
  
}
