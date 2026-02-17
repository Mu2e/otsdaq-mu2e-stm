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

// Timing variables
std::chrono::high_resolution_clock::time_point t1 ;
std::chrono::high_resolution_clock::time_point t2 ;
 
// Calculate the gradient for ADC values separated a distance window
// (in time would be window * tadc) 

// The gradient window (gradient between 100 ADC values)
static const uint64_t window = 100;

// Number of ADC values to average in the gradient
int n_average = 5;

// ADC samling frequency (MHz)
//const double fADC = 320.0520833313;
static const double fADC = 370;
//Sampling time of ADC (microsec)
static const double tadc = 1.0/(fADC);

// Store 1 us before found peak
static const double before_peak_max_time = 1; // us
static const uint before_peak_max = int(before_peak_max_time/tadc);

//store after_peak_time microseconds of data to the right of the trigger
static const double after_peak_max_time = 2; // us
static const uint after_peak_max = int(after_peak_max_time/tadc);

// Total number of ADC values stored per peak
static const int total_peak_max = before_peak_max + after_peak_max;

// Gradient threshold 
static const int threshold = -100;

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
  myFile.close();

  // Get time before suppression
  t1 = std::chrono::high_resolution_clock::now();

  //----------------------- PEAKS SUPPRESSION ALGORITHM-----------------------------------------

  std::cout << "Supressing data..." << std::endl;
  
  // Store suppressed data
  int16_t* suppressed_data = new int16_t[n-window];

  // Calculate the average of the gradient each
  // n_average ADC values to avoid fluctuations
  uint64_t j = 0;

  // Boolean if peak found                                               
  bool peak_found = false;

  // Counter for number of peaks found
  uint64_t peak_count = 0;

  // The suppressed data length
  uint64_t sup_data_len = 0;  

  // Boolean to signal the first peak
  bool first_peak = true;

  // The time of the previous peak
  uint64_t prev_peak_loc = 0;

  // Loop over all ADC values 
  while(j<(n-window)){
    
    // Each point of the gradient and ADCtime
    // averaged with the (n_average-1) following
    // points, each point is the mean of n_average points of the gradient
    if((j + n_average) > (n - window)){
      n_average= (n-window)-j;
    }

    // Initialise average gradient / time variables
    double av_gradient = 0;
    double av_ADCtime = 0;    

    // Loop over all average entries                                     
    for(int k = 0; k < n_average; k++){
      // Calculate gradient                                              
      int16_t grad_high = ADC[j+k+window];
      int16_t grad_low = ADC[j+k];
      int16_t gradient = grad_high - grad_low;
      // Sum average gradient                                            
      av_gradient += gradient;
      // Sum average time                                                
      double time = j+k;
      av_ADCtime += time;
    }

    // Calculate average gradient                                        
    double avg_gradient = av_gradient/n_average;
    // Calculate average trigger time                                    
    uint64_t peak_loc = av_ADCtime/n_average;
    
    // Increase loop counter                                             
    j += n_average;

    // If the averaged gradient is beyond the threshold
    if(avg_gradient > threshold){
      // No peak found
      peak_found=false;
      continue;
    }

    // Skip the rest indexes of the peak after                           
    // the trigger that has already been stored                          
    if(avg_gradient < threshold && peak_found==true){
      continue;
    }
    
    // If within the threshold and no peak previously found...
    if(avg_gradient < threshold && peak_found==false){

      // Peak found! 
      peak_found=true;

      // Increment the peak counter
      peak_count++;

      // The index location to copy from...
      uint64_t loc_copy_from = 0;
      // The data length to copy
      uint64_t data_len = 0;
      
      // If first peak
      if(first_peak){

	// Check whether there is 1 us of data before the first peak...
	// If not, the number of ADC values is: peak_loc
	if(peak_loc - before_peak_max < 0){	  

	  // Copy from first element in ADC array
	  loc_copy_from = 0;

	  // Store the data from ADC[0] --> peak_loc+after_peak_max
	  data_len = peak_loc + after_peak_max;

	}

	// Else if 1 us or more exists before the start of the first peak
	else{

	  // Copy from 1 us before peak
	  loc_copy_from = peak_loc-before_peak_max;	 

	  // Store the whole peak
	  data_len = total_peak_max;
 	}

	// Set first peak to false
	first_peak = false;
      
      } // End if first peak

      // Else if not first peak
      else{

	// Find the number of values between this peak
	// and the previous one
	int peak_sep = peak_loc - prev_peak_loc;
	
	// If the peak seperation is within the peak window
	if(peak_sep < total_peak_max){

	  // Copy from end of previous peak
	  loc_copy_from = prev_peak_loc + after_peak_max;

	  // Store just the new peak separation
	  data_len = peak_sep;

	}
	// Else if this new peak is outside the 3us window of the previous peak
	else{

	  // Copy from 1 us before peak
	  loc_copy_from = peak_loc-before_peak_max;
	  
	  // Store the total peak
	  data_len = total_peak_max;
	  
	} // End if (peak_sep < total_peak_max)
	
	// If the data to copy goes beyond the data length
	if(loc_copy_from + data_len > n){
	  
	  // Store only until end of data
	  data_len = n - loc_copy_from;
	  
	}
	
      } // End if first peak
            
      // Memcpy the data
      memcpy(&suppressed_data[sup_data_len],
      	     &ADC[loc_copy_from],
      	     data_len*sizeof(int16_t));
      
      // Increase suppressed data size
      sup_data_len += data_len;

      // Store peak time to compare against next peak
      prev_peak_loc = peak_loc;

    } // End if peak found

  } // End loop over ADC values

  // Get time after suppression
  t2 = std::chrono::high_resolution_clock::now();

  //Write ADC peaks to the binary file
  std::string s="hi.bin";
  int size = s.length();
  char file_name[size+1];
  // copying the contents of the
  // string to char array
  strcpy(file_name, s.c_str());
  FILE * fp = fopen(file_name, "wb");                                               
  fwrite(&suppressed_data[0], sizeof(int16_t), sup_data_len, fp);
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
  
  return 0;



}

