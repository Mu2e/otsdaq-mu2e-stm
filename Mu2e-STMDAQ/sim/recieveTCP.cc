//DV: this is a very rough TCP reciever, it works but not perfectly to recieve from my sim
//Only use for testing, use George's board reader for actual TCP recieving!
#include <cstring>
#include <iostream>
#include <netinet/in.h>
#include <sys/socket.h>
#include <unistd.h>
#include <vector>

int main(){
    // creating socket
    int serverSocket = socket(AF_INET, SOCK_STREAM, 0);    // specifying the address
    int port = 5050;

    sockaddr_in serverAddress;
    serverAddress.sin_family = AF_INET;
    serverAddress.sin_port = htons(port);
    serverAddress.sin_addr.s_addr = INADDR_ANY;    

    // binding socket
    bind(serverSocket, (struct sockaddr*)&serverAddress,
         sizeof(serverAddress));    
    // listening to the assigned socket
    listen(serverSocket, 5);    
    // accepting connection request
    std::cout<<"Listening to port: "<<port<<std::endl;
    int clientSocket
        = accept(serverSocket, nullptr, nullptr);    
    // recieving data buffer
    size_t buffersize = 4096;
    std::vector<int16_t> buffer(buffersize);
    std::fill(buffer.begin(),buffer.end(),int16_t(32787));
    

    std::vector<int16_t> outvector;
    //recv(clientSocket, buffer, sizeof(buffer), 0);       
    //cout << ntohs(buffer) << endl;   
 
    //empty buffer to recieve data  
    //std::shared_ptr<DataStruct> buffer = nullptr;
    ssize_t n = 0;
    
    int16_t bytes = 1000;
    //int16_t zsbytes = 1000;
    int16_t totalBytes = 0;
    bool first = true;

    while (true){

	  sleep(1);
          std::cout<<"Total: [ ";
          for (int val: outvector){
                  std::cout<<val<<",";

          }
          std::cout<<"]"<<std::endl;


       if (totalBytes >= bytes) {
	       clientSocket = accept(serverSocket, nullptr, nullptr);
	         n = 0;
		 bytes = 1000;
      		 totalBytes = 0;
		 first = true;
		 outvector.clear();
		 std::fill(buffer.begin(),buffer.end(),int16_t(32787));
       }


        n = recv(clientSocket,buffer.data(),buffer.size() * sizeof(int16_t),0);
	if (n > 0){

	  	
	//std::cout<<"[ ";
	if (first == true) {
		//sleep(1);
		//if (buffer[1] == -32749) bytes = buffer[0];
	bytes = buffer[1]+buffer[2]*3;
	
	}
	  first =false;


	  for (int16_t val: buffer){
          //std::cout<<val;
	  //std::cout<<" , ";
	  
	  if (val > -32749 and n > 0) outvector.push_back(val);
           }
	   std::cout<<" ]"<<std::endl;
          std::cout<<"Got bytes: "<<n<<" / "<<bytes<<std::endl;
	  totalBytes += n;
	  std::cout<<"Total bytes: "<<totalBytes<<std::endl;
          
	  //sleep(1);
	  //std::cout<<"Total: [ ";
	  //for (int val: outvector){
	//	  std::cout<<val<<",";
		  
	  //}
	  //std::cout<<"]"<<std::endl;
	  //std::cout<<"Length of data vector: "<<outvector.size();

	 //zsbytes = buffer[bytes/2];
         //std::cout<<"bytes: "<<bytes<<" ZS bytes: "<<zsbytes<<std::endl;
         //bytes += zsbytes;


	}

//	  if (n >= bytes) clientSocket = accept(serverSocket, nullptr, nullptr);



	  //std::cout<<"Got bytes: "<<n<<std::endl;
    }

    //std::cout<<"Got bytes: "<<n<<std::endl;
    //for (int val: buffer){
    //std::cout<<val<<std::endl;
    //}

    
    return 0;
}
