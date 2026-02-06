#include "otsdaq-mu2e-stm/FEInterfaces/ROCStoppingTargetMonitorInterface.h"

#include "otsdaq/Macros/InterfacePluginMacros.h"
#include <unistd.h>//used for usleep(microseconds)

using namespace ots;

#undef __MF_SUBJECT__
#define __MF_SUBJECT__ "FE-ROCStoppingTargetMonitorInterface"

//=========================================================================================
ROCStoppingTargetMonitorInterface::ROCStoppingTargetMonitorInterface(
    const std::string&       rocUID,
    const ConfigurationTree& theXDAQContextConfigTree,
    const std::string&       theConfigurationPath)
    : ROCCoreVInterface(rocUID, theXDAQContextConfigTree, theConfigurationPath)
{
	INIT_MF("." /*directory used is USER_DATA/LOG/.*/);

	__COUT_INFO__ << "ROCStoppingTargetMonitorInterface instantiated with link: "
	               << linkID_ << " and EventWindowDelayOffset = " << delay_ << __E__;

	/*ConfigurationTree rocTypeLink =
	    Configurable::getSelfNode().getNode("ROCTypeLinkTable");

	STMParameter_1_ = rocTypeLink.getNode("NumberParam1").getValue<int>();

	STMParameter_2_ = rocTypeLink.getNode("TrueFalseParam2").getValue<bool>();

	STMParameter_3_ = rocTypeLink.getNode("tempColumn1").getValueAsString();
				
	std::string STMParameter_3 = rocTypeLink.getNode("STMMustBeUniqueParam1").getValue<std::string>();
        
        __FE_COUTV__(STMParameter_1_);
        __FE_COUTV__(STMParameter_2_);
	__FE_COUTV__(STMParameter_3);*/                                
                 

}

//==========================================================================================
ROCStoppingTargetMonitorInterface::~ROCStoppingTargetMonitorInterface(void)
{
	// NOTE:: be careful not to call __FE_COUT__ decoration because it uses the
	// tree and it may already be destructed partially
	__COUT__ << FEVInterface::interfaceUID_ << " Destructor" << __E__;
}

//============================================================================================
void ROCStoppingTargetMonitorInterface::writeEmulatorRegister(uint16_t address,
                                                              uint16_t data_to_write)
{
	__FE_COUT__ << "Calling write ROC Emulator register: link number " << std::dec
	            << linkID_ << ", address = " << address
	            << ", write data = " << data_to_write << __E__;

	return;
}

//==================================================================================================
uint16_t ROCStoppingTargetMonitorInterface::readEmulatorRegister(uint16_t address)
{
	__FE_COUT__ << "Calling read ROC Emulator register: link number " << std::dec
	            << linkID_ << ", address = " << address << __E__;

	return -1;
}

//==================================================================================================
void ROCStoppingTargetMonitorInterface::configure(void){
  return;
}

//==============================================================================
void ROCStoppingTargetMonitorInterface::halt(void) {
  __COUT_INFO__ << "In ::halt()"<<event_number_ << __E__;
  return;
}

//==============================================================================
void ROCStoppingTargetMonitorInterface::pause(void) {
  __COUT_INFO__ << "In ::pause()"<<event_number_ << __E__;
  return;
}

//==============================================================================
void ROCStoppingTargetMonitorInterface::resume(void) {
  __COUT_INFO__ << "In ::resume()"<<event_number_ << __E__;
  return;
}

//==============================================================================
void ROCStoppingTargetMonitorInterface::start(std::string runNumber)
{
  return;
}

//==============================================================================
void ROCStoppingTargetMonitorInterface::stop(void) {
  return;
}

//==============================================================================
bool ROCStoppingTargetMonitorInterface::running(void) { 
  return false;
}

DEFINE_OTS_INTERFACE(ROCStoppingTargetMonitorInterface)
