#ifndef _ots_STMDAQSupervisor_h_
#define _ots_STMDAQSupervisor_h_

#include <thread>
#include "Mu2e-STMDAQ/frontend/stm_frontend.hh"
#include "otsdaq/CoreSupervisors/CoreSupervisorBase.h"

namespace ots
{

class STMDAQSupervisor : public CoreSupervisorBase
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

	virtual std::vector<SupervisorInfo::SubappInfo> getSubappInfo(void) override;

	// Trigger a clean stop for all threads and operations
	void stopDAQ();

  private:
	// Core DAQ objects
	std::shared_ptr<STMfrontend> stmFE_;

	// Bool for first STMDAQ config read
	bool configured_      = false;

	// STMDAQ failure received
	bool failureReported_ = false;

	// True between a successful start and the next stop/halt; prevents
	// start_stmdaq() being called twice in a row without a stop
	bool daqStarted_ = false;
};

}  // namespace ots

#endif
