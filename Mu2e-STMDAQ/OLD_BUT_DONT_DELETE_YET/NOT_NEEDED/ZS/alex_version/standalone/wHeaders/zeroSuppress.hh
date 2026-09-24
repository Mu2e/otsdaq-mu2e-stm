////////////////////////////////////////////////////////////////////////////////// 
/// This module ZSs the data as part of the ZS thread (header).
///////////////////////////////////////////////////////////////////////////////////

#ifndef ZS_hh
#define ZS_hh

#include<iostream>
#include<fstream>
#include<vector>
#include<tuple>

class daqZS {

private:

  // The gradient window (gradient between 100 ADC valuess
  uint64_t window = 100;

  // Number of ADC values to average in the gradient
  int n_average = 5;

  // ADC samling frequency (MHz)
  //  const double fADC = 320.0520833313;
  const double fADC = 370;
  //Sampling time of ADC (microsec)
  double tadc = 1.0/(fADC);

  // Store 1 us before found peak
  double before_peak_max_time = 1; // us                          
  uint before_peak_max = int(before_peak_max_time/tadc);

  //store after_peak_time microseconds of data to the right of the trigger
                double after_peak_max_time = 2; // us                           
  uint after_peak_max = int(after_peak_max_time/tadc);

  // Total number of ADC values stored per peak
  int total_peak_max = before_peak_max + after_peak_max;

  // The maximum amount of data to store from the previous event
  int prev_data_max = before_peak_max + window + n_average;
  
  // Gradient threshold
  int threshold = -100;

  // HPGe decay time constant
  double hpge_decay_time = 200; // us
  uint hpge_decay_adc = int(hpge_decay_time/tadc);

public:

  // Standard constructor - shouldn't be used
  daqZS();

  // The channel number [DELETE LATER]
  static const uint CHNUM = 2;

  // The number of data points to average from the previus event
  int prev_num[CHNUM] = {};

  // The end of the previous data event
  int16_t* prev_data_end[CHNUM] = {new int16_t [prev_data_max] (),
				   new int16_t [prev_data_max] ()};
  
  // Boolean to signal first peak found in run
  bool first_peak[CHNUM] = {true,true};
  
  // Boolean to signal a peak has been found
  bool peak_found[CHNUM] = {false,false};

  // The separation between two peaks
  uint64_t peak_separation[CHNUM] = {};

  uint16_t avg_count[CHNUM] = {};
    
  // The data left to store after finding a peak
  uint64_t left_to_store[CHNUM] = {};  
  
  // Get private ZS window
  uint64_t get_window(){
    return window;
  }
  
  // Get private ZS average num
  int get_n_average(){
    return n_average;
  }

  // Get private ZS before peak max
  uint get_before_peak_max(){
    return before_peak_max;
  }

  // Get private ZS after peak max
  uint get_after_peak_max(){
    return after_peak_max;
  }

  // Get private ZS total peak max
  uint get_total_peak_max(){
    return total_peak_max;
  }
  
  // Get private ZS threshold
  int get_threshold(){
    return threshold;
  }


  // ZS the data
  std::tuple<int16_t*,uint64_t,uint64_t,uint64_t,int64_t,int64_t> do_ZS(int chan, 
									uint64_t data_len, 
									int16_t* &data);
  
  // ZS the data
  std::tuple<int16_t*,uint64_t,uint64_t> ZS_safe(int chan, 
					       uint64_t data_len, 
					       int16_t* &data);

  
};

#endif
