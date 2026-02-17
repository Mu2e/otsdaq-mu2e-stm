#include <iostream>
#include <stdio.h>
#include <unistd.h>
#include <atomic>

#include "queue.hh"

// Define the buffer size - must be a multiple of 
// the number of bytes in a memory page, e.g. getpagesize()
#define BUFFER_SIZE (50*getpagesize())

// Define the number of consumer threads
#define CON_THREADS (1) 
// Define the number of consumer threads
#define PUB_THREADS (1)

// Define the total number of messages 
#define MESSAGES_TOT 250000

// Define the number of messages to be received per thread
#define MESSAGES_PER_THREAD (MESSAGES_TOT / CON_THREADS)

// Define queue class
queue cbq;

// Consumed data array
int16_t **consumedData;
// Published data array
int16_t **dataToPublish;
int arraySize = 4096;

// Consumed data counter
int con_count = 0;
// Published data counter
int pub_count = 0;

// Consumer mutex lock
pthread_mutex_t con_lock;
// Publisher mutex lock
pthread_mutex_t pub_lock;

// Consumer finished boolean
std::atomic<bool> con_finish;
// Publisher finished boolean
std::atomic<bool> pub_finish;

// The consumer loop (takes the queue as the argument)
void *consumer_loop(void *arg) {
 
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

// The publisher loop (takes the queue as the argument)
void *publisher_loop(void *arg) {
  
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

  // Initialise the queue with the given buffer size
  cbq.init(BUFFER_SIZE);

  // Initialise publisher thread
  pthread_t publishers[PUB_THREADS];

  // Initliase consumer threads
  pthread_t consumers[CON_THREADS];

  // Initialise attr with the default thread attributes
  pthread_attr_t attr;
  pthread_attr_init(&attr);

  // Loops over number of publisher threads
  for(int i = 0; i < PUB_THREADS; i++){    
    // Create publisher threads
    pthread_create(&publishers[i], &attr, &publisher_loop, NULL);
  }
    
  // Loops over number of consumer threads
  for(int i = 0; i < CON_THREADS; i++){
    // Create consuser threads
    pthread_create(&consumers[i], &attr, &consumer_loop, NULL);
  }

  // Define integer for number of sent messages
  int sent[PUB_THREADS];
  // Loops over number of publisher threads
  for(int i = 0; i < PUB_THREADS; i++){    
    // Join the publisher thread
    pthread_join(publishers[i], (void **) &sent[i]);
    // Print the number of messages received by the thread
    printf("publisher %d received %d messages\n", i, sent[i]);
  }
  // Print the number of sent messages
  printf("publisher sent %d messages\n", pub_count);
    
  // Define integer for number of recevied messages per thread
  int recd[CON_THREADS];
  // Loops over number of consumer threads
  for(int i = 0; i < CON_THREADS; i++){
    // Join that consumer thread
    pthread_join(consumers[i], (void **) &recd[i]);
    // Print the number of messages received by the thread
    printf("consumer %d received %d messages\n", i, recd[i]);
  }
  printf("consumer received %d messages\n", con_count);

  // Check published/consumed data is identical
  bool success = true;
  for (int i = 0; i < MESSAGES_TOT; i++){
    for (int j = 0; j < arraySize; j++){
      if (dataToPublish[i][j] != consumedData[i][j]) success = false;
    }
  }
  if (success) std::cout << "SUCCESS: Input/Output data identical." << std::endl;
  if (!success) std::cout << "ERROR: Input/Output data not identical!" << std::endl;
    
  // Destory the pthread
  pthread_attr_destroy(&attr);
    
  // Destroy the buffer queue
  cbq.destroy();
    
  return 0;
}
