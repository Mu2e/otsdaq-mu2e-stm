#include <iostream>
#include <unistd.h>

#include <fcntl.h>
#include <cstring>

#include "queue.hh"
#include "queue_bf.hh"

#include <algorithm>
#include <math.h>

// Queue class
queue_buffer cbq;
queue_bf_buffer bfq;

// Packet size
static const int packetSize = 8198;
//static const int packetSize = 4;
static const int packetLen = packetSize/2;
// Number of packets
static const int packetNum = 3e5;
// Packet array
int16_t** packets = new int16_t* [packetNum];
int16_t** packets2 = new int16_t* [packetNum];

// Signal exitFE to stop the data file write
bool exitFE = false;

// Put data into queue
void put(){
  for(int i = 0; i < packetNum; i++){
    //    cbq.push(0,packets[i]);
    bfq.push(0,packets[i]);
  }
  std::cout << "All packets pushed to write queue" << std::endl;
}

// Get data from queue
void get(){
  for(int i = 0; i < packetNum; i++){
    //    memcpy(packets2[i],cbq.pull(0),packetSize);
  }
}

// Write data to file
void write_data(){

  // Infinte loop
  while(1){
    // Try to write file
    bfq.try_write_file(0);
    // If exit has been signalled
    if (exitFE){
      // Write the remaining data
      bfq.write_end(0);
      // Break infinite loop
      break;
    }
  } 
}

// main function
int main (int argc, char** argv){

  // Loop over packets
  for(int i = 0; i < packetNum; i++){
    packets[i] = new int16_t [packetLen] ();
    packets2[i] = new int16_t [packetLen] ();
    // Fill packets with distinct data
    for(int j = 0; j < packetLen; j++) packets[i][j] = i + j;
  }  
  
  // Define threads
  std::thread *put_thread;
  std::thread *get_thread;
  std::thread *write_thread;

  // Start threads
  put_thread = new std::thread (&put);
  get_thread = new std::thread (&get);
  write_thread = new std::thread (&write_data);

  // Join threads
  put_thread->join();
  get_thread->join();
  
  // Counter to stop write thread
  int count = 0;
  // Wait 5 seconds...
  while(count < 5){
    std::cout << "Stopping write in " << 5 - count << std::endl;
    sleep(1);
    count++;
  }
  // Signal exit
  exitFE = true;
  // Join write thread
  write_thread->join();

  // Get the number of packets per binary file
  int packets_per_file = bfq.get_bf_packet_num();
  // Get the number of expected files
  int file_num = ceil(double(packetNum)/double(packets_per_file));
  if (packets_per_file > packetNum){
    packets_per_file = packetNum;
    file_num = 1;
  }

  // std::cout << "Packets per file = " << packets_per_file <<  std::endl;
  // std::cout << "Number of expected files = " << file_num <<  std::endl;
  // std::cout << "Number of expected packets = " << file_num*packets_per_file <<  std::endl;
  // if (file_num*packets_per_file != packetNum){
  //   std::cout << "ERROR; number of expected packets should be " << packetNum << std::endl;
  //   //    exit(0);
  // }
  
  // Initialise a packet counter
  int p = 0;
  // Loop over the number of files
  for (int i = 0; i < file_num; i++){
    // Get file name
    std::string filename = "channel_" + std::to_string(0)
      + "_subrun" + std::to_string(i) + ".bin";
    // Open binary file
    std::ifstream bf(filename, std::ios::out | std::ios::binary);
    // Check file exists
    if(!bf) {
      std::cout << "No file: " << filename << std::endl;
    }
    // Loop over number of packets per file
    for (int j = 0; j < packets_per_file; j++){
      // Read binary file a packet at a time
      bf.read((char *) packets2[p], packetSize);      
      // Increment packet counter
      p++;      
      if (p == packetNum) break;
    }
    // Close the binary file
    bf.close();
  }
     
  // Check input/output data
  for(int i = 0; i < packetNum; i++){
    for(int j = 0; j < packetLen; j++){
      if (packets[i][j] != packets2[i][j]){
	std::cout << i << "   " << j << "   " << packets[i][j] << "   " <<  packets2[i][j] << std::endl;
      	std::cout << "ERROR" << std::endl;
	exit(0);
      }
    }
  }
  
  std::cout << "SUCCESS" << std::endl;

  return 0;
}
