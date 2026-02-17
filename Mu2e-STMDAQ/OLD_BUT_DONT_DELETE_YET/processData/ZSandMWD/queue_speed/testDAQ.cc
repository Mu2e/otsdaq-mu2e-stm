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

// Queue header
#include "queue_zs.hh"

// Gen data                                                                 
#include "genData.hh"

// Generate data class
genData *gen;

// Split events into arrays of random integer no of events
int event_max = 10;

// Timing variables
std::chrono::high_resolution_clock::time_point t1 ;
std::chrono::high_resolution_clock::time_point t2 ;

// Dummy timeout varaibles
bool b = false;
bool* timeout = &b;

double speed = 0;

// Queue thread function
void queue_thread(int chan, int q_num, queue_zs<int16_t> &pullq, queue_zs<int16_t> &pushq, bool *timeout){

  //  std::cout << "Entered thread " << q_num << std::endl;

  // The return calue
  int retval = 0;

  // The total pulse count                                                 
  uint64_t data_count = 0;

  // Data buffer                                                           
  //  data_struct<int16_t> buffer;

  // While still data to pull                                              
  while(!*timeout){
    // Pull data from queue                                                
    retval = pullq.pull(timeout,chan,pullq.data_buffer);
    if (retval > 0){
      // If the queue_number is odd
      if (q_num % 2 != 0){
	// Add 1 to all elements
	for (int i = 0; i < pullq.data_buffer.data_len; i++){
	  pullq.data_buffer.adc_data[i]++;
	}
      }
      // Subtract 1 from all elements
      else{
	for (int i = 0; i < pullq.data_buffer.data_len; i++){
	  pullq.data_buffer.adc_data[i]--;
	}
      }
      // Push data to queue
      pushq.push(chan,pullq.data_buffer);
    }
  }

  //  std::cout << "Leaving thread " << q_num << std::endl;
  
}


// Put data into data structs
void split_headers(int chan, uint64_t n, int16_t* &data, data_struct<int16_t> &event_data){
  
  // The header int16_t counter
  uint64_t header_count = 0;

  // The start index of each event
  int64_t i = 0;

  // Header start index location                                    
  uint64_t hdr_start_loc = 0;
  // event length                                                   
  uint16_t event_len = 0;
    
  // Loop over all ADC values 
  while(i<n){       

    //    std::cout << i << " / " << n << std::endl;

    // Get header start index location                                    
    hdr_start_loc = i;
    
    // Get event length index location                                    
    uint64_t event_len_loc = hdr_start_loc + fw_tHdr::EvLen;
    // Get event length                                                   
    event_len = data[event_len_loc];           
    
    // Memcpy event header
    memcpy(&event_data.header_data[header_count],
    	   &data[hdr_start_loc],fw_tHdr_Len*sizeof(int16_t));

    // Skip past event header
    i += fw_tHdr_Len;      

    //    std::cout << "Header copied" << std::endl;

    // Account for header
    header_count += fw_tHdr_Len;

    // Memcpy adc data
    memcpy(&event_data.adc_data[event_data.data_len],
    	   &data[i],event_len*sizeof(int16_t));
     
    // Update i to be same as j
    i += event_len;

    //    std::cout << "Data copied" << std::endl;

    // Store the start of the event in the adc data
    event_data.start_index.push_back(event_data.data_len);

    // Account for header
    event_data.data_len += event_len;
   
    // Increment the event counter
    event_data.count++;                

  } // End loop over ALL event data

  //  std::cout << "Leaving split_headers" << std::endl;

}

 
// Start thread function                                            
void start(int chan, int16_t *data, uint64_t len, queue_zs<int16_t> &pushq, int event_num){

  //  std::cout << "In start thread" << std::endl;

  // Initialise event counter
  uint64_t event_count = 0;
  // Initialise total counter
  uint64_t tot_count = 0;  
  // Initialise new array
  int16_t* array = new int16_t [len]();
  //  time_t seed = time(NULL);
  // time_t seed = 1715953952;
  // std::cout << "Random Seed = " << seed << std::endl;
  // srand(seed);
  // While event_counter < number of events
  while(event_count < gen->numberhb){
    // Get a random number of events
    //    int event_num = 1 + (rand() % event_max);
    //    int event_num = gen->numberhb/2;
    // If the event_count + the number of events is greater than
    // the maximum number of events, only push remainder
    if ((event_count + event_num) > gen->numberhb){
      event_num = gen->numberhb - event_count;
    }
    // Initalise counter
    uint64_t count = 0;
    // Initialise new array length
    uint64_t array_len = 0;    
    // Loop over events to ZS
    while (count < event_num){
      // Get event length
      uint16_t event_len = data[array_len + fw_tHdr::EvLen];
      // Copy event to new array
      memcpy(&array[array_len],
     	     &data[tot_count+array_len],
     	     (fw_tHdr_Len + event_len)*sizeof(int16_t));
      // Increase array length
      array_len += fw_tHdr_Len + event_len;
      // Increase event counter
      count++;
    }
    data_struct<int16_t> event_data;
    event_data.header_data = new int16_t [event_num * fw_tHdr_Len] ();
    event_data.adc_data = new int16_t [event_num * off_spill_len] ();    
    // Form event data into a data struct
    split_headers(0,array_len,array,event_data);
    // Push the data to the queue                                          
    pushq.push(chan,event_data);    
    // Increase event_count
    event_count += event_num;
    // Increase total counter
    tot_count += array_len;

  }      

  // Wait five seconds to timeout
  for (int i = 0; i < 3; i++){
    // std::cout << "Waiting " << 5-i << "..." << std::endl;
    std::this_thread::sleep_for(std::chrono::seconds(1));
  }
  // Signal timeout
  b = true;

}


// Pull from zs queue thread function
void end(int chan, queue_zs<int16_t> &pullq){

  //  std::cout << "In end thread" << std::endl;

  // Data buffer
  //  data_struct<int16_t> data_call;

  // The return value
  int retval = 0;

  // The total pull length
  uint64_t pull_count = 0;

  // The total data length
  uint64_t tot_len = 0;
  
  // Time counter
  double time_ms = 0;  
  
  // While still data to pull
  while(!*timeout){    
    // Pull data from queue
    //    retval = pullq.pull(timeout,chan,data_call);
    retval = pullq.pull(timeout,chan,pullq.data_buffer);
    //retval = pulse_queue.pull(timeout,chan,data_call);
    if (retval > 0){
      if (tot_len == 0) t1 = std::chrono::high_resolution_clock::now();          
      // Memcpy data from buffer
      //      memcpy(&data[pull_count],data_call.adc_data,retval*sizeof(int16_t));      
      // Store number of peaks found
      tot_len += pullq.data_buffer.data_len;
      // Increase pull counter
      pull_count += retval;	
      // Get time after suppression
      t2 = std::chrono::high_resolution_clock::now();          
    }
  }      

  // Count time in ms
  time_ms = std::chrono::duration<double, std::milli>(t2-t1).count();   
  
  // Get data processing speed in Gbit/s
  speed = pull_count*sizeof(int16_t)*8*1e-9/(time_ms*1e-3);

  // std::cout<< "\nData length: " << pull_count << std::endl;
  // std::cout << "Data size: " << pull_count*sizeof(int16_t)*1e-9 << " Gbytes" << std::endl;
  // std::cout << "Computing time = " << time_ms << " milliseconds" << std::endl;
  // std::cout << "Computing speed = " << speed << " Gbit/s" << std::endl;
  
}

// Main functin
int main(int argc, char* argv[]){

  // Generate data
  //  genData gen(4,10);
  // Generate data
  gen = new genData(2e5,0x30D4);  

  // Generate events
  std::pair<uint64_t,int16_t*> events = gen->genEvents(0,gen->numberhb);

  std::cout << "Event ADC length = " << gen->deltahb_len << std::endl;

  // Store event array length                                           
  uint64_t event_array_len = events.first;
  // Store event data                                                   
  int16_t* event_data = events.second;  

  // Off-events per data struct
  int event_num = 1;
  
  data_struct<int16_t> test;
  test.header_data = new int16_t [event_num * fw_tHdr_Len] ();
  test.adc_data = new int16_t [event_num * off_spill_len] ();
  double struct_size = event_num * (off_spill_len+fw_tHdr_Len)*sizeof(int16_t);
  std::cout << "Each data struct has rough size " << struct_size*1e-9 << " Gbytes" << std::endl;
  
  // Get the available memeory of the system
  std::cout << "Calculating memory allocation..." << std::endl;
  uint64_t free_mem = sysconf(_SC_AVPHYS_PAGES)*sysconf(_SC_PAGE_SIZE);
  std::cout << "Available system memory  = " << free_mem*1e-9 << " Gbytes" << std::endl;
    
  // Number of queues
  int q_num = 2;
  
  // Calculate available mem
  double mem_percent_per_core = 0.7;
  std::cout << "Setting queue sizes to " << mem_percent_per_core*100 << "% of available memeory" << std::endl;
  uint64_t mem_per_queue = free_mem/q_num*mem_percent_per_core;
  
  std::cout << q_num << " queues = " << mem_per_queue*1e-9 << " Gbytes per queue" << std::endl;
  
  int buffer_size = floor(mem_per_queue/struct_size);
  
  std::cout << "Queues to be allocated " << buffer_size << " data structs" << std::endl;
  
  // Create queues
  //  queue_zs<int16_t> *queues = new queue_zs<int16_t> [q_num];
  queue_zs<int16_t> *queues[q_num];
  for (int i = 0; i < q_num; i++){
    queues[i] = new queue_zs<int16_t> (event_num,buffer_size);
  }
  
  sleep(1);
  
  free_mem = sysconf(_SC_AVPHYS_PAGES)*sysconf(_SC_PAGE_SIZE);
  std::cout << "Available system memory after queue allocation  = " << free_mem*1e-9 << " Gbytes" << std::endl;
  
  // Start thread
  std::thread *start_thread = new std::thread(start,0,
					      event_data,event_array_len,
					      std::ref(*queues[0]),
					      event_num);
  
  std::thread *threads[q_num];
  
  // Loop over number of queues
  for (int i = 0; i < q_num-1; i++){
    threads[i] = new std::thread(queue_thread,0,i,
				 std::ref(*queues[i]),std::ref(*queues[i+1]),timeout);
  }
  
  // End thread
  std::thread *end_thread = new std::thread(end,0,std::ref(*queues[q_num-1]));
  
  // Join threads
  start_thread->join();
  // Loop over number of queues
  for (int i = 1; i < q_num-1; i++){
    threads[i]->join();
  }
  end_thread->join();
  
  std::cout << speed << std::endl;
 
  
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

