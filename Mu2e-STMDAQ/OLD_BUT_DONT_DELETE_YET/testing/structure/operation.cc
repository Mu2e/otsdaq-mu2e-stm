// Ring buffer header
#include "ring_buffer.hh" 
// Dummy operation header
#include "operation.hh" 

// Constructor
DummyOperation::DummyOperation(){}

// Example data operation
void DummyOperation::operation (const std::shared_ptr<DataStruct>& buffer){
  
  // Loop over all data in buffer
  for (size_t i = 0; i < buffer->size; ++i) {
    // Increment each data element by 1
    buffer->data[i] += 1; 
  }

}
