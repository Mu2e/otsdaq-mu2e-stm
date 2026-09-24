#ifndef GEN_DATA_hh_
#define GEN_DATA_hh_

#include <iostream>
#include <fstream>
#include <vector>
#include <memory>
#include <cstdint>
#include <string>
#include <limits>

// Standard thread functions
class GenData {

private:

  // The ADC data
  std::shared_ptr<std::vector<int16_t>> data;

  // Size of the data vector
  size_t data_size = 0;
  // Length of the data vector
  size_t data_len = 0;
  
  // ADC sampling frequency (MHz)                                          
  static constexpr double fADC = 300;
  // ADC sampling time                                                     
  static constexpr double tADC = 1/fADC;

  // Number of events 
  double event_num;
  // Event length (us)
  const size_t event_len;
  // Event size (bytes)
  size_t event_size;

  // Packet number counter
  uint32_t packet_num = 0;
  // EWT counter
  uint64_t EWT = 0;
  // Data counter
  size_t data_count;

public:

  // Constructor 
  GenData(size_t event_num_, size_t event_len_, const std::string& filepath);

  // Function to get packet
  void get_packets();
  
  // Return the filled data vector
  std::shared_ptr<std::vector<int16_t>> genData() const {
    return data;
  }
  
};

#endif 
