#include "otsdaq-mu2e-stm/Supervisor/STMDAQSupervisor.h"
#include "Mu2e-STMDAQ/frontend/stm_frontend.hh"

using namespace ots;

XDAQ_INSTANTIATOR_IMPL(STMDAQSupervisor)

//==============================================================================
// Constructor
STMDAQSupervisor::STMDAQSupervisor(xdaq::ApplicationStub* s)
: FESupervisor(s)
{
  __SUP_COUT__ << "Constructor." << __E__;

  stmFE_ = std::make_shared<STMfrontend>(); 

  __SUP_COUT__ << "Constructed." << __E__;
}

//==============================================================================
// Destructor
STMDAQSupervisor::~STMDAQSupervisor(void)
{
  __SUP_COUT__ << "Destroying..." << __E__;
  __SUP_COUT__ << "Destructed." << __E__;
}

void STMDAQSupervisor::stopDAQ()
{
    __SUP_COUT__ << "Triggering stop..." << __E__;

    // Trigger stop for all threads
    stop::trigger_user_stop();

    __SUP_COUT__ << "DAQ stopped cleanly." << __E__;
}

//==============================================================================
/// transitionConfiguring
void STMDAQSupervisor::transitionConfiguring(toolbox::Event::Reference e)
{
  __SUP_COUT__ << "transitionConfiguring" << __E__;

  stop::reset_stops();
  
  CoreSupervisorBase::transitionConfiguring(e);

}


//==============================================================================
/// transitionStarting
void STMDAQSupervisor::transitionStarting(toolbox::Event::Reference e)
{
  __SUP_COUT__ << "transitionStarting" << __E__;

  CoreSupervisorBase::transitionStarting(e);

  // Start STMDAQ
  stmFE_->start_stmdaq();

}

//==============================================================================
/// transitionResuming
void STMDAQSupervisor::transitionResuming(toolbox::Event::Reference e)
{
  __SUP_COUT__ << "transitionResuming" << __E__;
  CoreSupervisorBase::transitionResuming(e);
}

//==============================================================================
/// transitionHalting
void STMDAQSupervisor::transitionHalting(toolbox::Event::Reference e)
{
    __SUP_COUT__ << "transitionHalting" << __E__;

    // Trigger a clean stop
    stopDAQ();

    CoreSupervisorBase::transitionHalting(e);
}

void STMDAQSupervisor::transitionStopping(toolbox::Event::Reference e)
{
    __SUP_COUT__ << "transitionStopping" << std::endl;
    stopDAQ();   // cleanly stop threads and reset resources
    CoreSupervisorBase::transitionStopping(e);
}
