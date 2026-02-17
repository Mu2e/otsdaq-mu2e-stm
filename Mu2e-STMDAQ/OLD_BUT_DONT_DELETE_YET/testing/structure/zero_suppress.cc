// Ring buffer header
#include "ring_buffer.hh" 
// Zero suppression header
#include "zero_suppress.hh" 

// Constructor
ZeroSuppress::ZeroSuppress(){}

// Example data operation
void ZeroSuppress::operation(const std::shared_ptr<DataStruct>& buffer){

  // Resize buffer 2 to be the same size as buffer 1
  buffer->data2.resize(buffer->data.size());
  buffer->size2 = buffer->size;
  for (size_t i = 0; i < buffer->size; ++i){
    buffer->data2[i] = buffer->data[i] * 2;
  }             
  
}
