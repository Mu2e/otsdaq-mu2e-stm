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

// Zero-supress data header
#include "zeroSuppress.hh"

// Gen data                                                                 
#include "genData.hh"

// Zero-suppression class
daqZS zsup;

// Timing variables
std::chrono::high_resolution_clock::time_point t1 ;
std::chrono::high_resolution_clock::time_point t2 ;
 
// Calculate the gradient for ADC values separated a distance window
// (in time would be window * tadc) 

// The gradient window (gradient between 100 ADC values)
static const uint64_t window = zsup.get_window();

// Number of ADC values to average in the gradient
int n_average = zsup.get_n_average();

// Store 1 us before found peak
static const uint before_peak_max = zsup.get_before_peak_max();

//store after_peak_time microseconds of data to the right of the trigger
static const uint after_peak_max = zsup.get_after_peak_max();

// Total number of ADC values stored per peak
static const int total_peak_max = zsup.get_total_peak_max();

// Gradient threshold 
static const int threshold = zsup.get_threshold();

// Main functin
int main(int argc, char* argv[]){

  // Get the filename from the argument
  std::string filename  = std::string(argv[1]);
  
  // Print for user
  std::cout << "filename = " << filename << std::endl;
  
  // Get the size of the binary file
  struct stat st;
  stat(filename.c_str(), &st);
  uint64_t file_size = st.st_size; // bytes

  // Get the number of ADC values
  uint64_t n = file_size/sizeof(int16_t); 

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
  // Generate data
  genData gen(ADC,n);

  std::cout << "\nWithout headers" << std::endl;
  std::cout << "-----" << std::endl;

  // Get time before suppression
  t1 = std::chrono::high_resolution_clock::now();

  // Call zero-supressin algorithmls
  std::tuple<int16_t*,uint64_t,uint64_t> output = zsup.ZS_safe(0,n,ADC);

  // Get time after suppression
  t2 = std::chrono::high_resolution_clock::now();

  // Get output data
  int16_t* suppressed_data_1 = std::get<0>(output);
  uint64_t sup_data_len = std::get<1>(output);
  uint64_t peak_count = std::get<2>(output);

  //Write ADC peaks to the binary file
  std::string s="hi.bin";
  int size = s.length();
  char file_name[size+1];
  // copying the contents of the
  // string to char array
  strcpy(file_name, s.c_str());
  FILE * fp = fopen(file_name, "wb");                                               
  fwrite(&suppressed_data_1[0], sizeof(int16_t), sup_data_len, fp);
  fclose(fp);

  // Get time in ms
  double time_ms = std::chrono::duration<double, std::milli>(t2-t1).count();
  // Get data processing speed in Gbit/s
  double speed = file_size*8*1e-9/(time_ms*1e-3);

  std::cout << "Zero Suppression Algorithm computing time = " << time_ms << " milliseconds" << std::endl;
  std::cout << "Zero Suppression Algorithm computing speed = " << speed << " Gbit/s" << std::endl;
  std::cout<< "Number of elements in suppressed file: " << sup_data_len << std::endl;
  std::cout << "Suppressed data size = " << sup_data_len*sizeof(int16_t)*1e-9 << " Gbytes" << std::endl; 
  std::cout<< "Number of triggers/peaks found: " << peak_count << std::endl;
  
  // Generate events
  std::pair<uint64_t,int16_t*> events = gen.genEvents(0,gen.numberhb);

  // Store event array length                                           
  uint64_t event_array_len = events.first;
  // Store event data                                                   
  int16_t* event_data = events.second;

  zsup.first_peak[0] = true;
  zsup.peak_found[0] = false;
  zsup.left_to_store[0] = 0;
  zsup.peak_separation[0] = 0;

  std::cout << "\nWith headers" << std::endl;
  std::cout << "-----" << std::endl; 
  
  // Get time before suppression
  t1 = std::chrono::high_resolution_clock::now();
  
  // Call zero-supressin algorithmls
  std::tuple<int16_t*,uint64_t,uint64_t,double,uint64_t,uint64_t> sup = zsup.do_ZS(0,event_array_len,event_data);
  
  // Get time after suppression
  t2 = std::chrono::high_resolution_clock::now();

  // Get output data
  int16_t* suppressed_data_2 = std::get<0>(sup);
  sup_data_len = std::get<1>(sup);
  peak_count = std::get<2>(sup);
  double rate = std::get<3>(sup);
  int64_t baseline_mean = std::get<4>(sup);
  int64_t baseline_rms = std::get<5>(sup);
  
  // Get time in ms
  time_ms = std::chrono::duration<double, std::milli>(t2-t1).count();
  // Get data processing speed in Gbit/s
  speed = file_size*8*1e-9/(time_ms*1e-3);

  std::cout << "Zero Suppression Algorithm computing time = " << time_ms << " milliseconds" << std::endl;
  std::cout << "Zero Suppression Algorithm computing speed = " << speed << " Gbit/s" << std::endl;
  std::cout<< "Number of elements in suppressed file: " << sup_data_len << std::endl;
  std::cout << "Suppressed data size = " << sup_data_len*sizeof(int16_t)*1e-9 << " Gbytes" << std::endl; 
  std::cout<< "Number of triggers/peaks found: " << peak_count << std::endl;
  std::cout<< "Rate = " << rate*1e-3 <<  " kHz" << std::endl;
  std::cout<< "Baseline mean = " << baseline_mean << " ± " << baseline_rms << std::endl;

  // Compare data
  bool success = true;
  for (int i = 0; i < sup_data_len; i++){
    if (suppressed_data_1[i] != suppressed_data_2[i]){
      success = false;
    }
  }
  if (success) std::cout << "SUCCESS: YOU ARE NOT A TIT!" << std::endl;
  if (!success) std::cout << "FAILURE: YOU ARE A HUGE TIT!" << std::endl;

  s="baseline.bin";
  size = s.length();
  strcpy(file_name, s.c_str());
  fp = fopen(file_name, "wb");
  fwrite(&suppressed_data_2[0], sizeof(int16_t), sup_data_len, fp);
  fclose(fp);

  std::cout << "\nWith headers + random number of events" << std::endl;
  std::cout << "-----" << std::endl;
  
  // Split events into arrays of random integer no of events
  int event_max = 1000;
  // Set ZS time to 0
  time_ms = 0;
  // Reinitialise peak count
  peak_count = 0;
  // Reinitalise sup_data_len
  sup_data_len = 0;
  rate = 0;
  baseline_mean = 0;
  baseline_rms = 0;

  // Reinitialise ZS variables
  zsup.first_peak[0] = true;
  zsup.peak_found[0] = false;
  zsup.left_to_store[0] = 0;
  zsup.peak_separation[0] = 0;
  zsup.prev_num[0] = 0;  
  // Initialise event counter
  uint64_t event_count = 0;
  // Initialise total counter
  uint64_t tot_count = 0;
  // Initialise new array
  int16_t* array = new int16_t [event_array_len]();
  // Number of zero suppression calls
  int rate_count = 0;
  int baseline_count = 0;
  // While event_counter < number of events
  while(event_count < gen.numberhb){
    // Get a random number of events
    int event_num = 1 + (rand() % event_max);
    //    int event_num = gen.numberhb/2;
    // If the event_count + the number of events is greater than
    // the maximum number of events, only push remainder
    if ((event_count + event_num) > gen.numberhb){
      event_num = gen.numberhb - event_count;
    }
    // Initalise counter
    uint64_t count = 0;
    // Initialise new array length
    uint64_t array_len = 0;    
    // Loop over events to ZS
    while (count < event_num){
      // Get event length
      uint16_t event_len = event_data[array_len + fw_tHdr::EvLen];
      // Copy event to new array
      memcpy(&array[array_len],
     	     &event_data[tot_count+array_len],
     	     (fw_tHdr_Len + event_len)*sizeof(int16_t));
      // Increase array length
      array_len += fw_tHdr_Len + event_len;
      // Increase event counter
      count++;
    }
    // Get time before suppression
    t1 = std::chrono::high_resolution_clock::now();
    // Call zero-supressin algorithmls
    sup = zsup.do_ZS(0,array_len,array);    
    // Get time after suppression
    t2 = std::chrono::high_resolution_clock::now();    
    // Get output data
    int16_t* sup_data = std::get<0>(sup);
    memcpy(&suppressed_data_2[sup_data_len],
	   sup_data,std::get<1>(sup));
    sup_data_len += std::get<1>(sup);
    peak_count += std::get<2>(sup);
    rate += std::get<3>(sup);
    baseline_mean += std::get<4>(sup);
    baseline_rms += std::get<5>(sup);    
    // Get time in ms
    time_ms += std::chrono::duration<double, std::milli>(t2-t1).count();
    
    // Increase event_count
    event_count += event_num;
    // Increase total counter
    tot_count += array_len;
    // Increment zs call couter
    rate_count++;
    if (baseline_mean != 0) baseline_count++;

  }      

  // Find average rate, baseline_mean and rms
  rate /= rate_count;
  // Ensure not dividing by zero
  if (baseline_count != 0) {
    baseline_mean /= baseline_count;
    baseline_rms /= baseline_count;
  }
  
  // Get data processing speed in Gbit/s
  speed = file_size*8*1e-9/(time_ms*1e-3);

  std::cout << "Zero Suppression Algorithm computing time = " << time_ms << " milliseconds" << std::endl;
  std::cout << "Zero Suppression Algorithm computing speed = " << speed << " Gbit/s" << std::endl;
  std::cout<< "Number of elements in suppressed file: " << sup_data_len << std::endl;
  std::cout << "Suppressed data size = " << sup_data_len*sizeof(int16_t)*1e-9 << " Gbytes" << std::endl; 
  std::cout<< "Number of triggers/peaks found: " << peak_count << std::endl;
  std::cout<< "Rate = " << rate*1e-3 <<  " kHz" << std::endl;
  std::cout<< "Baseline mean = " << baseline_mean << " ± " << baseline_rms << std::endl;

  
  // Compare data
  success = true;
  for (int i = 0; i < sup_data_len; i++){
    if (suppressed_data_1[i] != suppressed_data_2[i]){
      success = false;
    }
  }
  if (success) std::cout << "SUCCESS: YOU ARE NOT A TIT!" << std::endl;
  if (!success) std::cout << "FAILURE: YOU ARE A HUGE TIT!" << std::endl;

    return 0;



}

