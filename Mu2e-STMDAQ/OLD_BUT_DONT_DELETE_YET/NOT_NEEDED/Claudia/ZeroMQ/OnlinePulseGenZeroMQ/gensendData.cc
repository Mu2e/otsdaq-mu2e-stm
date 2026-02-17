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

int main ()
{
   
  //Socket to talk to clients
  void *context = zmq_ctx_new ();
  void *responder = zmq_socket (context, ZMQ_REP);
  int rc = zmq_bind (responder, "tcp://*:5555");
  assert (rc == 0);




  //----------------Create the array with data: int16_t* data_element ----------------//

  //Pulse variables

  //The amplitude of the pulse is the half of this value  
  double twiceA=2370;
  //The x point in where we have the pulse (50 us)
  double xshift=50;
  //fall from baseline time
  double invtaufall=6;
  //decaytime(rise to baseline)
  double invtaudecay=0.028;
  //Amplitude of noise
  double Anoise=10;

  //Sampling time of ADC (microsec) 
  double tadc=0.0027;
  //Pulse duration (microsec)
  double pulselength=xshift+220;
  //Frequency is the inverse of the pulse duration
  double pulsefreq=1000./(xshift+220);
  //Number of ADC values in one peak ([1]+220)/0.0027: (El pulso dura xshift+220 microsec de subida aprox). Si en 0.0027 microsec hay 1 ADC value en la duracion del pulso habra n ADC values.          
  unsigned long int n=(xshift+220)/tadc;
  //Number of bytes in one peak                   
  unsigned long int nbytes=n*2;

  //Duration of the sample generated (microseconds)
  long double timesample=1000;
  //Number of peaks in the sample                     
    unsigned long int npeaks=timesample/pulselength;
  //Total number of bytes generated
  unsigned long int totalbytes=npeaks*nbytes;
  //Total number of ADC values generated
  unsigned long int totalADCvalues=npeaks*n;
 

  double* x = new double[totalADCvalues];
  int16_t* data_element = new int16_t[totalADCvalues];
  //vector<int16_t> data_element;
  int16_t ADC;
  unsigned long counter=0;

 
    //Number of peaks                                                                                                                             
    for (unsigned long j=0;j<npeaks;j++){
      //Number of bytes in one peak                                                                                                                            
      for (unsigned long i=0;i<n;i++){
	x[i]=tadc*i;
	//ADC=-(2370 / (1 + exp(-(x[i] - 50)*6)) ) * ( 1.0  - ( 1.0/ (1 + exp(-(x[i] - 50)*0.028))))+10*sin(x[i]);                                               
	ADC=-(twiceA / (1 + exp(-(x[i] - xshift)*invtaufall)) ) * ( 1.0  - ( 1.0/ (1 + exp(-(x[i] - xshift)*invtaudecay))))+Anoise*sin(x[i]);
	data_element[counter]=ADC;
	counter++;
      }
    }





    //----------------Create the array with data and headers per packet int16_t* data_header----------------// 
    
    //Server sends one packet at a time
    //Size of the packet in bytes
    //unsigned long int npacketsbytes=8240;
    unsigned long int npacketsbytes=100000;    
    //--------//Create the packet header 6 bytes----------
    //Create packetnumber 4 bytes
    int A=1;
    //Remaining 2 bytes
    int16_t B=1;

    //Packet header (lo hago de 8 bytes)
    int8_t packetheader= 1;
    //Size of header packet in bytes
    int sizepacketheader=sizeof(packetheader);
    
    //--------//Create the trigger header 32 bytes-------
    //Trigger number 4 bytes
    int C=1;
    //Trigger time 8 bytes
    int64_t D=1;
    //Trigger type 1 bit
    //Channel number 1 byte
    int8_t F=1;
    //Baseline mean & RMS 4 bytes
    int G=1;
    //Overflow counter 2 bytes
    int16_t H=1;
    //Remaining 13 bytes
    
    //Trigger header
    int32_t triggerheader=1;
    //Size of trigger header
    int sizetriggerheader= sizeof(triggerheader);

    //--------//Create the slice header 16 bytes-------  
    //Time of first  ADC in slice
    //Time of 1st adc in slice 8 bytes
    int64_t I=1;
    //Size of slice
    int64_t J=1;
    //Slice header
    int16_t sliceheader=1;
    //Size of slice header
    int sizesliceheader=sizeof(sliceheader);
   
    //N slices per packet
    unsigned long int Nslices = 8;  

    //Bytes available to store data in y=the packet=(size of packet - size packet header - size trigger header - Nslices* size slice header)
    unsigned long int Available_Bytes_packet = npacketsbytes-sizepacketheader-sizetriggerheader-Nslices*sizesliceheader;

    //Number of bytes per slice
    unsigned long int bytes_slice = Available_Bytes_packet/Nslices;


    









    //----------------Server sends the requested data----------------//   




  //Number of bytes to read is one packet
  unsigned long byteschunk=npacketsbytes; 
    /* unsigned long byteschunk=12;
  int16_t* example = new int16_t[100];
  for(int16_t i=0;i<100;i++){
    example[i]=i;
    }*/

  //Number of elements per read
  unsigned long numelements = byteschunk/2;
  //elements read
  unsigned long elementsread = 0;
  //Number of bytes sent
  unsigned long bytessent = 0;
  
  int count=0;
  std::cout<<"Press enter to start sending data when all the clients are ready"<<std::endl;
  getchar();
  
  //while we have a request
  while (1) {
   

    int16_t buffer[numelements];
    int16_t* r1 = new int16_t[numelements];

    //fread(&r1[0], sizeof(int16_t), numelements, myFile);
    //memcpy( r1, &example[elementsread], numelements*sizeof(int16_t) );
    memcpy( r1, &data_element[elementsread], numelements*sizeof(int16_t) ); 

    //for(int16_t i=0;i<numelements;i++){
    //std::cout<<r1[i]<<std::endl;
    //}
    

    //(socket,buffer,size of buffer in bytes, 0)
    zmq_recv (responder, buffer,byteschunk, 0);
    printf ("Received-Sending data\n");
    sleep (1);
      
    int nbytes= zmq_send (responder,  r1 , byteschunk, 0);
    std::cout<<count<<" Number of bytes requested from client: "<<byteschunk<<" Number of bytes sent: "<<nbytes<<std::endl;
 
    bytessent = bytessent+byteschunk;
    std::cout<<"Total bytes sent: "<<bytessent<<std::endl;
    
    elementsread=elementsread+numelements;
    std::cout<<"elementsread: "<<elementsread<<std::endl;
    //Stop sending data when there are no more bytes to send
    if(elementsread>=totalADCvalues){break;}
    count++;  

  }
  return 0;
}
