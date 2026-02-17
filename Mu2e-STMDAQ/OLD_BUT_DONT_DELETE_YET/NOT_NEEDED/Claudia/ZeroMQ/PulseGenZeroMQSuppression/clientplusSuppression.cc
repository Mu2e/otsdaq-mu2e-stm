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
//Zero Suppression Algorithm                                                                                                         
#include "ZeroSupbaseline.h"

using namespace std;


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
  //unsigned long byteschunk = 2165140;
  unsigned long byteschunk = 200000; 

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
    std::string rootname=output_filename+"/"+input_filenamestring.substr(input_filenamestring.find("pulse"), 8)+"_"+clientnumber+"_"+std::to_string(request_nbr)+".root";
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
    


    std::vector<int16_t> data;//Data vector
    data.clear();
    std::vector<int16_t> supdata;//Suppressed data vector
    supdata.clear();
    //Input for the Suppression Algorithm
    double mean=5.38095;
    double sigma=31.8822;
    ZEROSUPBASELINE fZeroSup;
    
    //Fill the received data in the vector. Data received is stored in buffer[i]
    for(unsigned long i=0;i<numelements;i++){                
      data.push_back(buffer[i]);
    }

    std::cout<<"1"<<std::endl;

    //Apply suppression algorithm to the data vector and get the suppressed data vector 
    for(unsigned long i=0;i<numelements;i++){
        supdata= fZeroSup.zerosupbaseline_alg(data, mean, sigma);
      }

    std::cout<<"2"<<std::endl;
    //Fill root files with zero sup data
    for(unsigned long i=0;i<supdata.size();i++){
      ADC=supdata.at(i);
      tree->Fill();
    }
      rootfile->Write();
      rootfile->Close();
       
  } //for request

 
  zmq_close (requester);
  zmq_ctx_destroy (context);
  return 0;
}
