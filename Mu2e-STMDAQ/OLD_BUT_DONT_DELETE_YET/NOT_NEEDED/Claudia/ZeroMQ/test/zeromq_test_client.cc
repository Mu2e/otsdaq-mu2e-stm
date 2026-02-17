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
//MWD+Pulse Finding
#include "MWD.h"
#include "PulseFinding.h"

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

  TH1F*h1 = new TH1F("TH1","", 1000, -3000, 0);

  //Size of file in bytes: lSize is the number of bytes in the .bin file
  struct stat st;
  stat(input_filename, &st);
  unsigned long lSize = st.st_size;
  printf("File size in bytes: %ld\n", st.st_size);

  //Total number of elements: lSize/2

  //Number of bytes to read
  unsigned long byteschunk = 2165140;
  //unsigned long byteschunk = 4000000; 

  //Number of elements per read
  unsigned long numelements = byteschunk/2;
  
  //Number of bytes read
  unsigned long bytesread = 0;

  //Number of requests
  int  Nreq = int (lSize/byteschunk);




  for (int request_nbr = 0; request_nbr <= Nreq; request_nbr++) {

    //Create a .root file to store the data: run000x_client_numberrequest_energies.root
    std::string rootname=output_filename+"/"+input_filenamestring.substr(input_filenamestring.find("run"), 8)+"_"+clientnumber+"_"+std::to_string(request_nbr)+"energies.root";
    TFile *rootfile=new TFile(rootname.c_str(),"recreate");

    TTree*tree=new TTree("treeADC","treeADC");
    double ADCpeaks;
    int16_t ADCval;
    tree->Branch("ADC",&ADCval);
    tree->Branch("ADCpeaks",&ADCpeaks);
 

   std::cout<<"Request data "<<request_nbr<<std::endl;
    zmq_send (requester, "Request data", 15, 0);
    
    //if number of bytes in the file is little than the number of bytes we are reading (byteschunk)
     if(lSize-bytesread<byteschunk){ byteschunk=lSize-bytesread;
       numelements=byteschunk/2;} 

     int16_t buffer[byteschunk/2];
     
    //Number of bytes to read
    int nbytes = zmq_recv (requester, buffer, byteschunk, 0);
    bytesread = bytesread+byteschunk;

    //Store time and ADC values read
    std::vector<int16_t> ADC;
    std::vector<double> time;
    time.clear();
    double t=0;
    //double T0=2.7;                                                                                                                            
    double T0=3.125;

      for(unsigned long i=0;i<numelements;i++){
        ADCval=buffer[i];
        tree->Fill();
        ADC.push_back(ADCval);
        time.push_back(t);
        t+=T0;
      }
      
      std::cout<<"Received data: "<<nbytes<<" bytes"<<std::endl;
      
      /*      //Moving Window
      MWD fMWD;
      double tau=65544.1;
      double M=8000;
      double L=1000;

      std::vector<double> l = fMWD.mwd_alg(ADC, tau, T0, M, L);

      //Pulse Finding
      PulseFinding fPulse;

      std::vector<double> vpeak= fPulse.peaks(ADC.size(),l, time);
      //Filling the .root file
      for (long unsigned int i=0; i<vpeak.size();i++){
        ADCpeaks=vpeak.at(i);
        tree->Fill();
        h1->Fill(ADCpeaks); 
       }

       h1->Write(); */                                                                                    
      rootfile->Write();
      rootfile->Close();
      /*h1->Reset();*/
  
  } //for request

 
  zmq_close (requester);
  zmq_ctx_destroy (context);
  return 0;
}
