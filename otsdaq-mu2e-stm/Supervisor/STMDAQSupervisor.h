#ifndef _ots_STMDAQSupervisor_h_
#define _ots_STMDAQSupervisor_h_

#include "otsdaq/CoreSupervisors/FESupervisor.h"
#include "Mu2e-STMDAQ/frontend/stm_frontend.hh"

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
    std::shared_ptr<STMfrontend> stmFE_;
  };

}  // namespace ots

#endif
