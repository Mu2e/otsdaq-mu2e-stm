// Ring buffer header
#include "ring_buffer.hh" 
// MWD header
#include "mwd.hh" 

// Constructor
MWD::MWD(){}

// Example data operation
void MWD::operation(const std::shared_ptr<DataStruct>& buffer){

  // Resize buffer 3 to be the same size as buffer 2
  buffer->data3.resize(buffer->data2.size());                                 
  buffer->size3 = buffer->size2;                                              
  for (size_t i = 0; i < buffer->size2; ++i) {                                
    double temp = static_cast<double>(buffer->data2[i]) / 4.0;                 
    buffer->data3[i] = static_cast<int16_t>(temp);                             
  }                                                                           
}
