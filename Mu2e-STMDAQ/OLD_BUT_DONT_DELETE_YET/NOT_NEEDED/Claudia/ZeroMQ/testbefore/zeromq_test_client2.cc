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

int main (void)
{
  printf ("Connecting to server…\n");
  void *context = zmq_ctx_new ();
  void *requester = zmq_socket (context, ZMQ_REQ);
  zmq_connect (requester, "tcp://localhost:5555");

  //TH1F*h1 = new TH1F("TH1","", 1000, -10, 10);

  //Size of file in bytes: lSize is the number of bytes in the .bin file
  char file_name[] = "run00083.bin";
  struct stat st;
  stat(file_name, &st);
  unsigned long lSize = st.st_size;
  printf("File size in bytes: %ld\n", st.st_size);

  //Total number of elements: lSize/2

  //Number of bytes to read
  unsigned long byteschunk = 2165140;

  //Number of elements per read
  unsigned long numelements = byteschunk/2;
  
  //Number of bytes read
  unsigned long bytesread = 0;

  //Number of requests
  int  Nreq = int (lSize/byteschunk);


  for (int request_nbr = 0; request_nbr <= Nreq; request_nbr++) { 
    //Create a .root file to store the data                                                         

    std::string rootname= "run00083_2_"+std::to_string(request_nbr)+".root";
    TFile *rootfile=new TFile(rootname.c_str(),"recreate");
    TTree*tree=new TTree("treeADC","treeADC");
    int16_t ADC;
    tree->Branch("ADC",&ADC);

   std::cout<<"Request data "<<request_nbr<<std::endl;
    zmq_send (requester, "Request data", 15, 0);
    
    //if number of bytes in the file is little than the number of bytes we are reading (byteschunk)
     if(lSize-bytesread<byteschunk){ byteschunk=lSize-bytesread;
       numelements=byteschunk/2;} 

     int16_t buffer[byteschunk/2];
     
    //Number of bytes to read
    int nbytes = zmq_recv (requester, buffer, byteschunk, 0);
    bytesread = bytesread+byteschunk;

      for(unsigned long i=0;i<numelements;i++){
	//std::cout<<i<<" "<<buffer[i]<<std::endl;
        ADC=buffer[i];
        tree->Fill();
       //h1->Fill(buffer[i]);
      }
      //double mean = h1->GetMean();
      std::cout<<"Received data: "<<nbytes<<" bytes"<<std::endl;
      //std::cout<<"Mean: "<<mean<<std::endl;

      //h1->Write();
      rootfile->Write();
      rootfile->Close();
      //h1->Reset();
  
  } //for request

 

  zmq_close (requester);
  zmq_ctx_destroy (context);
  return 0;
}
