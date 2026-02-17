#ifndef _ots_STMDAQSupervisor_h_
#define _ots_STMDAQSupervisor_h_

#include "otsdaq/CoreSupervisors/FESupervisor.h"
#include "Mu2e-STMDAQ/utils/cpu_utils.hh"
#include "Mu2e-STMDAQ/utils/async_logger.hh"
#include "Mu2e-STMDAQ/utils/signal_handler.hh"
#include "Mu2e-STMDAQ/config/stm_data.hh"
#include "Mu2e-STMDAQ/config/config.hh"
#include "Mu2e-STMDAQ/processing/buffer_pool.hh"
#include "Mu2e-STMDAQ/processing/thread_manager.hh"
#include "Mu2e-STMDAQ/processing/operation_manager.hh"
#include "Mu2e-STMDAQ/hardware/hw_manager.hh"

namespace ots
{

  class STMDAQSupervisor : public FESupervisor
  {
  public:
    XDAQ_INSTANTIATOR();

    STMDAQSupervisor(xdaq::ApplicationStub* s);
    virtual ~STMDAQSupervisor(void);

    virtual void transitionConfiguring(toolbox::Event::Reference e) override;
    virtual void transitionStarting(toolbox::Event::Reference e) override;
    virtual void transitionResuming(toolbox::Event::Reference e) override;
    virtual void transitionHalting(toolbox::Event::Reference e) override;
    virtual void transitionStopping(toolbox::Event::Reference e) override;

    // Trigger a clean stop for all threads and operations
    void stopDAQ();

  private:
    // Core DAQ objects
    Config& cfg_;
    std::shared_ptr<cpu_utils> cpu_;
    std::shared_ptr<AsyncLogger> logger_;
    std::shared_ptr<SignalHandler> signal_;
    std::shared_ptr<STMdata> stm_;
    std::shared_ptr<HardwareManager> hw_;
    std::shared_ptr<OperationManager> om_;
    std::shared_ptr<BufferPool> pool_;
    std::shared_ptr<ThreadManager> tm_;
  };

}  // namespace ots

#endif
