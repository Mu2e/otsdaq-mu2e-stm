//Client-Request Data---Receive Data---Analyze Data
#include <iostream>
#include <string.h>
#include <stdio.h>
#include <unistd.h>
#include <vector>
#include <sys/stat.h>
//ROOT
#include "TH1F.h"
#include "TCanvas.h"
#include "TRandom.h"
#include "TF1.h"
#include "TFile.h"
#include "TTree.h"
//ZMQ
#include "zmq.h"
int main (int argc, char *av[])
{

  printf ("Connecting to server…\n");
  void *context = zmq_ctx_new ();
  void *requester = zmq_socket (context, ZMQ_REQ);
  zmq_connect (requester, "tcp://localhost:5555");

  //av[0]=program av[1]=input av[2]=output av[3]=number of client
  char *input_filename = av[1];
  std::string  input_filenamestring = std::string(av[1]);
  std::string  output_filename = std::string(av[2]);
  std::string  clientnumber = std::string(av[3]);

  int Nclient = atoi(av[3]); 

  //Size of file in bytes: lSize is the number of bytes in the .bin file
  struct stat st;
  //  stat(input_filenamestring.c_str(), &st);
  stat(input_filename, &st);
  unsigned long lSize = st.st_size;
  printf("File size in bytes: %ld\n", st.st_size);

  //Total number of elements: lSize/2

  //Number of bytes to read
  unsigned long byteschunk = 2165140;

  //Number of elements per read
  unsigned long numelements = byteschunk/2;
  
  //Number of bytes read
  unsigned long bytesread = Nclient*byteschunk;

  //Number of clients
  int Nclients = 1;

  //Number of requests (Number of request is in reality Nreq/number of clients)
  int  Nreq = int ((lSize/byteschunk)/Nclients);


  for (int request_nbr = 0; request_nbr <= Nreq; request_nbr++) {
    //Create a .root file to store the data                                                                 
    std::string rootname=output_filename+"/"+input_filenamestring.substr(input_filenamestring.find("run"), 8)+"_"+clientnumber+"_"+std::to_string(request_nbr)+".root";
    TFile *rootfile=new TFile(rootname.c_str(),"recreate");
    TTree*tree=new TTree("treeADC","treeADC");
    int16_t ADC;
    tree->Branch("ADC",&ADC);

 

   std::cout<<"Request data "<<request_nbr<<std::endl;
    zmq_send (requester, "Request data", 12, 0);
    //if number of bytes in the file is little than the number of bytes we are reading (byteschunk)                                        
    // if(lSize-bytesread<byteschunk){ byteschunk=lSize-bytesread;
    //numelements=byteschunk/2;}
    if(lSize-bytesread<byteschunk){ break;} 

     int16_t buffer[byteschunk/2];
     
    //Number of bytes to read
    int nbytes = zmq_recv (requester, buffer, byteschunk, 0);
    if(request_nbr != 0){bytesread += byteschunk;}
    std::cout<<"Bytes read: "<<bytesread<<" Bytes requested: "<<byteschunk<<" Bytes received: "<<nbytes<<std::endl;
    bytesread +=(Nclients-1)*byteschunk;
    

      for(unsigned long i=0;i<numelements;i++){
        ADC=buffer[i];
	//std::cout<<ADC<<std::endl;
        tree->Fill();
       //h1->Fill(buffer[i]);
      }
      

      //double mean = h1->GetMean();
      //std::cout<<"Received data: "<<nbytes<<" bytes"<<std::endl;
      //std::cout<<"Mean: "<<mean<<std::endl;

      //h1->Write();                                                                                     
      rootfile->Write();
      rootfile->Close();
      //delete tree;
      //delete rootfile;
      //h1->Reset();
  
  } //for request

 
  zmq_close (requester);
  zmq_ctx_destroy (context);
  return 0;
}
