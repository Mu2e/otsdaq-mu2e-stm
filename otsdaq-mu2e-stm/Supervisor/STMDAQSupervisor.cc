#include "otsdaq-mu2e-stm/Supervisor/STMDAQSupervisor.h"
#include "Mu2e-STMDAQ/utils/cpu_utils.hh"
#include "Mu2e-STMDAQ/utils/async_logger.hh"
#include "Mu2e-STMDAQ/utils/EnvVars.hh"
#include "Mu2e-STMDAQ/utils/timer.hh"
#include "Mu2e-STMDAQ/utils/signal_handler.hh"
#include "Mu2e-STMDAQ/config/stm_data.hh"
#include "Mu2e-STMDAQ/config/config.hh"
#include "Mu2e-STMDAQ/processing/buffer_pool.hh"
#include "Mu2e-STMDAQ/processing/thread_manager.hh"
#include "Mu2e-STMDAQ/processing/operation_manager.hh"
#include "Mu2e-STMDAQ/hardware/hw_manager.hh"

using namespace ots;

XDAQ_INSTANTIATOR_IMPL(STMDAQSupervisor)

//==============================================================================
// Constructor
STMDAQSupervisor::STMDAQSupervisor(xdaq::ApplicationStub* s)
: FESupervisor(s),
  cfg_(Config::getInstance(EnvVars::expand("${STM_XML}")))
{
  __SUP_COUT__ << "Constructor." << __E__;

  // DAQ timer
  Timer t("DAQ");

  // Load configuration
  //std::string xml_path = EnvVars::expand("${STM_XML}");
  //cfg_(Config::getInstance(xml_path));

  // Initialise CPU utils
  cpu_ = cpu_utils::getInstance(cfg_);

  if(cpu_){
    __SUP_COUT__ << "CPU is valid" << __E__;
    // Initialise logger
    logger_ = std::make_shared<AsyncLogger>(cfg_, cpu_);
  }
  /*
  // Initialise signal handler
  signal_ = std::make_shared<SignalHandler>(logger_, cpu_);

  // Initialise STM data
  stm_ = std::make_shared<STMdata>(cfg_, logger_);

  // Initialise Hardware Manager
  hw_ = std::make_shared<HardwareManager>(logger_, stm_);

  // Prepare OperationManager (no threads started)
  om_ = std::make_shared<OperationManager>(cfg_, logger_, stm_, signal_);*/

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

    // Join all threads so we clean up
    if (tm_) {
        tm_.reset();
    }

    if (pool_) pool_.reset();
    if (om_)   om_.reset();

    __SUP_COUT__ << "DAQ stopped cleanly." << __E__;
}

//==============================================================================
/// transitionConfiguring
void STMDAQSupervisor::transitionConfiguring(toolbox::Event::Reference e)
{
  __SUP_COUT__ << "transitionConfiguring" << __E__;

  stop::reset_stops();
  
  CoreSupervisorBase::transitionConfiguring(e);

  __SUP_COUT__ << "Class_num = " << om_->class_num() << __E__;
  
  // Only create the BufferPool here
  if (om_->class_num() > 0)
  {
    pool_ = std::make_shared<BufferPool>(cpu_, logger_, stm_, om_);
  }
}


//==============================================================================
/// transitionStarting
void STMDAQSupervisor::transitionStarting(toolbox::Event::Reference e)
{
  __SUP_COUT__ << "transitionStarting" << __E__;

  CoreSupervisorBase::transitionStarting(e);

  __SUP_COUT__ << "Class_num = " << om_->class_num() << __E__;

  // Start DAQ threads now
  if (om_->class_num() > 0 && pool_)
  {
    tm_ = std::make_shared<ThreadManager>(cpu_, logger_, stm_, signal_, pool_, om_, hw_);
  }
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
