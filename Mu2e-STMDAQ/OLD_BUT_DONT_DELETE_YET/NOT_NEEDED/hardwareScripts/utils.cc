/////////////////////////////////////////////////////////////////
/// Module contains various helpers such a samples manipulation.
/////////////////////////////////////////////////////////////////

#include <time.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>		// For memset()
#include <math.h>
#include <bitset>

// Hardware scripts
#include "STMDAQ-TestBeam/hardwareScripts/utils.hh"

//Standard constructor - shouldn't be used
utils::utils() {}

// Generate a 16 bit waveform into a previously allocated memory buffer. 
// This function can generate several data types and both signal period 
// as well as amplitude are configurable.  
int16_t* utils::GenerateWaveform16(uint32_t numbersamples, uint32_t period, uint32_t amplitude, uint8_t datatype){

  int32_t amp = 0;
  double pi    = 3.1415926535;
  int32_t tmp2 = 0x0;
  double x, y;
  
  // set our wav_buffer with known value (0). Note the MUL(2) because memset takes a byte size
  int16_t* wav_buffer;
  wav_buffer = new int16_t[numbersamples];
  for (uint32_t i = 0; i < numbersamples; i++) wav_buffer[i] = 0;
  
  // proceed with the data generation
  if (datatype == DATAGEN_SINE_WAVE){
    for(uint32_t i=0; i < numbersamples/2; i++) {
      amp=amplitude/2-1;
      x = (pi /period*2)*(i*2+0);
      y = sin(x);
      wav_buffer[2*i+0] =  ((int16_t)(y*amp));
      x = (pi /period*2)*(i*2+1);
      y = sin(x);
      wav_buffer[2*i+1] =  ((int16_t)(y*amp));
    }
  }
  else if (datatype == DATAGEN_SAW_WAVE){
    for(uint32_t i=0; i < numbersamples/2; i++) {
      tmp2 =(0xffff)& ((2*i)%period*(amplitude-1)/period*2);
      wav_buffer[2*i+0] = (uint16_t)(tmp2);
      tmp2 = (0xffff)& ((2*i)%period*(amplitude-1)/period*2);
      wav_buffer[2*i+1] = (uint16_t)(tmp2);
    }
  }
  
  return wav_buffer;
  
}

// Function to convert integer to binary string
std::string utils::int2bin(uint16_t integer, int digits){

  std::string binary;
  
  // If integer is negative...
  if (integer < 0) integer = pow(2,digits) + integer;
  
  // Converts to binary string with given number of digits
  if (digits == 8){
    binary = std::bitset< 8 >( integer ).to_string();
  }
  else if (digits == 16){
    binary = std::bitset< 16 >( integer ).to_string();
  }
  else if (digits == 32){
    binary = std::bitset< 32 >( integer ).to_string();
  }
  else if (digits == 64){
    binary = std::bitset< 64 >( integer ).to_string();
  }
  else{
    std::cout << "int2bin in utils.cc must be given: digits = 8,16,32,64!!!" << std::endl;
    exit(0);
  }
  
  return binary;

}
