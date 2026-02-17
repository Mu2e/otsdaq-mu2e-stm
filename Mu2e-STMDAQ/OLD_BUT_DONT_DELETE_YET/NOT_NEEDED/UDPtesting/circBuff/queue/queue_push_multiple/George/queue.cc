#include <iostream>
#include <thread>
#include <atomic>
#include <cinttypes>

// Queue buffer header
#include "queue.hh"

// Constructor 
template<typename T> queue_buffer<T>::queue_buffer() {
  
  // For each channel, set read and write pointers to zero
  for (int chan = 0; chan < CHNUM; chan++){
    write[chan].store(0);
    read[chan].store(0);
    buffer[chan] = new T [size];      
  }
  // // Get the maximum number of datagrams to pull based on the data type 
  // if (is_same<T,UDPsocket::packet>::value){
  //   pull_max = UDPsocket::RECVMMSG_NUM;
  // }
  // else if (is_same<T,dataVars::off_event>::value){
  //   pull_max = UDPsocket::SENDMMSG_NUM;
  // }


}

// Try to push data to queue
template<typename T> int queue_buffer<T>::try_push(int chan, T *data, 
						   int n, int index){  
  int retval = 0;

  // Get the current write pointer
  const_cast<int64_t&>(current_tail[chan]) = write[chan].load();
  
  // If write is ahead of read...
  if(write_ahead){
    // If the write pointer + n is smaller than buffer max
    if (current_tail[chan] + n <= size){
      // Push 
      retval = n;
      // memcpy to queue buffer 
      memcpy(&buffer[chan][current_tail[chan]],
	     &data[index],retval*sizeof(data[0]));
    }
    // If write is going to wrap around
    else{
      // Get the distance to the end of the queue buffer
      int end = size - current_tail[chan];
      // memcpy to end of queue buffer 
      memcpy(&buffer[chan][current_tail[chan]],
	     &data[index],end*sizeof(data[0]));
      // Get the number left to push
      int left = n - end;
      // Get available space in buffer
      int avail = read[chan].load();
      // If space for all remaining packets
      if (left <= avail){
	// memcpy to start of queue buffer 
	memcpy(&buffer[chan][0],&data[index+end],
	       left*sizeof(data[0]));
	// Update retval
	retval = end + left;
      }
      // Else if more packets that space
      else{	
	// memcpy to start of queue buffer 
	memcpy(&buffer[chan][0],&data[index+end],
	       avail*sizeof(data[0]));	
	// Update retval
	retval = end + avail;
      }
    }
  }
  // Else if read is ahead of write
  else{
    // Get available space in buffer
    int avail = read[chan].load();
    // If the write pointer + n <= read
    if (current_tail[chan] + n <= avail){
      // Push all
      retval = n;
    }
    // Else if write pointer + n > read
    else{
      // Push what's available 
      retval = avail - current_tail[chan];
    }
    // memcpy to queue buffer 
    memcpy(&buffer[chan][current_tail[chan]],
	   &data[index],retval*sizeof(data[0]));    
  }

  // Increment the next write pointer
  write[chan].store(increment(chan,current_tail[chan],retval));

  return retval;
  

  // // Get available space  
  // if (retval = size-current_tail[chan]-read[chan].load() == 0){
    
  //   // Return unsuccesful push
  //   return 0;
    
  // }

  // std::cout << "Available space = " << retval << std::endl;
  
  // // If buffer has wrapped around...
  // if (read[chan].load()-current_tail[chan] < 0){
  //   // Make sure the return value is positive
  //   retval += size;
  //   // If fewer to push than available space
  //   if (n < retval) retval = n;
  //   // Get the distance to the end of the queue buffer
  //   int end = RING_BUFFER_SIZE - current_head[chan];
  //   // memcpy to end of queue buffer 
  //   memcpy(&buffer[chan][current_tail[chan]],
  // 	   &data[index],end*sizeof(data[0]));
  //   // memcpy to start of queue buffer 
  //   memcpy(&buffer[chan][0],&data[index+end],
  // 	   (retval-end)*sizeof(data[0]));
  // }
  // // If copying from central region of buffer
  // else{    
  //   // If fewer to push than available space
  //   if (n < retval) retval = n;
  //   // memcpy to end of queue buffer 
  //   memcpy(&buffer[chan][current_tail[chan]],
  // 	   &data[index],retval*sizeof(data[0]));
  // }
  
  // // Increment the next write pointer
  // const_cast<int64_t&>(next_tail[chan]) = increment(current_tail[chan],retval);

  // // Return succeesful pull
  // return retval;

  // // Get the current write pointer
  // const_cast<int64_t&>(current_tail[chan]) = write[chan].load();
  
  // // Increment the next write pointer
  // const_cast<int64_t&>(next_tail[chan]) = increment(current_tail[chan],1)

  // // If the next write pointer != the current read pointer
  // if (next_tail[chan] != read[chan].load()){

  //   // Add data to queue buffer 
  //   memcpy(&buffer[chan][current_tail[chan]],&data,sizeof(data));

  //   // Store the updated write pointer
  //   write[chan].store(next_tail[chan]);

  //   // Return succesful push
  //   return true;

  // }

  // // Return unsuccesful push
  // return false;  

}
  
// Push data to queue
template<typename T> void queue_buffer<T>::push(int chan, T *data, int n){   
  // The number of datagrams to push
  int retval = n;
  // The index in the array to push from
  int index = 0;
  
  // While still to push
  while (retval > 0){
    
    //    std::cout << "Trying to push " << retval << std::endl;

    // Try to push data to queue
    retval -= try_push(chan,data,retval,index);
    // Update the index in the array to push from
    index = n - retval;
   
    //    std::cout << "Pushed " << index << std::endl;
    
  }
  
  // // for packets to add
  // for (int i = 0; i < n; i++){
  //   // Try to push data to queue
  //   while( ! try_push(chan,data[i]) ){}; 
  // }

}
  
// Try to pull data from queue
template<typename T> int queue_buffer<T>::try_pull(int chan, T *&data){

  int retval = 0;

  // Get current read pointer
  current_head[chan] = read[chan].load();

  // If the current read pointer == write pointer
  if ((retval = write[chan].load() - current_head[chan]) == 0){

    // Return unsuccesful pull
    return retval;
  }

  // If buffer has wrapped arround...
  if (retval < 0){
    // Make sure the return value is positive
    retval += size;
    // Find the distance to the end of the buffer
    int end = size - current_head[chan];
    // Memcpy from end of buffer
    memcpy(data,&buffer[chan][current_head[chan]],end*sizeof(data[0]));
    // Memcpy from the start of the buffer
    memcpy(&data[end],&buffer[chan][0],(retval-end)*sizeof(data[0]));
  }
  // If copying from central region of buffer
  else{    
    // Get data from queue
    memcpy(data,&buffer[chan][current_head[chan]],retval*sizeof(data[0]));
  }

  // Store the updated read pointer
  read[chan].store(increment(chan,current_head[chan],retval));

  //  std::cout << "Pulled " << retval << std::endl;
  
  // Return succeesful pull
  return retval;

}

// Pull data from queue
//template<typename T> int queue_buffer<T>::pull(UDPsocket *udp, int chan, T *&data){
template<typename T> int queue_buffer<T>::pull(bool *timeout, int chan, T *&data){

  int retval = 0;

  // Wait until...
  while( (retval = try_pull(chan,data)) == 0 && !*timeout ) {};

  if (*timeout and (write[chan].load() - current_head[chan]) != 0){
    std::cout << "ERROR: Still data in queue!!!!" << std::endl;
    exit(0);
  }
  
  // Return pulled data
  return retval;

}
  
// Instantiate UDPsocket::packet queue class
template class queue_buffer<packet>;
