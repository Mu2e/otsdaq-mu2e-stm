/////////////////////////////////////////////////////////////////
/// Module contains various helpers such a samples manipulation.
/////////////////////////////////////////////////////////////////

#ifndef UTILS_HH
#define UTILS_HH

// Hardware scripts
#include "STMDAQ-TestBeam/hardwareScripts/IPBusManager.hh"

class utils {

public:

  utils();

  // Generate a 16 bit waveform into a previously allocated memory buffer. 
  // This function can generate several data types and both signal period  
  // as well as amplitude are configurable.   
  int16_t* GenerateWaveform16(uint32_t numbersamples, 
			      uint32_t period, 
			      uint32_t amplitude, 
			      uint8_t datatype);

  // Function to convert integer to binary string
  std::string int2bin(uint16_t integer,int digits);

  // Get flag to generate sine wave
  uint64_t genSineWave(){
    return DATAGEN_SINE_WAVE;
  }
  
  // Get flag to generate saw wave
  uint64_t genSawWave(){
    return DATAGEN_SAW_WAVE;
  }


private : 

  // Flag to generate sine wave
  uint64_t DATAGEN_SINE_WAVE = 0;
  // Flag to generate saw wave
  uint64_t DATAGEN_SAW_WAVE = 1;

};

#endif
