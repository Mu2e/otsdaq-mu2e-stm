#ifndef queue_write_h_
#define queue_write_h_

#include <iostream>
#include <thread>
#include <atomic>
#include <cinttypes>
#include <string>
#include <algorithm>
#include <fstream>

// Data variables header
#include "STMDAQ-TestBeam/utils/dataVars.hh"
// UDP socket header
#include "STMDAQ-TestBeam/utils/UDPsocket.hh"

class queue_write{

public :

  // Constructor
  queue_write(std::string date_time, uint64_t buffer_len);

  // Try to push data to queue
  uint64_t try_push(int chan, int16_t *data, int n);
  
  // Push data to queue
  void push(int chan, int16_t *data, int n);
  
  // Try to write data to file
  uint64_t try_write_file(int chan);

  // Write data to file
  void write_file(bool *timeout, int chan);

  // Try to write data to file
  void write_end(int chan);

  // Get a new file name
  std::string get_file_name(int chan, int subrun);

  // Monitor and close/open binary files                                    
  void monitor_files(int chan);

  // Thread function to call to write data to file
  void write_data_func(int chan, UDPsocket &udp);

  // Thread function to call to monitor files to open/close
  void write_monitor_func(int chan, UDPsocket &udp);

  // The file date and time
  std::string file_date_time;

  // The data directory
  std::string data_dir = mainOutDir+"data/Nov24_MiniGR/";
  
  // The maximum binary file size per subrun
  static const uint64_t max_bf_size = uint64_t(2e9); // 2 Gb   

  // Atomic write pointer
  std::atomic<int64_t> write[CHNUM];
  // Atomic read pointer
  std::atomic<int64_t> read[CHNUM];
  // Atomic occupancy counter
  std::atomic<int64_t> occupancy[CHNUM];
  // Atomic subrun counter
  std::atomic<uint64_t> subrun[CHNUM] = {};

  // Current tail pointer
  int64_t current_tail[CHNUM];
  // Next tail pointer
  int64_t next_tail[CHNUM];
  // Current head pointer
  int64_t current_head[CHNUM];
  // Current subrun pointer
  int64_t current_subrun[CHNUM];

  // Total data written                                                     
  uint64_t total_data[CHNUM] = {};

  // Number written to the buffer
  int64_t write_num[CHNUM]; // in push
  int64_t read_num[CHNUM]; // in pull

  // The maximum number of datagrams to pull
  int write_max = 0;

  // Write data array
  int16_t* write_data[CHNUM];

  // The buffer
  int16_t* buffer[CHNUM];

  // Number of data files to cycle between (the data file ring buffer)      
  static const uint file_num = 1;

  // The subrun_buff_index for each channel                                 
  int subrun_buff_index[CHNUM] = {};

  // Current head pointer                                                   
  uint64_t current_subrun_buff[CHNUM] = {};

  // Data files                                                             
  std::ofstream bf[CHNUM][file_num];

  // Data file names                                                        
  std::string bf_name[CHNUM][file_num];

  // Booleans to signal new data file needed                                
  std::atomic<bool> new_file[CHNUM][file_num] = {{}};

  // Boolean to signal whether the data write is finished                   
  bool write_finished[CHNUM] = {};

  // The buffer length
  int64_t RING_BUFFER_LEN = 0;

  // Increment function
  int64_t increment(uint64_t n, uint64_t x){
    return (n + x) % RING_BUFFER_LEN;
  }

private :


};

#endif
  
