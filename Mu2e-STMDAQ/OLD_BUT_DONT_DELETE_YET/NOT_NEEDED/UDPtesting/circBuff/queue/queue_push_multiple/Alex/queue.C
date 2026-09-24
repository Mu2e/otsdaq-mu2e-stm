#include <iostream>                                                                                   
#include <thread>                                                                                      
#include <atomic>  
#include <cinttypes>
#include <cstring>
#include <thread>
#include <mutex>
#include "queue.h"

using namespace std;

// Constructor 
template<typename T> queue_buffer<T>::queue_buffer() {
  
  // For each channel, set read and write pointers to zero
  for (int chan = 0; chan < CHNUM; chan++){
    write[chan].store(0);
    read[chan].store(0);
    numInBuffer[chan].store(0);
    buffer[chan] = new T [buffer_size*packet_len];      
  }
}

// // Try to push data to queue
// template<typename T> int queue_buffer<T>::try_push(int chan, T *data, 
// 						   int n, int index){  
  
//   // Initialise the return value
//   int retval=0;

//   // Load the current tail
//   current_tail[chan] = write[chan].load();

//   // Load the amount in buffer
//   writeNum[chan] = numInBuffer[chan].load();

//   // Get the available space in the buffer
//   int space = buffer_size - writeNum[chan];

//   // If space is zero
//   if(space == 0){
//     // Return zero
//     return retval;
//   }
//   // Else if n < space
//   else if(n < space){
//     // Push all n
//     retval = n;
//   }
//   // Else if n > space
//   else if(n > space){
//     // Push only avaliable space
//     retval = space;
//   }

//   // If the increase wraps the buffer around
//   if(increment(current_tail[chan],retval) < current_tail[chan]){

//     // Get the distance to the end of the buffer
//     int end = buffer_size - current_tail[chan];
    
//     // Memcpy to the end of buffer
//     memcpy(&buffer[chan][current_tail[chan]],
// 	   &data[index],end*sizeof(data[0]));

//     // Memcpy the rest to the start of the buffer
//     memcpy(&buffer[chan][0],&data[index+end],(retval-end)*sizeof(data[0]));

//   }
//   // Else if if it doesn't wrap around
//   else{
//     // Memcpy retval datagrams
//     memcpy(&buffer[chan][current_tail[chan]],
// 	   &data[index],(retval*sizeof(data[0])));
//   }
  
//   // Store the updated number in buffer
//   numInBuffer[chan].fetch_add(retval);

//   // Update the write pointer
//   write[chan].store(increment(current_tail[chan],retval));

//   // Return retval
//   return retval;
// }
  
// // Push data to queue
// template<typename T> void queue_buffer<T>::push(int chan, T *data, int n){   
  
//   // We want to push n datagrams
//   int retval = n;
  
//   // While datagrams to push > 0
//   while(retval > 0) {
//     // Find the index in the array to push from
//     int index = n - retval;    
//     // Push, return the amount pushed and reclculate 
//     // how many left to push
//     retval -= try_push(chan,data,retval,index);
//   };

// }
  
// // Try to pull data from queue
// template<typename T> int queue_buffer<T>::try_pull(int chan, T *&data){

//   // Initialise retval
//   int retval = 0;
  
//   // Get current read pointer
//   current_head[chan] = read[chan].load();

//   // Get number to read in buffer
//   readNum[chan] = numInBuffer[chan].load();
  
//   // If nothing to read in the buffer
//   if ((retval = readNum[chan]) == 0){
//     // Return unsuccesful pull
//     return retval;
//   }
  
//   // Make sure we don't try and pull more than the push_max
//   if(retval > push_max) retval = push_max;


//   // If the increase wraps the buffer around
//   if(increment(current_head[chan],retval) < current_head[chan]){ 

//     // Get the distance to the end of the buffer
//     int end = buffer_size - current_head[chan];

//     // Memcpy from end of buffer
//     memcpy(data,&buffer[chan][current_head[chan]],end*sizeof(data[0]));
//     // Memcpy from the start of the buffer
//     memcpy(&data[end],&buffer[chan][0],(retval-end)*sizeof(data[0]));
    
//   }
//   // Else if if it doesn't wrap around
//   else{
//     // Memcpy retval datagrams
//     memcpy(data,&buffer[chan][current_head[chan]],retval*sizeof(data[0]));
//   }
  
//   // Store the updated number in buffer
//   numInBuffer[chan].fetch_sub(retval);

//   // Store the updated read pointer
//   read[chan].store(increment(current_head[chan],retval));
//   // Return successful pull
//   return retval;
// }

// // Pull data from queue
// template<typename T> int queue_buffer<T>::pull(int chan, T *&data){
//   int retval = 0;
//   // Wait until...
//   while( (retval = try_pull(chan,data)) == 0) {};
//   // Return pulled data
//   return retval;
// }
  
// template class queue_buffer<int>;

queue_buffer<int> order_up;

void pushFn(int chan, int *data, int n){
  // int retVal = 0;
  // int* buffer = new int[push_max]();  
  // while(retVal < (m_num)){
  //   int n = 1 + (rand() % push_max);
  //   if(retVal+n > m_num) n = m_num - retVal;
  //   memcpy(buffer,&data[retVal],n*sizeof(buffer[0]));
  //   order_up.push(chan,buffer,n);
  //   retVal += n;
  // }
}

// void pullFn(int chan, int *data){
//   int retVal = 0;
//   int pullCnt = 0;
//   int* buffer = new int[push_max]();
//   while(pullCnt < (m_num)){
//     retVal = order_up.pull(chan,buffer);
//     memcpy(&data[pullCnt],buffer,retVal*sizeof(buffer[0]));
//     pullCnt += retVal;
//   }
// }

int main(){

  //  queue_buffer<int> order_up;

  int chan = 1;  
  int* pushData[m_num];
  int* pullData[m_num];
  for(int i = 0; i < m_num; i++){
    pushData[i] = new int[packet_len]();
    pullData[i] = new int[packet_len]();
  }
  int n = m_num;
  srand((unsigned) time(NULL));

  for(int i = 0; i < m_num; i++){
    for(int j = 0; j < packet_len; j++){
      int random = 1 + (rand() % m_num);
      pushData[i][j] = random;
    }
  }

  std::thread *pushT = new std::thread(pushFn, chan, pushData, push_max);
  //  std::thread *pullT = new std::thread(pullFn, chan, pullData);
  pushT->join();
  exit(0);
  //  pullT->join();
  int x=0;
  bool success = true;
  for(int i=0; i < m_num; i++){
    if(pushData[i] != pullData[i]) success = false;
  }
  if(success){
    cout << "\n Successfully pushed all events \n" << endl;
  }
  else{
    cout << "\n Job failed \n" << endl;
  }

  return 1;
}
