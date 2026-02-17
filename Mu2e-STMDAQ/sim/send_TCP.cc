// Send TCP header
#include "Mu2e-STMDAQ/simdaq/send_TCP.hh"

#include <cstdlib>
#include <netinet/in.h>
#include <sys/socket.h>
#include <unistd.h>
#include <arpa/inet.h>

// Include config
#include "Mu2e-STMDAQ/config/config.hh"
// Include gen data code
#include "Mu2e-STMDAQ/simdaq/gen_eventData.hh"





//setup things for TCP/IP
const char* server_ip = "127.0.0.1";
int port = 5050;




// Constructor
SendTCP::SendTCP(const Config& cfg_,
		       const std::shared_ptr<STMdata>& stm_,
		       const int CHAN_,
	               const int nevents_,
		       bool sendTCP_) :
  cfg(cfg_), stm(stm_), CHAN(CHAN_), nevents(nevents_), sendTCP(sendTCP_) {
    
}





// Add headers to data
void SendTCP::send_TCP(std::shared_ptr<std::vector<int16_t>>& buffer){

  //std::cout << "In send_TCP"  << std::endl;


  // Reference event_data
  auto& event = *buffer;


//  for (int i = 0; i < event.size(); i++){
            
//   std::cout<<"(SendTCP) "<< i << " : "<<event[i]<<std::endl;
     //std::cout<<i<<std::endl;;
   
//   }



  if (sendTCP == 0)  std::cout<<"TCP SEND STATUS: OFF ";

  else if (sendTCP == 1) {
     std::cout<<"TCP SEND STATUS: ON ";



    

//  int nevents = 10;
    //TCP socket
    int sockfd = socket(AF_INET, SOCK_STREAM, 0);
    //if can't set up, stop
    if (sockfd < 0) {std::cout<<"Error, could not set up TCP sender"<<std::endl;}

    //connection info
    sockaddr_in server_addr{};
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(port);
    inet_pton(AF_INET, server_ip, &server_addr.sin_addr);

    //connect to reciever, if failed, stop


    if (connect(sockfd, (sockaddr*)&server_addr, sizeof(server_addr)) < 0) {
            std::cout<<"Error, could not connect to reciever"<<std::endl;;
    }

    else std::cout << "Connected to receiver\n";

      //send the buffer 
      size_t bytes_sent = 0;
      //const char* data = reinterpret_cast<const char*>(&buffer->raw);
      size_t total_bytes = event.size() * sizeof(int16_t);
      //std::cout<<"Buffer size: "<<total_bytes<<std::endl;

      while (bytes_sent < total_bytes) {


      for (int i = 0; i < event.size(); i++){
        //std::cout << i << " " << buffer->raw[i] << std::endl;
        // SEND BUFFER(S) VIA TCP/IP
        //const char* testmessage = " hello world ";
        int16_t val = event[i];
        //while (i <= e*buffer->raw_len/nevents + 6) 

        ssize_t n = send(sockfd, &val,sizeof(int16_t) , 0);
        bytes_sent += n;
        //file.write(reinterpret_cast<const char*>(&buffer->raw[i]), sizeof(int16_t));           
      }

   

    } //bytes sent loop


  } //if loop

  else std::cout<<"Error, invalid TCP send bool, please change to 0 (no send) or 1 (send)"<<std::endl;

  return;
  
}


