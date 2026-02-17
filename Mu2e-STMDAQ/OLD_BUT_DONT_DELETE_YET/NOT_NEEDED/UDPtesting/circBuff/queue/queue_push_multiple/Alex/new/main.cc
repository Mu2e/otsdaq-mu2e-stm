// Queue buffer header
#include "queue.hh"

queue_buffer<int16_t> queue;

static const int packet_num = 1e5;
static const int packet_size = 8198;
static const int packet_len = packet_size/2;
static const int push_max = 65536;

static const int chnum = 1;

// Push to queue thread function
void push(int chan, int16_t **data){

  // The return value
  int retval = 0;    

  // Push buffer
  int16_t **buffer;
  buffer = new int16_t*[push_max];
  for (int i = 0; i < push_max; i++) buffer[i] = new int16_t [packet_len];
  
  // While retval < than the number of packets
  while(retval < (packet_num)){                    
    // Get a random number of packets to push
    int push_num = 1 + (rand() % push_max);         
    // If the retval + the number to push is greater than
    // the maximum number of packets, only push remainder
    if ((retval + push_num) > packet_num) push_num = packet_num - retval;
    // Memcpy the data to the buffer
    memcpy(buffer,&data[retval],push_num*sizeof(buffer[0]));                                           
    // Push the data to the buffer
    queue.push(chan,*buffer,push_num);  
    // Update the retval
    retval += push_num;                                                             
  }      
  
}
int main(){


  int16_t test[packet_num][packet_len] = {{}};
  std::cout << "Test complete" << std::endl;
  exit(0);
  
  int16_t *push_packets[packet_num];

  std::cout << "Generating random data..." << std::endl;
  for(int i = 0; i < packet_num; i++){
    push_packets[i] = new int16_t[packet_len];
    for(int j = 0; j < packet_len; j++){
      int16_t random = 1 + (rand() % packet_len);
      push_packets[i][j] = random;
    }
  }

  std::thread *push_thread = new std::thread(push,0,push_packets);

  push_thread->join();

  return 1;

}
