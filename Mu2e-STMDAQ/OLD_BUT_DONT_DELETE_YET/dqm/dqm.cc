///////////////////////////////////////////////////////////////////////////
/// This module contains the DQM functions (main). 
///////////////////////////////////////////////////////////////////////////

/********************************************************************/

// Loggerr
#include "STMDAQ-TestBeam/utils/Logger.hh"

// DQM
#include "STMDAQ-TestBeam/dqm/dqm.hh"

// Get environment variables
#include "STMDAQ-TestBeam/utils/EnvVars.hh"

// Data variables
dataVars datVars; 

UDPsocket *udp;
checkData *cd;
daqZS *zs;
prescaleData *ps;
formEvents *fe; 
queue_write* wq;

//Standard constructor
stmDQM::stmDQM(UDPsocket* udpp,
	       checkData &cdp,
	       formEvents &fep,
	       daqZS &zsp,
	       prescaleData &psp,
	       queue_write* &wqp){
  
  // Store classes as global pointer
  udp = &udpp[0];
  zs = &zsp;
  ps = &psp;
  cd = &cdp;
  fe = &fep;
  wq = wqp;

  // Get STM DAQ directory
  std::string stmdaq_dir = EnvVars::expand("${STMDAQ_ROOT}");

  // Open the dqm data map file
  ifstream fin;
  fin.open (stmdaq_dir+"/dqm/dqm_map.txt");

  // Get the file 
  std::string line;  
  // Vector of strings for map id and length
  std::vector < string > id;
  std::vector < int > len;

  // if the file is open
  if ( fin.is_open ( )){
    // Get the line in the file
    while ( getline ( fin, line )){
      // Stringstream the file
      stringstream ss ( line );	
      // Counter to distinguish id/len
      int count = 0;
      // Seperate the line by the space between the id and map
      while ( getline (ss ,line, ' ')){
	// First entry = id
	if (count == 0){
	  // Store the id
	  id.push_back(line);
	  // Increment counter
	  count += 1;
	}
	// Second entry = len
	else if (count == 1){
	  // Store the length
	  len.push_back(std::stoi(line));
	  // Skip description	  
	  break;
	}
      }
      // Increment the number of info ids
      info_num++;
    }    
  }
  // Throw error if dqm file not found
  else{
    std::cout << "ERROR: can't find dqm data map file! Exiting..." << std::endl;
    exit(0);
  }

  // Allocate info id, length and map arrays
  info_id = new std::string [info_num];
  info_len = new int [info_num];
  info_map = new int [info_num];

  // Fill id, length and map arrays
  int map_num = 0;
  for (int i = 0; i < id.size(); i++){
    info_id[i] = id[i];
    info_len[i] = len[i];
    info_map[i] = map_num;
    map_num += len[i];
  }

  // Get the total stm info array length
  tot_info_len =  accumulate(len.begin(), len.end(), 0);
  
  // Calculate the total data array length
  tot_data_len = tot_info_len + 2*zs->get_total_peak_max();

  // Initialise data array
  dqm_data = new int16_t [tot_data_len] ();

} 

// DQM client to send data over socket to DQM   
void stmDQM::dqm_client(int socket, UDPsocket &udp_dqm){
  
  // Notify user
  Logger::Instance()->write(1,"In DQM thread: socket = " 
			    + std::to_string(socket));

  // While the main udp socket hasn't timed out...
  while(!udp->timeout){
    // Update once per second
    this_thread::sleep_for(1s);
    build_dqm_array();
    // //    memcpy(dqm_data,split_temp,sizaeof(uint64_t));
    // for (int i = 0; i < CHNUM; i++){
    //   memcpy(&dqm_data[4 + i*zs->get_total_peak_max()],
    // 	     zs->dqm_pulse[i],
    // 	     zs->get_total_peak_max()*sizeof(int16_t));
    // }
    // Send to socket
    udp_dqm.sendOne(dqm_data,tot_data_len*sizeof(int16_t),socket);
  }


}

// Function to build the dqm data array
void stmDQM::build_dqm_array(){
  
  // Loop over STM infos
  for (int i = 0; i < info_num; i++){
    // Get id map number
    int map = info_map[i];
    int len = info_len[i];
    // Skip hardware checks for now
    if (info_id[i] == "receiving_data"){
      get_receiving_data(map);
    }
    else if (i < 10){
    }
    // Get the data dir
    else if (info_id[i] == "data_dir"){
      get_data_dir(map,len);
     }
    // Get the total data size
    else if (info_id[i] == "tot_data_size"){
      get_tot_data_size(map);
    }
    // Get the total data rate
    else if (info_id[i] == "tot_data_rate"){
      get_tot_data_rate(map);
    }
    // Get the run number
    else if (info_id[i] == "run_num"){
      get_run_number(map);
    }
    // Get the subrun number
    else if (info_id[i] == "subrun_num"){
      get_subrun_number(map);
    }
    // Get the event window tag
    else if (info_id[i] == "ewt"){
      get_EWT(map);
    }
    // Get pulse rates
    else if (info_id[i] == "pulse_rate_0"){
      get_pulse_rates(map,0);
    }    
    else if (info_id[i] == "pulse_rate_1"){
      get_pulse_rates(map,1);
    }    
    // Get baseline variables
    else if (info_id[i] == "b_mean_0"){      
      get_baseline(map,0);
    }
    else if (info_id[i] == "b_mean_1"){      
      get_baseline(map,1);
    }
    // Skip baseline rms id (calculated in mean)
    else if (info_id[i] == "b_rms_0" or info_id[i] == "b_rms_1"){
      continue;
    }
    // Get zs factor variables
    else if (info_id[i] == "zs_factor_0"){      
      get_zs_factor(map,0);
    }
    else if (info_id[i] == "zs_factor_1"){      
      get_zs_factor(map,1);    
    }
    // Get prescale factors
    else if (info_id[i] == "prescale_0"){      
      get_prescale(map,0);
    }
    else if (info_id[i] == "prescale_1"){      
      get_prescale(map,1);
    }
    // Get channel data rate
    else if (info_id[i] == "data_rate_0"){      
      get_chan_data_rate(map,0);
    }
    else if (info_id[i] == "data_rate_1"){      
      get_chan_data_rate(map,1);
    }
    // Get dropped packets
    else if (info_id[i] == "packets_0"){       
      get_dropped_packets(map,0);
    }
    else if (info_id[i] == "packets_1"){       
      get_dropped_packets(map,1);
    }
    // Get event counters
    else if (info_id[i] == "events_0"){       
      get_event_count(map,0);
    }
    else if (info_id[i] == "events_1"){       
      get_event_count(map,1);
    }
    // Get on-spill counters
    else if (info_id[i] == "on_0"){       
      get_on_count(map,0);
    }
    else if (info_id[i] == "on_1"){       
      get_on_count(map,1);
    }
    // Get off-spill counters
    else if (info_id[i] == "off_0"){       
      get_off_count(map,0);
    }
    else if (info_id[i] == "off_1"){       
      get_off_count(map,1);
    }
    else{
      std::cout << "ERROR: could not find DQM map ID: " << info_id[i] << ". Exiting..." << std::endl;
      exit(0);
    }
  }

  // Copy pulse data
  for (int i = 0; i < CHNUM; i++){
    get_pulse_data(i);
  }
  
}

// Get whether we're receiving data
void stmDQM::get_receiving_data(int map) {

  // Get receive data bool
  int16_t receiving = 0;
  if (udp->receiving_data[0]) receiving = 1;
  // Copy to dqm data array
  dqm_data[map] = receiving;

}

// Get data directory
void stmDQM::get_data_dir(int map, int len){

  // Get the data directory
  std::string str = "/data1/STM_VST_DATA/March24";

  // Convert to char array
  char* cstr = new char [len];
  std::strcpy(cstr, wq->data_dir.c_str());

  // Data directory array
  int16_t* data_dir_array = new int16_t [len] ();
  for (int i = 0; i < str.size() + 1; i++){
    data_dir_array[i] = (int16_t)cstr[i];
  }
  
  // Memcpy to dqm data array
  memcpy(&dqm_data[map],data_dir_array,len*sizeof(int16_t));

  // Delete the data array
  delete[] data_dir_array;

} 

// Get the total data size written to disk
void stmDQM::get_tot_data_size(int map) {

  // Get the total data size
  double data_size = wq->total_data[0] + wq->total_data[1]; 
  // Memcpy to dqm data array
  memcpy(&dqm_data[map],convert_double(data_size),sizeof(uint32_t));

}

// Get the total data rate
void stmDQM::get_tot_data_rate(int map) {

  // Get the total data size
  double data_rate = 0; 
  // Memcpy to dqm data array
  memcpy(&dqm_data[map],convert_double(data_rate),sizeof(uint32_t));

}

// Get the run number
void stmDQM::get_run_number(int map){

  // Get the run number
  uint64_t run_number = 1;
  // Memcpy to dqm data array
  memcpy(&dqm_data[map],
	 datVars.split_uint64_t(run_number),
	 sizeof(uint64_t));
 
}

// Get the subrun number
void stmDQM::get_subrun_number(int map){

  // Get the subrun number
  uint64_t subrun_number = 1;
  // Memcpy to dqm data array
  memcpy(&dqm_data[map],
	 datVars.split_uint64_t(subrun_number),
	 sizeof(uint64_t));

}

// Get the event window tag
void stmDQM::get_EWT(int map){

  // Get the event number
  uint64_t event = fe->event_number;
  // Memcpy to dqm data array
  memcpy(&dqm_data[map],
	 datVars.split_uint64_t(event),
	 sizeof(uint64_t));
  
}

// Get the current pulse rates
void stmDQM::get_pulse_rates(int map, int chan){

  // Get the rate
  double rate = zs->rate[chan];    
  // Memcpy to dqm data array
  memcpy(&dqm_data[map],
	 convert_double(rate),
	 sizeof(uint32_t));

}

// Get the current pulse rates
void stmDQM::get_baseline(int map, int chan){

  // Get the summed mean 
  int64_t mean_sum = zs->b_mean[chan];  
  // Get the summed rms
  int64_t rms_sum = zs->b_rms[chan];  
  // Get the summed count 
  uint64_t count = zs->b_count[chan];         
  
  // Calculate mean and rms
  int16_t mean = 0;
  int16_t rms = 0;
  if (count != 0){
      mean = (mean_sum/count);
      rms = (sqrt(rms_sum/count - mean*mean));
  }
  // Copy to dqm data array
  dqm_data[map] = mean;
  dqm_data[map+1] = rms;
  
}


// Get the current zs factor (%)
void stmDQM::get_zs_factor(int map, int chan){

  // Get zero suppression factor
  double factor = zs->sup_factor[chan];  
  // Memcpy to dqm data array
  memcpy(&dqm_data[map],
	 convert_double(factor),
	 sizeof(uint32_t));

}

// Get the current prescale
void stmDQM::get_prescale(int map, int chan){

  // Get prescale number
  dqm_data[map] = ps->get_prescale_num(chan);

}



// Get the individual channel data rate
void stmDQM::get_chan_data_rate(int map, int chan) {

  // Get the total data size
  double data_rate = 0; 
  // Memcpy to dqm data array
  memcpy(&dqm_data[map],
	 convert_double(data_rate),
	 sizeof(uint32_t));

}

// Get the dropped packet counter per channel
void stmDQM::get_dropped_packets(int map, int chan){

  uint64_t dropped = cd->dropped_packet_count[chan];
  // Memcpy to dqm data array
  memcpy(&dqm_data[map],
	 datVars.split_uint64_t(dropped),
	 sizeof(uint64_t));
}


// Get the event counter
void stmDQM::get_event_count(int map,int chan){

  // Get the event counter
  uint64_t events = fe->event_count[chan];
  // Memcpy to dqm data array
  memcpy(&dqm_data[map],
	 datVars.split_uint64_t(events),
	 sizeof(uint64_t));
  
}

// Get the on-spill event counter
void stmDQM::get_on_count(int map,int chan){

  // Get the on-spill event count
  uint64_t on_events = fe->on_spill_count[chan];
  // Memcpy to dqm data array
  memcpy(&dqm_data[map],
	 datVars.split_uint64_t(on_events),
	 sizeof(uint64_t));
  
}

// Get the off-spill event counter
void stmDQM::get_off_count(int map,int chan){

  // Get the off-spill event count
  uint64_t off_events = fe->off_spill_count[chan];
  // Memcpy to dqm data array
  memcpy(&dqm_data[map],
	 datVars.split_uint64_t(off_events),
	 sizeof(uint64_t));
  
}

// Get the the pulse data
void stmDQM::get_pulse_data(int chan){

  // Memcpy to dqm data array
  memcpy(&dqm_data[tot_info_len + chan*zs->get_total_peak_max()],
	 zs->dqm_pulse[chan],
	 zs->get_total_peak_max()*sizeof(int16_t));
  
}


// Convert double to int16_t values
int16_t* stmDQM::convert_double(double d) {
  uint32_t value = d*1e4;
  return datVars.split_uint32_t(value);
}

