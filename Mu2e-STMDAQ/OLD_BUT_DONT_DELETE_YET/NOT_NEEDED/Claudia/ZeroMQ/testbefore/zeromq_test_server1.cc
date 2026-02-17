//Server-Send data
#include <iostream>
#include <string.h>
#include <stdio.h>
#include <unistd.h>
#include <assert.h>
#include <vector>
#include <fstream>
//#include <bitset>
//ROOT
#include "TH1F.h"
#include "TCanvas.h"
#include "TRandom.h"
#include "TF1.h"
//ZMQ
#include "zmq.h"

int main (int argc, char *av[])
{
  //av[0]=program av[1]=input/Users/user/Dropbox/Thesis/Test-char* and uint16_t/Test.cc
   
  //Socket to talk to clients
  void *context = zmq_ctx_new ();
  void *responder = zmq_socket (context, ZMQ_REP);
  int rc = zmq_bind (responder, "tcp://*:5555");
  assert (rc == 0);

  char *input_filename = av[1];

  //Read the binary file
  FILE *myFile = fopen(input_filename, "rb");
  if (myFile==NULL)
    exit(1);

  //Size of file in bytes: lSize is the number of bytes in the .bin file     
  fseek (myFile , 0 , SEEK_END);
  unsigned long lSize = ftell (myFile);
  std::cout<<"File size in bytes: "<<lSize<<std::endl;
  rewind (myFile);

  //Total number of elements: lSize/2                                                                

  //Number of bytes to read                                                                          
  unsigned long byteschunk=2165140;

  //Number of elements per read
  unsigned long numelements = byteschunk/2;

  //Number of bytes sent
  unsigned long bytessent = 0;
  
  int counter=0;
  std::cout<<"Press enter to start sending data when all the clients are ready"<<std::endl;
  getchar();
  
  //while we have a request
  while (1) {
    // if(lSize-bytessent<byteschunk){ byteschunk=lSize-bytessent;
    //numelements=byteschunk/2;}

    if(lSize-bytessent<byteschunk){break;}   

    int16_t buffer[numelements];
    int16_t r1[numelements];

    fread(&r1[0], sizeof(int16_t), numelements, myFile);

  
    //(socket,buffer,size of buffer in bytes, 0)
    zmq_recv (responder, buffer,byteschunk, 0);
    printf ("Received-Sending data\n");
    sleep (1);
      
    int nbytes= zmq_send (responder,  &r1 , byteschunk, 0);
    std::cout<<counter<<" Number of bytes requested from client: "<<byteschunk<<" Number of bytes sent: "<<nbytes<<std::endl;
 
    bytessent = bytessent+byteschunk;
    std::cout<<"Total bytes sent: "<<bytessent<<std::endl;
    //Stop sending data when there are no more bytes to send
    if(bytessent==lSize){break;}
    counter++;  

  }
  return 0;
}
