///////////////////////////////////////////////////////////////////////////
/// This module contains the DQM functions (header).  
///////////////////////////////////////////////////////////////////////////

/********************************************************************/

#ifndef DQM_hh
#define DQM_hh

#include<iostream>
#include<fstream>

#include <sstream>
#include <vector>
#include <string>
#include <charconv>
#include <numeric>

#include <thread>
#include <mutex>

#include<string.h> //memset 
#include<fcntl.h> // for open
#include<unistd.h> // for close 

#include <time.h>
#include <math.h>

// UDP socket
#include "STMDAQ-TestBeam/utils/UDPsocket.hh"

// Hex reader
#include "STMDAQ-TestBeam/utils/Hex.hh"

// Check data
#include "STMDAQ-TestBeam/processData/checkData.hh"

// Form events
#include "STMDAQ-TestBeam/processData/formEvents.hh"

// Zero suppress
#include "STMDAQ-TestBeam/processData/zeroSuppress.hh"

// Prescale events
#include "STMDAQ-TestBeam/processData/prescale.hh"

// Write data
#include "STMDAQ-TestBeam/utils/queue_write.hh"

class stmDQM {		   
  
private:

public:
  
  // Standard constructor
  stmDQM(UDPsocket* udp,
	 checkData &cdp,
	 formEvents &fep,
	 daqZS &zsp,
	 prescaleData &psp,
	 queue_write* &wqp);
  
  // Number of int16_ts in a uint64_t
  static const int u32_len = 2;
  static const int u64_len = 4;

  // STM info num id, length and map arrays 
  int info_num = 0;
  std::string *info_id;
  int *info_len;
  int *info_map;

  // Total stm info length
  int tot_info_len;  
  // Total data length
  int tot_data_len;

  // DQM data array
  int16_t* dqm_data;
  
  // DQM client to send data over socket to DQM
  void dqm_client(int socket, UDPsocket &udp_dqm);

  // Build the dqm data arry
  void build_dqm_array();

  // Get whether we're receiving data
  void get_receiving_data(int map);

  // Get the data directory being written to
  void get_data_dir(int map, int len);		    

  // Get the total data size written to disk
  void get_tot_data_size(int map);

  // Get the total data rate
  void get_tot_data_rate(int map);

  // Get the run number
  void get_run_number(int map);

  // Get the subrun number
  void get_subrun_number(int map);

  // Get the current event window tag
  void get_EWT(int map);

  // Get the current pulse rates (both channels)
  void get_pulse_rates(int map, int chan);

  // Get the current baseline_mean (both channels)
  void get_baseline(int map, int chan);

  // Get the current zs factor (%)
  void get_zs_factor(int map, int chan);

  // Get the current prescale
  void get_prescale(int map, int chan);

  // Get the individual channel data rate
  void get_chan_data_rate(int map, int chan);

  // Get the dropped packet counter per channel
  void get_dropped_packets(int map, int chan);

  // Get the event counter
  void get_event_count(int map, int chan);

  // Get the on-spill event counter
  void get_on_count(int map, int chan);

  // Get the off-spill event counter
  void get_off_count(int map, int chan);

  // Get the the pulse data
  void get_pulse_data(int chan);
  
  // Convert double to int16_t values   
  int16_t* convert_double(double d);
  
};

#endif
