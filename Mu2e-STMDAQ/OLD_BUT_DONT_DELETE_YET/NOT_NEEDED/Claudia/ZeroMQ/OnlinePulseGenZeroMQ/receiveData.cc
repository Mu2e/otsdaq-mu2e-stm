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
using namespace std;
int main (int argc, char *av[])
{

  printf ("Connecting to server…\n");
  void *context = zmq_ctx_new ();
  void *requester = zmq_socket (context, ZMQ_REQ);
  zmq_connect (requester, "tcp://localhost:5555");

  
  std::string  output_filename = std::string(av[1]);
  std::string  clientnumber = std::string(av[2]);

  int Nclient = atoi(av[2]); 

  

  //Number of bytes to read
  //unsigned long int npacketsbytes=8240;
  unsigned long int npacketsbytes=100000;
  unsigned long byteschunk = npacketsbytes; 
  //unsigned long byteschunk = 12;  
  //Number of elements per read
  unsigned long numelements = byteschunk/2;
  
  //Number of clients
  int Nclients = 1;

    
  //for (int request_nbr = 0; request_nbr <= Nreq; request_nbr++) {
  int request_nbr=0;
  while(1){
    //Create a .root file to store the data                                                                 
    std::string rootname=output_filename+"/"+std::to_string(request_nbr)+".root";
    TFile *rootfile=new TFile(rootname.c_str(),"recreate");
    TTree*tree=new TTree("treeADC","treeADC");
    int16_t ADC;
    tree->Branch("ADC",&ADC);


   std::cout<<"Request data "<<request_nbr<<std::endl;
    zmq_send (requester, "Request data", 12, 0);
  
   
     int16_t buffer[numelements];
     
    //Number of bytes to read
    int nbytes = zmq_recv (requester, buffer, byteschunk, 0);

    std::cout<<"Bytes requested: "<<byteschunk<<" Bytes received: "<<nbytes<<std::endl;
   
    

      for(unsigned long i=0;i<numelements;i++){
        ADC=buffer[i];
	std::cout<<ADC<<std::endl;
        tree->Fill();
      }
      

     
      rootfile->Write();
      rootfile->Close();
      //delete tree;
      //delete rootfile;
       
      request_nbr++;
  } //while request

 
  zmq_close (requester);
  zmq_ctx_destroy (context);
  return 0;
}
