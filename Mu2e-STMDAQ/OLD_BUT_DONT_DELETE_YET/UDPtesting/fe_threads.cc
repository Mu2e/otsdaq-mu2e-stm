//////////////////////////////////////////////////////////////////////////////////
/// This module contains the threads for the STM frontend (main). 
/////////////////////////////////////////////////////////////////////////////////// 

/********************************************************************/

// Loggerr
#include "STMDAQ-TestBeam/utils/Logger.hh"

// Frontend threads
#include "STMDAQ-TestBeam/UDPtesting/fe_threads.hh"

// Boolean to indicate receiving packets from firmware
bool fw = false;

// Boolean to send data to ArtDaq over socket
bool sendToArtDaq = true;

// Boolean to signal exiting frontend
bool exitFE = false;

// The difference in time between the last
// packet recevied and the UDP timeout
double timeout_time[CHNUM] = {};

// // Instance of check data class
checkData check;

// Instance of prescale data class
prescaleData prescale[CHNUM];

//Standard constru- shouldn't be used
FEthreads::FEthreads() {}

// Server function to receive packets and put into queue
void FEthreads::server(int chan, int socket, UDPsocket &udp, 
		       queue_buffer *&pushq){
  
  // Notify user
  Logger::Instance()->write(1,"In receive thread: Channel = " 
   			    + std::to_string(chan)
			    + " [" + udp.get_channel_name(chan)
			    + "], socket = " 
			    + std::to_string(socket));
  
  // Start timing clock
  auto start = chrono::steady_clock::now();
  // End timing clock
  auto end = chrono::steady_clock::now();

  // Receive data counter
  uint64_t recvCount = 0;

  // Define receive buffer as array of packets
  //int16_t *data_buffer = new int16_t [udp.RECVMMSG_NUM*MAX_PACKET_LEN] ();

  // The return value (number of datagrams received in call)
  int retval = 0;
  // The final retval
  int final_retval = 0;
  // Signal first packet received
  bool first_packet = true;

  // Loop until timeout
  while(udp.timeout_counter != udp.getTimeoutMax()){
    // Get packet and check if it times out (returns 0)
    if ((retval = udp.recv(pushq->process_buffer[chan],socket,chan)) <= 0){
      //std::cout << "UDP timed out: " << udp.timeout_counter << "\n";
      // if this is the first timeout, store end time
      if (udp.timeout_counter == 0) end = chrono::steady_clock::now();
      // Increment timeout counters
      if(!first_packet) udp.timeout_counter++;
    }
    // If not timed out...
    else{
      // Restart the timing clock when first messages is received
      if (recvCount == 0){
	start = chrono::steady_clock::now();
	// And set first packet to false
	first_packet = false;
      }
      // Push data to queue
      pushq->push(chan,pushq->process_buffer[chan],retval*MAX_PACKET_LEN);
      // Increase the received packet counter
      recvCount += retval;
      // Update the final retval
      final_retval = retval;
      // Reset timeout countet
      udp.timeout_counter = 0;
    } // End if timeout
  } // End infinite loop

  auto timeout_marker = chrono::steady_clock::now();
  auto timeout_diff = timeout_marker - end;
  timeout_time[chan] = chrono::duration <double, nano> (timeout_diff).count();

  // Tell the system the UDP software has timed out
  udp.timeout = true;
  Logger::Instance()->write(2,"UDP timeout: Channel = "
                            + std::to_string(chan)
                            + " [" + udp.get_channel_name(chan)
                            + "], socket = "
                            + std::to_string(socket));			   

  // Reconstruct the final packet number
  uint64_t lastPacketStart = (final_retval-1)*MAX_PACKET_LEN;
  uint32_t lastPacketNum = (uint16_t) pushq->process_buffer[chan][lastPacketStart+1] << 16 | (uint16_t) pushq->process_buffer[chan][lastPacketStart];
  // Print for user
  std::cout << "Channel: " << chan 
  	    << " last packet = " << lastPacketNum << std::endl;
 
  // Find time taken to receive packets
  auto diff = end - start;
  // Get time in nanoseconds
  double timeNano = chrono::duration <double, nano> (diff).count();
  // Calculate the total data size in Gbytes
  double data_size = recvCount*MAX_PACKET_SIZE*1e-9;
  // Calculate speed as Gbits/s
  double recvSpeed = data_size*8/(timeNano*1e-9);

  // Notify user
  Logger::Instance()->write(1,"Channel = " 
  			    + std::to_string(chan)
                            + " [" + udp.get_channel_name(chan)
  			    + "] received " 
  			    + std::to_string(recvCount)
  			    + " packets ["
  			    + std::to_string(data_size)
  			    + " Gbytes] at "
  			    + std::to_string(recvSpeed)
  			    + " Gbit/s");

}

// Function to check data packets in queue
void FEthreads::check_data(int chan, UDPsocket &udp, 
			   queue_buffer *pullq, 
			   queue_buffer *pushq){

  // Notify user
  Logger::Instance()->write(1,"In check data thread: Channel = " 
			    + std::to_string(chan)
                            + " [" + udp.get_channel_name(chan)
			    + "]");

  // Check data counter
  uint64_t checkCount = 0;

  // Push buffer                                                              
  //int16_t *data_buffer = new int16_t [queue_buffer::RING_BUFFER_LEN] ();

  // Define the return value of number of messages got from queue             
  uint64_t pull_len = 0; // Total length of data  
  
  // Get start time before packet receiving
  auto start = chrono::steady_clock::now();

  // Get end time after packet receiving
  auto end = chrono::steady_clock::now();

  // While the udp socket hasn't timed out...
  while(!udp.timeout){
    // Get data packet from queue
    pull_len = pullq->pull(&udp.timeout,chan,pushq->process_buffer[chan]);
    if (pull_len > 0){
      // Restart the timing clock when first messages is pulled
      if (checkCount == 0) start = chrono::steady_clock::now();
      // Check for packet loss
      check.check_packets(chan,pull_len,pushq->process_buffer[chan]);  
      // Push data to check data queue
      pushq->push(chan,pushq->process_buffer[chan],pull_len);    
      // Increment got packet counter
      checkCount += pull_len;
      // Restart end timing clock
      end = chrono::steady_clock::now();
    }
  }

  // Find time taken to receive all packets
  auto diff = end - start;
  // Get time in nanoseconds
  double timeNano = chrono::duration <double, nano> (diff).count();
  // Get check count as number of packets
  checkCount /= MAX_PACKET_LEN;
  // Calculate the total data size in Gbytes
  double data_size = checkCount*MAX_PACKET_SIZE*1e-9;

  // Delay cout
  this_thread::sleep_for(1s);
 
  // Store speed as Gbits/s
  //  double checkSpeed = data_size*8/((timeNano-timeout_time[chan])*1e-9);
  double checkSpeed = data_size*8/((timeNano)*1e-9);

  // Notify user
  Logger::Instance()->write(1,"Channel = " 
			    + std::to_string(chan)
                            + " [" + udp.get_channel_name(chan)
			    + "] checked " 
			    + std::to_string(checkCount)
			    + " packets ["
			    + std::to_string(data_size)
			    + " Gbytes] at "
			    + std::to_string(checkSpeed)
			    + " Gbit/s");

}

// Function to return data as single events 
void FEthreads::form_events(int chan, 
			    formEvents &ev,
			    UDPsocket &udp, 
			    queue_buffer *pullq, 
			    queue_buffer *pushq){

  // Notify user
  Logger::Instance()->write(1,"In form events thread: Channel = " 
			    + std::to_string(chan)
                            + " [" + udp.get_channel_name(chan)
			    + "]");

  // Push buffer
  //int16_t *data_buffer = new int16_t [queue_buffer::RING_BUFFER_LEN] ();

  // Define the return value of number of messages got from queue
  uint64_t pull_len = 0; // Total length of data  

  // Packet counter
  uint64_t packetCount = 0;

  // Event counter
  uint64_t eventCount = 0;

  // Get start time before packet receiving
  auto start = chrono::steady_clock::now();

  // Get end time after forming events
  auto end = chrono::steady_clock::now();

  // While the udp socket hasn't timed out...
  while(!udp.timeout){
    // Get data from queue
    pull_len = pullq->pull(&udp.timeout,chan,pushq->process_buffer[chan]);
    // If pulled data...
    if (pull_len > 0){
      // Restart the timing clock when first messages is pulled
      if (packetCount == 0) start = chrono::steady_clock::now();
      // Get and push packets
      eventCount += ev.get_events(chan,pull_len,pushq->process_buffer[chan],pushq);  
      // Increment event packet counter
      packetCount += pull_len;
      // Restart end timing clock
      end = chrono::steady_clock::now();
    }
  }

  // Find time taken to form all events
  auto diff = end - start;
  // Get time in nanoseconds
  double timeNano = chrono::duration <double, nano> (diff).count();
  // Get number of packets
  packetCount /= MAX_PACKET_LEN;
  // Calculate the total data size in Gbytes
  double data_size = packetCount*MAX_PACKET_SIZE*1e-9;

  // Delay cout
  this_thread::sleep_for(2s);

  // Store speed as Gbits/s
  //  double eventSpeed = data_size*8/((timeNano-timeout_time[chan])*1e-9);
  double eventSpeed = data_size*8/(timeNano*1e-9);

  // Notify user
  Logger::Instance()->write(1,"Channel = " 
			    + std::to_string(chan)
                            + " [" + udp.get_channel_name(chan)
			    + "] formed " 
			    + std::to_string(packetCount)
			    + " packets ["
			    + std::to_string(data_size)
			    + " Gbytes] into " 
			    + std::to_string(eventCount)
			    + " events at "
			    + std::to_string(eventSpeed)
			    + " Gbit/s");

}

// Function to zero suppress event data
void FEthreads::zs_data(int chan, daqZS &zs, 
			UDPsocket &udp, 
			queue_buffer *pullq, 
			queue_buffer *pushq){

  // Notify user
  Logger::Instance()->write(1,"In zero suppress thread: Channel = " 
			    + std::to_string(chan)
                            + " [" + udp.get_channel_name(chan)
			    + "]");

  // Push buffer
  //int16_t *data_buffer = new int16_t [queue_buffer::RING_BUFFER_LEN] ();

  // Define the return value of number of messages got from queue
  uint64_t pull_len = 0; // Length of data  

  // Pull data counter
  uint64_t pull_count = 0; // Summed length of data pulled

  // Zero suppressed data length
  uint64_t supLen = 0;

  // Pulse counter
  uint64_t pulseCount = 0;

  // Pulse rate
  uint64_t rate = 0;

  // Baseline count
  uint64_t baselineCount = 0;

  // Baseline mean
  int64_t baselineMean = 0;

  // Baseline rms
  int64_t baselineRMS = 0;

  // Tuple for zero suppresion call
  std::tuple<uint64_t,uint64_t,uint64_t,uint64_t,int64_t,int64_t> sup_data;
  
  // Get start time before packet receiving
  auto start = chrono::steady_clock::now();

  // Get end time after forming events
  auto end = chrono::steady_clock::now();

  // While the udp socket hasn't timed out...
  while(!udp.timeout){
    // Get data from queue
    pull_len = pullq->pull(&udp.timeout,chan,pushq->process_buffer[chan]);
    // If pulled data...
    if (pull_len > 0){
      // Restart the timing clock when first messages is pulled
      if (pull_count == 0) start = chrono::steady_clock::now();
      // Increase pull counter
      pull_count += pull_len;
      // Get and push packets
      //      sup_data = zs.do_ZS(chan,pull_len,pushq->process_buffer[chan],pushq);  
      //      zs.do_ZS(chan,pull_len,pushq->process_buffer[chan],pushq);  
      // // Increase suppressed data length
      // supLen += std::get<0>(sup_data);
      // // Increase pulse counter
      // pulseCount += std::get<1>(sup_data);
      // Restart end timing clock
      end = chrono::steady_clock::now();
    }
  }

  /*// Find time taken to form all events
  auto diff = end - start;
  // Get time in nanoseconds
  double timeNano = chrono::duration <double, nano> (diff).count();
  // Calculate the pulled data size in Gbytes
  double pull_size = pull_count*sizeof(int16_t)*1e-9;
  // Calculate the suppressed data size in Gbytes
  double data_size = supLen*sizeof(int16_t)*1e-9;*/

  // Delay cout
  this_thread::sleep_for(3s);

  /*// Store speed as Gbits/s
  double speed = pull_size*8/(timeNano*1e-9);

  // Notify user
  Logger::Instance()->write(1,"Channel = " 
			    + std::to_string(chan)
                            + " [" + udp.get_channel_name(chan)
			    + "] zero suppressed " 
			    + std::to_string(pull_size)
			    + " Gbytes into " 
			    + std::to_string(data_size)
			    + " Gbytes ["
			    + std::to_string(pulseCount)
			    + " pulses] ("
			    + std::to_string(1-data_size/pull_size)
			    + "% reduction) at "
			    + std::to_string(speed)
			    + " Gbit/s");*/

}
 

// Function to prescale data and return as single events 
void FEthreads::prescale_data(int chan, UDPsocket &udp, 
			      queue_buffer *pullq, 
			      queue_buffer *pushq){

  // Initiliase random number generator
  Random::Init();  

  // Notify user
  Logger::Instance()->write(1,"In prescale data thread: Channel = " 
			    + std::to_string(chan)
                            + " [" + udp.get_channel_name(chan)
			    + "]");

   // Push buffer
  //int16_t *data_buffer = new int16_t [queue_buffer::RING_BUFFER_LEN] ();

   // Length of data pulled from queue
   uint64_t pull_len = 0; 
   // Total length of accumulated data  
   uint64_t tot_len = 0;

   // Event counter
   uint64_t eventCount = 0;

   // Get start time 
   auto start = chrono::steady_clock::now();

   // Get end time after forming events
   auto end = chrono::steady_clock::now();

   // While the udp socket hasn't timed out...
   while(!udp.timeout){
     // Get data from queue
     pull_len = pullq->pull(&udp.timeout,chan,pushq->process_buffer[chan]);
     // If pulled data...
     if (pull_len > 0){
       // Restart the timing clock when first messages is pulled
       if (eventCount == 0) start = chrono::steady_clock::now();
       // Prescale event data
       //       eventCount += prescale[chan].prescale_data(chan,pull_len,pushq->process_buffer[chan],pushq);
       // Increment event packet counter
       tot_len += pull_len;
       // Restart end timing clock
       end = chrono::steady_clock::now();
     }
   }

   // Find time taken to form all events
   auto diff = end - start;
   // Get time in nanoseconds
   double timeNano = chrono::duration <double, nano> (diff).count();
   // Calculate the total data size in Gbytes
   double data_size = tot_len*sizeof(int16_t)*1e-9;

   // Delay cout
   this_thread::sleep_for(3s);

   // Store speed as Gbits/s
   //   double prescaleSpeed = data_size*8/((timeNano-timeout_time[chan])*1e-9);
   double prescaleSpeed = data_size*8/(timeNano*1e-9);

   // Notify user
   Logger::Instance()->write(1,"Channel = " 
   			    + std::to_string(chan)
                             + " [" + udp.get_channel_name(chan)
   			    + "] prescaled " 
   			    + std::to_string(data_size)
   			    + " Gbytes of data into " 
   			    + std::to_string(eventCount)
   			    + " events at "
   			    + std::to_string(prescaleSpeed)
   			    + " Gbit/s");

}

//  Function for final data pull from queue for distribution
void FEthreads::distribute_raw(int chan, int socket, 
 			       UDPsocket &udp, UDPsocket &sw_udp, 
			       queue_buffer *pullq,
			       uint64_t buffer_len){

   //Notify user
   Logger::Instance()->write(1,"In distribute data thread: Channel = " 
			     + std::to_string(chan)
                             + " [" + udp.get_channel_name(chan)
                             + "], socket = " 
			     + std::to_string(socket));

   // Distribute data counter 
   uint64_t distCount = 0;

   // Create data buffer for artDAQ send
   int16_t *dist_buffer = new int16_t[buffer_len] ();
   
   // Calculate the total data size in Gbytes
   double data_size = 0;
   
   // Define the return value of number of messages got from queue
   int retval = 0;
   
   // Get start time before packet receiving
   auto start = chrono::steady_clock::now();

   // Else pull data packet from queue
   int count = 0;
   int event_counter=0;

   // Initialise event buffer
   int16_t* event_buffer = new int16_t[UDPsocket::MAX_UDP_LEN] ();

   int tot_len = 0;
   
   // While the udp socket hasn't timed out...
   while(!udp.timeout){
     // Pull from queue into buffer
     retval = pullq->pull(&udp.timeout,chan,dist_buffer);
     while(count < retval){
       // Get start of data
       int hdr_start_loc = count;
       // Get event length from header
       int16_t event_len = dist_buffer[hdr_start_loc+fw_tHdr::EvLen];
       // Get size to copy 
       uint32_t size_to_copy = (event_len + fw_tHdr_Len)*sizeof(int16_t);
       // Initialise event buffer
       memcpy(&event_buffer[0], &dist_buffer[hdr_start_loc], size_to_copy);
       //Send data to Artdaq socket
       if(sendToArtDaq) sw_udp.sendOne(event_buffer, size_to_copy, socket);
       // Increment counters
       count += event_len+fw_tHdr_Len;
       tot_len+=count;
       event_counter++;
     }
     // Reset counter for next pull
     count = 0;
     //Restart the timing clock when first messages is pulled
     if (distCount == 0) start = chrono::steady_clock::now();
     //Increment distribute event counter
     distCount = event_counter;
   }
   
   // Get end time after event distribution
   auto end = chrono::steady_clock::now();
   // Find time taken to distribute all events
   auto diff = end - start;
   // Get time in nanoseconds
   double timeNano = chrono::duration <double, nano> (diff).count();
   // Calculate the total data size in Gbytes
   //double data_size = tot_len*sizeof(int16_t)*1e-9;
   
   // Delay cout
   this_thread::sleep_for(3s);
   
   // Notify user
   Logger::Instance()->write(1,"Channel = " 
			     + std::to_string(chan)
			     + " [" + udp.get_channel_name(chan)
			     + "] distributed " 
			     + std::to_string(distCount)
			     + " events");

   delete[] dist_buffer;
     
}

// // Function for final data pull from queue for distribution
// void FEthreads::distribute_data(int chan, int socket, 
// 				UDPsocket &udp, UDPsocket &sw_udp, 
// 				queue_buffer<dataVars::off_event> &pullq,
// 				shm_queue<dataVars::on_event> *shmq){
  

//   // Notify user
//   Logger::Instance()->write(1,"In distribute data thread: Channel = " 
// 			    + std::to_string(chan)
//                             + " [" + udp.get_channel_name(chan)
//                             + "], socket = " 
// 			    + std::to_string(socket));

//   // Distribute data counter 
//   uint64_t distCount = 0;

//   // Calculate the total data size in Gbytes
//   double data_size = 0;

//   // Define distribution buffer as array of events
//   dataVars::off_event *dist_buffer = new dataVars::off_event [udp.RECVMMSG_NUM]; 

//   // Define the return value of number of messages got from queue
//   int retval = 0;

//   // Get start time before packet receiving
//   auto start = chrono::steady_clock::now();

//   // Infinite loop
//   while(1){
//     // End condition
//     if (udp.timeout){
//       // Exit infinite loop
//       break;
//     }    
//     // Else pul data packet from queue
//     retval = pullq->pull(&udp.timeout,chan,dist_buffer);
//     // Copy event data to event array to check output
//     //    for (int i = 0; i < retval; i++){
//     //   data_size += dist_buffer[i].size*1e-9;
//     //   //      shmq->push(dist_buffer[i]);
//     //    }
//     // Send data to Artdaq socket
//     if (sendToArtDaq) sw_udp.send(dist_buffer,retval,socket);
//     // Restart the timing clock when first messages is pulled
//     if (distCount == 0) start = chrono::steady_clock::now();
//     // Increment distribue event counter
//     distCount += retval;
//   }
  
//   // Get end time after event distribution
//   auto end = chrono::steady_clock::now();
//   // Find time taken to distribute all events
//   auto diff = end - start;
//   // Get time in nanoseconds
//   double timeNano = chrono::duration <double, nano> (diff).count();

//   // Delay cout
//   this_thread::sleep_for(4s);

//   // Notify user
//   Logger::Instance()->write(1,"Channel = " 
// 			    + std::to_string(chan)
// 			    + " [" + udp.get_channel_name(chan)
// 			    + "] distributed " 
// 			    + std::to_string(distCount)
// 			    + " events");

// }


 // Function to push data to queue to write
void FEthreads::write_data(int chan, uint64_t buffer_len, 
			   UDPsocket &udp, 
			   queue_buffer *pullq,
			   queue_write *pushq){  
  
   // Notify user
   Logger::Instance()->write(1,"In write data thread: Channel = " 
 			    + std::to_string(chan)
                             + " [" + udp.get_channel_name(chan)
                             + "]"); 
  
   // Write counter 
   uint64_t writeCount = 0;

   // Calculate the total data size in Gbytes
   double data_size = 0;

   // Define write buffer as array of events
   int16_t *write_buffer = new int16_t[buffer_len]; 

   // Define the return value of number of messages got from queue
   int retval = 0;
  
   // Get start time before packet receiving
   auto start = chrono::steady_clock::now();
  
   // Infinite loop
   while(1){
     // End condition
     if (udp.timeout){
       // Exit infinite loop
       break;
     }    
     // Else pull data packet from queue
     retval = pullq->pull(&udp.timeout,chan,write_buffer);
     // Push to write queue
     pushq->push(chan,write_buffer,retval);
     // Restart the timing clock when first messages is pulled
     if (writeCount == 0) start = chrono::steady_clock::now();
     // Increment write event counter
     writeCount += retval;
   }
  
   // Get end time after event writeribution
   auto end = chrono::steady_clock::now();
   // Find time taken to write all events
   auto diff = end - start;
   // Get time in nanoseconds
   double timeNano = chrono::duration <double, nano> (diff).count();

   // Delay cout
   this_thread::sleep_for(3s);

   // Notify user
   Logger::Instance()->write(1,"Channel = " 
 			    + std::to_string(chan)
 			    + " [" + udp.get_channel_name(chan)
 			    + "] wrote " 
 			    + std::to_string(writeCount)
 			    + " events");

 }
