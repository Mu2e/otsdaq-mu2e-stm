#include "otsdaq-mu2e-stm/Supervisor/STMDAQSupervisor.h"
#include "Mu2e-STMDAQ/frontend/stm_frontend.hh"

using namespace ots;

XDAQ_INSTANTIATOR_IMPL(STMDAQSupervisor)

//==============================================================================
// Constructor
STMDAQSupervisor::STMDAQSupervisor(xdaq::ApplicationStub* s)
: CoreSupervisorBase(s)
{
  __SUP_COUT__ << "Constructor." << __E__;

  //clean up any leftover DQMS that didn't close properly 
  std::string commandResponse = StringMacros::exec("pgrep -f app.py | xargs kill -9");

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

    stmFE_.reset();

    __SUP_COUT__ << "DAQ stopped cleanly." << __E__;
}

//==============================================================================
/// transitionConfiguring
void STMDAQSupervisor::transitionConfiguring(toolbox::Event::Reference e)
{
  __SUP_COUT__ << "transitionConfiguring" << __E__;

  CoreSupervisorBase::configureInit();

  stop::reset_stops();

  //auto& stm_config = Config::getInstance();
  //stm_config.reinit();

  stmFE_ = std::make_shared<STMfrontend>();

  CoreSupervisorBase::transitionConfiguring(e);

  // Get the channel for log file labelling
  int currentChannel = stmFE_->return_channel();

  //HPGe DQM start
  if (currentChannel == 0){
    std::string commandResponse = StringMacros::exec("nohup python /home/mu2eshift/ots_ops_stm/srcs/otsdaq-mu2e-stm/Mu2e-STMDAQ/dqm/app.py > /home/mu2eshift/ots_ops_stm/Data_stm/Logs/DQMlogs/dqmout_HPGe.log 2>&1 &");
  }
  //LaBr DQM start
  else{
    std::string commandResponse = StringMacros::exec("nohup python /home/mu2eshift/ots_ops_stm/srcs/otsdaq-mu2e-stm/Mu2e-STMDAQ/dqm/app.py > /home/mu2eshift/ots_ops_stm/Data_stm/Logs/DQMlogs/dqmout_LaBr.log 2>&1 &");
  }


}


//==============================================================================
/// transitionStarting
void STMDAQSupervisor::transitionStarting(toolbox::Event::Reference e)
{
  __SUP_COUT__ << "transitionStarting" << __E__;

  CoreSupervisorBase::transitionStarting(e);

  stop::reset_stops();
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

    // Stop DQM processes on halt, otherwise keep them going in case of new run
    std::string commandResponse = StringMacros::exec("pgrep -f app.py | xargs kill -9");

    CoreSupervisorBase::transitionHalting(e);
}

void STMDAQSupervisor::transitionStopping(toolbox::Event::Reference e)
{
    __SUP_COUT__ << "transitionStopping" << std::endl;

    // Close thread manager but leave FE
    stmFE_->close_threads();

    CoreSupervisorBase::transitionStopping(e);
}
