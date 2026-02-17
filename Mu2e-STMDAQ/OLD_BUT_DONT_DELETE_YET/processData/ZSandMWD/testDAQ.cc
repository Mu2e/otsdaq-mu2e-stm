#include<iostream>
#include <string>
#include <fstream>
#include <vector>
#include <utility> // std::pair
#include <stdexcept> // std::runtime_error
#include <sstream> // std::stringstream
#include <fstream>
#include <chrono>
#include <cmath>
#include <sys/stat.h>
#include <cstring>
#include <time.h>
#include <math.h>
#include <unistd.h>

// Zero-supress data header
#include "zeroSuppress.hh"

// Zero-supress data header
#include "MWD.hh"

// Queue header
#include "queue.hh"

// Gen data                                                                 
#include "genData.hh"

// Generate data class
genData *gen;

// Zero-suppression class
daqZS zsup;

// MWD class                                                                                 
MWD mwd;

//Queue class
queue_buffer queue;

// The percent of available memeory to allocate per core
static const double mem_percent_per_core = 0.7;

// The number of threads                                                                                
static uint thread_num;

// Start, ZS: Find Pulses, ZS: Suppress Data, MWD: Deconvolute, MWD: Differentiate, MWD: Average
static const int process_max = 6;

// Process map                                                                                          
static const int START_Q = 0;
static const int ZS_FIND_PULSES = 1;
static const int ZS_SUP_DATA = 2;
static const int MWD_DECONV = 3;
static const int MWD_DIFF = 4;
static const int MWD_AVG = 5;

// Doing those processes                                                                                
static const bool process[process_max] = {true, // Start                                                
                                          true, // ZS: Find Pulses
                                          true, // ZS: Suppress Data                                      
                                          true, // MWD: Deconvolute                                    
                                          true, // MWD: Differentiate
                                          true}; // MWD: Average

// Store each queue data type
static const std::string queue_type [process_max] = {"int16_t", // Start
						"int16_t", // ZS: Find Pulses
						"int16_t", // ZS: Suppress Data
						"double", // MWD: Deconvolute
						"double", // MWD: Differentiate
						"double"}; // MWD: Average


// The number of queues to allocate
int q_num = 0;

// ZS queue class
//queue_zs<int16_t> event_queue;

// ZS queue class
//queue_zs<int16_t> pulse_queue;

// ZS queue class
//queue_zs<int16_t> sup_queue;

// ZS queue class
//queue_zs<double> deconv_queue;

// ZS queue class
//queue_zs<double> diff_queue;

// ZS queue class
//queue_zs<double> avg_queue;

// The original data length
uint64_t n = 0;

// The original suppressed data length
uint64_t sup_data_len = 0;

// Split events into arrays of random integer no of events
int event_max = 1000;

// Tuple to return from zero-supression algorithm
std::tuple<int16_t*,uint64_t,uint64_t,double,uint64_t,uint64_t> sup;

// Timing variables
std::chrono::high_resolution_clock::time_point t1 ;
std::chrono::high_resolution_clock::time_point t2 ;

// Dummy timeout varaibles
bool b = false;
bool* timeout = &b;
 
// Push to queue thread function                                            
void push(int chan, int16_t *data, uint64_t len){

  // std::cout << "In push to queue thread" << std::endl;
  
  // // Initialise event counter
  // uint64_t event_count = 0;
  // // Initialise total counter
  // uint64_t tot_count = 0;  
  // // Initialise new array
  // int16_t* array = new int16_t [len]();
  // //  time_t seed = time(NULL);
  // time_t seed = 1715953952;
  // std::cout << "Random Seed = " << seed << std::endl;
  // srand(seed);
  // // While event_counter < number of events
  // while(event_count < gen->numberhb){
  //   // Get a random number of events
  //   int event_num = 1 + (rand() % event_max);
  //   //    int event_num = gen->numberhb/2;
  //   // If the event_count + the number of events is greater than
  //   // the maximum number of events, only push remainder
  //   if ((event_count + event_num) > gen->numberhb){
  //     event_num = gen->numberhb - event_count;
  //   }
  //   // Initalise counter
  //   uint64_t count = 0;
  //   // Initialise new array length
  //   uint64_t array_len = 0;    
  //   // Loop over events to ZS
  //   while (count < event_num){
  //     // Get event length
  //     uint16_t event_len = data[array_len + fw_tHdr::EvLen];
  //     // Copy event to new array
  //     memcpy(&array[array_len],
  //    	     &data[tot_count+array_len],
  //    	     (fw_tHdr_Len + event_len)*sizeof(int16_t));
  //     // Increase array length
  //     array_len += fw_tHdr_Len + event_len;
  //     // Increase event counter
  //     count++;
  //   }
  //   // Push the data to the queue                                          
  //   queue.push(chan,array,array_len);    
  //   // Increase event_count
  //   event_count += event_num;
  //   // Increase total counter
  //   tot_count += array_len;

  // }      

  // // Wait five seconds to timeout
  // for (int i = 0; i < 5; i++){
  //   std::cout << "Waiting " << 5-i << "..." << std::endl;
  //   std::this_thread::sleep_for(std::chrono::seconds(1));
  // }
  // // Signal timeout
  // b = true;

}

// Pull from queue thread function                                          
void push_to_zs(int chan, int16_t *data){

  // std::cout << "In push to zs thread" << std::endl;
  
  // // Pull buffer
  // int16_t *buffer = new int16_t [n] ();

  // // The return value
  // int retval = 0;

  // // The total pull length
  // uint64_t pull_count = 0;
    
  // // While still data to pull
  // while(pull_count < n){
  //   // Pull data from queue
  //   retval = queue.pull(timeout,chan,buffer);
  //   // Get time before suppression
  //   if (pull_count == 0) t1 = std::chrono::high_resolution_clock::now();
  //   // Seperate headers from data and push to queue
  //   event_queue.push(chan,zsup.split_headers(0,retval,buffer));
  //   // Increase pull counter
  //   pull_count += retval;
  // }      

}

// Pull from zs queue thread function
void pull_from_zs(int chan, double *&data){

  // std::cout << "In test pull thread" << std::endl;

  // // Data buffer
  // data_struct<double> data_call;
  // //  data_struct<int16_t> data_call;

  // // The return value
  // int retval = 0;

  // // The total pull length
  // uint64_t pull_count = 0;

  // // The total suppressed length
  // uint64_t tot_sup = 0;
  
  // // Peak rate variables
  // uint64_t peak_count = 0;
  // double rate = 0;
  // int rate_count = 0;
  // // Baseline variables
  // int64_t baseline_mean = 0;
  // int64_t baseline_rms = 0;
  // int baseline_count = 0;

  // // Time counter
  // double time_ms = 0;  
  
  // // While still data to pull
  // while(!*timeout){
  //   // Pull data from queue
  //   retval = deconv_queue.pull(timeout,chan,data_call);
  //   //retval = pulse_queue.pull(timeout,chan,data_call);
  //   if (retval > 0){
  //     // Memcpy data from buffer
  //     //      memcpy(&data[pull_count],data_call.adc_data,retval*sizeof(int16_t));
  //     memcpy(&data[pull_count],data_call.adc_data,retval*sizeof(double));
  //     // Store number of peaks found
  //     peak_count += data_call.pulse_index.size();
  //     // Increase pull counter
  //     pull_count += retval;	
  //     // Get time after suppression
  //     t2 = std::chrono::high_resolution_clock::now();          
  //   }
  // }      

  // sup_data_len = pull_count;

  // // Count time in ms
  // time_ms = std::chrono::duration<double, std::milli>(t2-t1).count();   
  
  // // Get data processing speed in Gbit/s
  // double speed = n*sizeof(int16_t)*8*1e-9/(time_ms*1e-3);

  // std::cout<< "\nData length: " << sup_data_len << std::endl;
  // std::cout << "Data size: " << sup_data_len*sizeof(int16_t)*1e-9 << " Gbytes" << std::endl;
  // std::cout<< "Number of triggers/peaks found: ZS = " << peak_count << std::endl;
  // std::cout << "ZS + MWD Algorithm computing time = " << time_ms << " milliseconds" << std::endl;
  // std::cout << "ZS + MWD computing speed = " << speed << " Gbit/s" << std::endl;

  // // std::cout<< "Rate = " << rate*1e-3 <<  " kHz" << std::endl;
  // // std::cout<< "Baseline mean = " << baseline_mean << " ± " << baseline_rms << std::endl;
  
  
}

// Allocate queues, threads and memoru
void allocation(){
  
  // Get the available memeory of the system                                                            
  std::cout << "\nCalculating memory allocation for queues..." << std::endl;
  uint64_t free_mem = sysconf(_SC_AVPHYS_PAGES)*sysconf(_SC_PAGE_SIZE);
  std::cout << "Available system memory  = " << free_mem*1e-9 << " Gbytes" << std::endl;
  // Set to 70% of available memeory
  free_mem *= mem_percent_per_core;
  std::cout << "Using " << mem_percent_per_core*100 << "% of available memory = " << free_mem*1e-9 << " Gbytes" << std::endl;


  // Calculate rough data struct size with 1 off-spill event
  static const int event_num_per_struct = 10;
  static const double struct_size_int16 = event_num_per_struct*(off_spill_len+fw_tHdr_Len)*sizeof(int16_t);
  static const double struct_size_double = event_num_per_struct*(off_spill_len*sizeof(double)+fw_tHdr_Len*sizeof(int16_t));
  std::cout << "Each int16_t data struct has rough size " << struct_size_int16*1e-9 << " Gbytes" << std::endl;
  std::cout << "Each double data struct has rough size " << struct_size_double*1e-9 << " Gbytes" << std::endl;

  // Get the ratio of double/int16_t struct sizes
  double double_int_ratio = struct_size_double/struct_size_int16;
  std::cout << "Ratio of double/int16_t struct sizes = " << double_int_ratio << std::endl;

  // Get number of int16_t and double queue buffers  
  int int_qs = 0;
  int double_qs = 0;
  for (int i = 0; i < process_max; i++){
    if (process[i]){
      q_num++;
      if (queue_type[i] == "int16_t"){
	int_qs++;      
      }
      else if (queue_type[i] == "double"){
	double_qs++;
      }
      else{
	std::cout << "Do not recognise queue type: " << queue_type[i] << std::endl;
      }
    }
  }
  
  // Calculate queue buffer sizes
  double queue_factor = int_qs*1 + double_qs*double_int_ratio;
  double int_buffer_size = int_qs*1/queue_factor*free_mem/int_qs;
  double double_buffer_size = double_qs*double_int_ratio/queue_factor*free_mem/double_qs;
  std::cout << "Allocating " << int_qs << " int16_t queues of size " << int_buffer_size*1e-9 << " GB" << std::endl;
  std::cout << "Allocating " << double_qs << " double queues of size " << double_buffer_size*1e-9 << " GB" << std::endl;

  // Create queues
  for (int i = 0; i < q_num; i++){
    if (queue_type[i] == "int16_t") queues[i] = new queue_zs<int16_t> (event_num_per_struct,int_buffer_size);
    if (queue_type[i] == "double") queues[i] = new queue_zs<double> (event_num_per_struct,double_buffer_size);
  }

}

// Original algorithms
void do_orig_algs(int16_t* ADC){

  std::cout << "\nPerforming original algorithms" << std::endl;
  std::cout << "-----" << std::endl;

  // Get time before ZS + MWD
  t1 = std::chrono::high_resolution_clock::now();

  // Call zero-supressin algorithmls
  std::tuple<int16_t*,uint64_t,uint64_t> output = zsup.ZS_safe(0,n,ADC);

  // Get output data
  int16_t* sup_data1 = std::get<0>(output);
  sup_data_len = std::get<1>(output);
  uint64_t peak_count = std::get<2>(output);

  // Do MWD
  double* mwd_data1 = mwd.mwd_algorithm_original(sup_data_len,sup_data1);
  std::pair<double,double> baseline = mwd.calculate_baseline(sup_data_len,mwd_data1);
  std::cout << "MWD baseline = " << baseline.first << " ± " << baseline.second << std::endl;
  MWD_peaks mwd_peaks1 = mwd.find_peaks(sup_data_len,mwd_data1,0,0,0);

  // Get time after ZS + MWD
  t2 = std::chrono::high_resolution_clock::now();

  // Get time in ms
  double time_ms = std::chrono::duration<double, std::milli>(t2-t1).count();
  // Get data processing speed in Gbit/s
  double speed = n*sizeof(int16_t)*8*1e-9/(time_ms*1e-3);

  std::cout<< "Data length after ZS: " << sup_data_len << std::endl;
  std::cout << "Data size after ZS: " << sup_data_len*sizeof(int16_t)*1e-9 << " Gbytes" << std::endl;
  std::cout<< "Number of triggers/peaks found: ZS = " << peak_count << ", MWD = " << mwd_peaks1.npeaks << std::endl;
  std::cout << "ZS + MWD Algorithm computing time = " << time_ms << " milliseconds" << std::endl;
  std::cout << "ZS + MWD computing speed = " << speed << " Gbit/s" << std::endl;

}

// Read the input data from file
int16_t* read_data(int argc, char* argv[]){

  // Get the filename from the argument
  std::string filename  = std::string(argv[1]);
  
  // Print for user
  std::cout << "filename = " << filename << std::endl;
  
  // Get the size of the binary file
  struct stat st;
  stat(filename.c_str(), &st);
  uint64_t file_size = st.st_size; // bytes

  // Get the number of ADC values
  n = file_size/sizeof(int16_t); 

  // Print for user
  std::cout << "File size = " << (double)file_size*1e-9 << " Gbytes" << std::endl;
  std::cout << "Number of ADC values = " << n << std::endl;
  
  //Open the binary file with data to suppress
  std::ifstream myFile;
  int16_t* ADC = new int16_t[n];
  myFile.open(filename, std::ios::in | std::ios::binary);
  myFile.read( (char*) ADC, n*sizeof(ADC[0]));
  myFile.close();

  // Generate data
  //  genData gen(4,10);

  // Form the ADC data into events
  gen = new genData(ADC,n);  

  // Return the ADC data
  return ADC;

}

// Main functin
int main(int argc, char* argv[]){

  // Read input data
  int16_t* ADC = read_data(argc,argv);
 
  // Perform original algorithms
  do_orig_algs(ADC);  

  // Allocate queues
  allocation();
  
  
  // // Generate events
  // std::pair<uint64_t,int16_t*> events = gen->genEvents(0,gen->numberhb);

  // // Store event array length                                           
  // uint64_t event_array_len = events.first;
  // // Store event data                                                   
  // int16_t* event_data = events.second;  

  // std::cout << "\nWith headers + random number of events" << std::endl;
  // std::cout << "-----" << std::endl;
  
  // // Set ZS time to 0
  // time_ms = 0;
  // // Reinitialise peak count
  // peak_count = 0;
  // // New suppressed data array
  // n = event_array_len;
  // int16_t* sup_data2 = new int16_t [n] ();


  // // New MWD data array
  // double* mwd_data2 = new double [n] ();
  
  // // Reinitialise ZS variables
  // zsup.first_peak[0] = true;
  // zsup.peak_found[0] = false;
  // //  zsup.left_to_store[0] = 0;
  // zsup.peak_separation[0] = 0;
  // zsup.prev_num[0] = 0;  

  // // Start push to queue thread
  // std::thread *push_thread = new std::thread(push,0,event_data,event_array_len);

  // // Start push to zs thread
  // std::thread *push_zs_thread = new std::thread(push_to_zs,0,sup_data2);

  // // Pulse finding thread
  // std::thread *pulse_thread = new std::thread (&daqZS::pulse_thread,
  // 					       std::ref(zsup),
  // 					       0,
  // 					       std::ref(event_queue),
  // 					       std::ref(pulse_queue),
  // 					       timeout);

  // // Zero suppression thread
  // std::thread *suppress_thread = new std::thread (&daqZS::suppress_thread,
  // 						  std::ref(zsup),
  // 						  0,
  // 						  std::ref(pulse_queue),
  // 						  std::ref(sup_queue),
  // 						  timeout);

  // // MWD deconv thread
  // std::thread *deconv_thread = new std::thread (&MWD::deconv_thread,
  // 						std::ref(mwd),
  // 						0,
  // 						std::ref(sup_queue),
  // 						std::ref(deconv_queue),
  // 						timeout);

  // // // MWD diff thread
  // // std::thread *diff_thread = new std::thread (&MWD::diff_thread,
  // // 					      std::ref(mwd),
  // // 					      0,
  // // 					      std::ref(deconv_queue),
  // // 					      std::ref(diff_queue),
  // // 					      timeout);

  // // // MWD average thread
  // // std::thread *avg_thread = new std::thread (&MWD::avg_thread,
  // // 					     std::ref(mwd),
  // // 					     0,
  // // 					     std::ref(diff_queue),
  // // 					     std::ref(avg_queue),
  // // 					     timeout);
  
  
  // // Start test pull                                                      
  // std::thread *pull_zs_thread = new std::thread(pull_from_zs,
  // 						0,
  // 						//std::ref(sup_data2));
  //   						std::ref(mwd_data2));
  
  // // Join threads
  // push_thread->join();
  // push_zs_thread->join();
  // pulse_thread->join();
  // suppress_thread->join();
  // deconv_thread->join();
  // // diff_thread->join();
  // // avg_thread->join();
  // pull_zs_thread->join();

  // // Compare data
  // bool success = true;
  // uint64_t fail_count = 0;
  // for (int i = 0; i < sup_data_len; i++){
  //   //    int16_t round_mwd_data1 = round(mwd_data1[i]);
  //   //    if (round_mwd_data1 != mwd_data2[i]){
  //   if (mwd_data1[i] != mwd_data2[i]){
  //     if (fail_count < 10){
  // 	if (fail_count == 0) std::cout << "CORRECT: " << i-1 << " /  " << sup_data_len << "   " << mwd_data1[i-1] << "   " << mwd_data2[i-1] << std::endl;
  //       std::cout << i << " /  " << sup_data_len << "   " << mwd_data1[i] << "   " << mwd_data2[i] << std::endl;
  //     }    
  //     fail_count++;
  //     success = false;
  //   }
  // }
  // if (success) std::cout << "SUCCESS: YOU ARE NOT A TIT!" << std::endl;
  // if (!success){
  //   std::cout << fail_count << "/" << sup_data_len << " (" << (double)fail_count/(double)sup_data_len*100 << "%) incorrect elements." << std::endl;
  //   std::cout << "FAILURE: YOU ARE A HUGE TIT!" << std::endl;
  // }

  
  return 0; 

}

