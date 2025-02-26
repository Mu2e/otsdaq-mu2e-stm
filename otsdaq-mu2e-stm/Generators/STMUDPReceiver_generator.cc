#include "otsdaq-mu2e-stm/Generators/STMUDPReceiver.hh"
#include "artdaq/Generators/GeneratorMacros.hh"
#include "artdaq/DAQdata/Globals.hh"
#include "STMDAQ-TestBeam/utils/dataVars.hh"
#include "STMDAQ-TestBeam/utils/xml.hh"
#include "STMDAQ-TestBeam/utils/Hex.hh"
#include "STMDAQ-TestBeam/utils/EnvVars.hh"

#include "trace.h"
#define TRACE_NAME "STMUDPReceiver"
mu2e::STMUDPReceiver::STMUDPReceiver(fhicl::ParameterSet const &ps)
  : artdaq::CommandableFragmentGenerator(ps)
  , i_ch(ps.get<int>("channelNumber"))
  , ip_address(ps.get<std::string>("ip_address"))
  , port(ps.get<int>("port"))
  , verboseLevel_(ps.get<int>("verboseLevel", 0))

{
  TLOG(TLVL_DEBUG) << "STMUDPReceiver Initialized";
  
  // Setup server
  recvSock = udp.setupServer(i_ch,SW);
  rcv_buffer = new int16_t[udp.MAX_UDP_LEN];
  recvCount = 0;

  TLOG(TLVL_INFO) << "PORT = " << udp.getPort(i_ch,SW);
  TLOG(TLVL_INFO) << "IP = " << udp.getIPaddress(i_ch,SW);

}

mu2e::STMUDPReceiver::~STMUDPReceiver()
{
  delete[] rcv_buffer;
}

bool mu2e::STMUDPReceiver::getNext_(artdaq::FragmentPtrs &frags)//getNext_packet
{
  
  TLOG(TLVL_DEBUG) << "000: Now entered getNext function : " << gNextCounter << " !"; 
  gNextCounter++; // Increment number of function calls;
  
  if (should_stop())
    {
       recvCount = 0;
       TLOG(TLVL_DEBUG) << "001:  Received stop signal! ";
       return false;
    }

  // Receive data counter
  uint64_t recvCount = 0;
  
  int global_data_len = 0;
  
  // The return value (number of datagrams received in call)
  int retval = 0;

  // Signal first packet received
  bool first_packet = true;

  // Re-initialise udp timeout for each call
  udp.timeout_counter = 0;

  // Loop until timeout
  while(udp.timeout_counter != udp.getTimeoutMax()){

    if (should_stop())
      {
	recvCount = 0;
	TLOG(TLVL_DEBUG) << "002: Received loop stop signal! ";
	return false;
      }

    // Get packet and check if it times out (returns 0)
    if ((retval = udp.recvOne(rcv_buffer,recvSock,i_ch)) <= 0){      
      // Increment timeout counters
      if(!first_packet) {
	udp.timeout_counter++;
      }
    }
    // If not timed out...
    else{
      if (recvCount == 0){
	// And set first packet to false
	first_packet = false;
      }
      int event_len = rcv_buffer[fw_tHdr::EvLen];
      if(event_len != 109 && event_len != 0){
	
	int event_size = event_len*sizeof(int16_t);

	// Get event window tag
	int16_t ewt[4] = {rcv_buffer[fw_tHdr::EWT_0],rcv_buffer[fw_tHdr::EWT_1],rcv_buffer[fw_tHdr::EWT_2],0};
	uint64_t EWT;
	memcpy(&EWT, ewt, sizeof(EWT));
	TLOG(TLVL_DEBUG+1) << std::hex << "003: EWT0: " << rcv_buffer[fw_tHdr::EWT_0] << " EWT1: " << rcv_buffer[fw_tHdr::EWT_1] << " EWT2: " << rcv_buffer[fw_tHdr::EWT_2] << "  | EWT= " << EWT;

	// Get event mode
	int16_t EM2 = rcv_buffer[fw_tHdr::EM_2_DRTDC] & 0xFF;
	int16_t em[4] = {rcv_buffer[fw_tHdr::EM_0],rcv_buffer[fw_tHdr::EM_1],EM2,0};
	uint64_t EM;
	memcpy(&EM, em, sizeof(EM));
	TLOG(TLVL_DEBUG+1) << std::hex << "004: EM: " << EM << " at EWT: " << EWT;
	if(EM==0){
	  TLOG(TLVL_DEBUG+1) << "005: EventMode changed, now returning false: " << EM;
	  return false;
	}

	metricMan->sendMetric("Last event window tag", EWT, "status", 3, artdaq::MetricMode::LastPoint); 
	
	frags.emplace_back(new artdaq::Fragment(EWT, fragment_id(), FragmentType::STM, 0));
	frags.back()->resizeBytes(fw_tHdr_Size+event_size); // very important: caused issues with writing data when this line was missing...
	auto dataBegin = frags.back()->dataBegin();
	memcpy(dataBegin, rcv_buffer, fw_tHdr_Size+event_size); // copy the data
	global_data_len += fw_tHdr_Len+event_len;
	TLOG(TLVL_DEBUG) << "006: n_data_copied at end of loop = " << global_data_len;
	ev_counter_inc();  // increment event counter
	// Increase the received packet counter
	recvCount += retval;
	// Reset timeout counter
	udp.timeout_counter = 0;
	//return true;
      }
    } // End if not timed-out
  } // End infinite loop

  if(recvCount>0){
    TLOG(TLVL_DEBUG) << "007: STMUDPReceiver: Number of packets received = " << recvCount << std::endl;
  }
  
  return true;
}

void mu2e::STMUDPReceiver::stop()
{
	// if (skip_dtc_init_) return;  // skip any control of DTC

	// theInterface_->DisableDetectorEmulator();
	// theInterface_->DisableCFOEmulation();
}

// The following macro is defined in artdaq's GeneratorMacros.hh header
DEFINE_ARTDAQ_COMMANDABLE_GENERATOR(mu2e::STMUDPReceiver)
