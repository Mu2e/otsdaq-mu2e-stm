#include <iostream>
#include <stdio.h>
#include <unistd.h>

#include "queue.hh"

// Instance of queue
queue_buffer<int16_t> dstq;


// Pull thread
void pull_thread() {
 
  // Define counter for number of messages
  size_t count = 0;

  // Loop until the finish is signalled
  while(!con_finish){

    // If the consumer has already finished
    if (con_finish) {
      break;
    }

    // Get the message from the queue
    cbq.get(consumedData[con_count],arraySize*sizeof(int16_t));

    // Increment thread counter
    count++;
    // Increment consumer counter
    con_count++;

    // If the consumer count equals the data total, signal finsheed
    if (con_count == MESSAGES_TOT) con_finish = true;    

  }

  // Return the counter
  return (void *) count;

}

// Push thread
void push_thread() {
  
  // Define counter for number of messages
  size_t count = 0;

  // Loop until the finish is signalled
  while(!pub_finish){

    // If the publisher has already finished
    if (pub_finish) {
      // Unlock the consumer mutex
      break;
    }
    
    // Put the message in the queue (the loop counter)
    cbq.put(dataToPublish[pub_count], arraySize*sizeof(int16_t));

    // Increment thread counter
    count++;
    // Increment publisher counter
    pub_count++;

    // If the publisher count equals the data total, signal finsheed
    if (pub_count == MESSAGES_TOT) pub_finish = true;    

  }

  // Return the total number of messages published
  return (void *) count;

}

// Main function
int main(int argc, char *argv[]){

  // Initialise and formulate send/receive arrays
  dataToPublish = new int16_t* [MESSAGES_TOT];
  consumedData = new int16_t* [MESSAGES_TOT];
  // Set data to be published as incrementing counter
  for (int i = 0; i < MESSAGES_TOT; i++){
    dataToPublish[i] = new int16_t [arraySize] ();
    consumedData[i] = new int16_t [arraySize] ();
    for (int j = 0; j < arraySize; j++){
      dataToPublish[i][j] = i-10*j;
    }
  }

  // Initialise push thread
  std::thread *push_thread = new thread;

  // Initialise pull thread
  std::thread *pull_thread = new thread;

  push_thread->join();
  pull_thread->join();

  // Check published/consumed data is identical
  bool success = true;
  for (int i = 0; i < MESSAGES_TOT; i++){
    for (int j = 0; j < arraySize; j++){
      if (dataToPublish[i][j] != consumedData[i][j]) success = false;
    }
  }
  if (success) std::cout << "SUCCESS: Input/Output data identical." << std::endl;
  if (!success) std::cout << "ERROR: Input/Output data not identical!" << std::endl;
    
  return 0;
}
