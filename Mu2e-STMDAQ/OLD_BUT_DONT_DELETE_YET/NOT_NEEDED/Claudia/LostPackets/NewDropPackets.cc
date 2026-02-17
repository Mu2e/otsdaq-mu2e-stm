#include <iostream>
#include <string>
#include <cstring>
#include <fstream>
#include <vector>
#include <utility> // std::pair
#include <stdexcept> // std::runtime_error
#include <sstream> // std::stringstream
#include <fstream>
#include <sys/stat.h>
#include <stdio.h>      /* printf, scanf, puts, NULL */
#include <stdlib.h>     /* srand, rand */
#include <time.h>       /* time */
#include <cmath>
#include <numeric>
#include <random>
#include <list>
#include <cstdio>
#include <memory>
#include <array>
#include <iomanip>
#include <map>


//#define packetSize 8000 //Bytes
//#define packetSize 8192 //Bytes
#define packetSize 4096 //Bytes 
#define fadc 320.0520833313 //MHz  


// This function drops packets generating a random number, if the number is lower than percentage_dropped/100 then skip one packet,
//if the number is bigger, copy the packet to the output data
void NewDropPackets(std::string  filename, int percentage_dropped){

  //Open input file
  struct stat st;
  stat(filename.c_str(), &st);

  //Number of elements in sample
  int n = st.st_size/2; 
  
  std::cout<<"Number of elements in original file: "<<n<<std::endl;
  
  int16_t* ADC = new int16_t[n];
  std::ifstream myFile;
  myFile.open(filename, std::ios::in | std::ios::binary);
  myFile.read( (char*)ADC, n*sizeof(ADC[0]));
  myFile.close();


  //Drop Packets in the array (missing data)
  //New array to store the data with missing packets
  int16_t* ADC_droppackets = new int16_t[n];

  //const double fadc = 320.0520833313; //MHz

  //Between 8 an 9 kb (?)
  //int packetSize = 8000; //Bytes
  int packetLen = packetSize/2;
  std::cout<<"Size of the packet in time: "<< packetLen*(1./fadc) <<" us"<<std::endl;


  //Number of packets in the sample
  int numPackets = floor(n/packetLen);
 
  int counter = 0;
  int counterdrop = 0;
  int countercopy=0;
  float prob;

  /* initialize random seed: */
  srand ( (unsigned)time(NULL));

  //Loop over packets
  for(int i = 0; i < (numPackets+1); i++){
    
    std::cout<<"----------Packet "<<i<<"--------"<<std::endl;
    if(countercopy + counterdrop + packetLen > n){packetLen = n - countercopy - counterdrop;}

    //Generate random number between 0 and 1 
    prob = (float) rand()/RAND_MAX;
    std::cout<<"prob: "<<prob<<std::endl;

    std::cout<<"counterdrop: "<<counterdrop<<std::endl;
    std::cout<<"countercopy: "<<countercopy<<std::endl;
    std::cout<<"counter: "<<counter<<std::endl;

    //skip one packet
    if(prob < ((double)percentage_dropped/100)){
      std::cout<<"Skip packet"<<std::endl;
      counter = counter + packetLen;
      counterdrop = counterdrop + packetLen;
    } 
    //copy one packet
    else{
      std::cout<<"Copy packet"<<std::endl;
      //copy 8 kb of data if the probability is less than percentage_dropped
      memcpy(&ADC_droppackets[countercopy], &ADC[counter], 2*packetLen);
      countercopy = countercopy + packetLen;
      counter = counter + packetLen;
    }
    std::cout<<"counterdrop: "<<counterdrop<<std::endl;
    std::cout<<"countercopy: "<<countercopy<<std::endl;
    std::cout<<"counter: "<<counter<<std::endl;
}

  packetLen = packetSize/2;
  std::cout<<"Total elements: "<<counterdrop+countercopy<<", should  be the same as: "<<n<<std::endl; 
  std::cout<<"Total elements skipped: "<<counterdrop<<std::endl;
  std::cout<<"Total elements copied: "<<countercopy<<std::endl;
  double total_droppedpackets = (double)(counterdrop/packetLen);
  std::cout<<"Number of packets in sample: "<<numPackets<<std::endl;
  std::cout<<"Packets dropped: "<<total_droppedpackets<<std::endl;
  std::cout<<"We are skipping: "<<100*total_droppedpackets/numPackets<<" %"<<std::endl;
  

  //Create new file with dropped packets
  std::ofstream outputfile;
  //Convert the rate and the percentage to string
  std::string percentage = std::to_string(percentage_dropped);

  /*  std::string output_filename = "/work/cgarcia/DATA/Claudia/GenData/MWDEfficiency_SimPoisson/DropPackets/ELBESimulationRate/"+filename.substr(filename.find("Gendata"), 18)+"_prob1droppedpacket_"+percentage+"percent"+".bin";
  //std::string output_filename = "/work/cgarcia/DATA/Claudia/GenData/MWDEfficiency_SimPoisson/DropPackets/Drop4kPackets_SimulationEff/"+filename.substr(filename.find("Gendata"), 19)+"_prob1droppedpacket_"+percentage+"percent"+".bin";

  outputfile.open(output_filename, std::ios::out | std::ios::binary);
  outputfile.write((char *)ADC_droppackets, countercopy*sizeof(ADC_droppackets[0]));
  
  //Elements in output file
  //Open input file
  struct stat st1;
  stat(output_filename.c_str(), &st1);
  //Number of elements in sample
  int n1 = st1.st_size/2;
  std::cout<<"Elements in output file: "<<n1<<" Bytes: "<<n1*2<<std::endl;
  */
}







//This function receives an integer numerator num and an integer denominatord drop num  packets each den packets
//if num = 1 and den = 5, the code rejects 1 packet each 5 packets. In the end we are rejecting  1/5 = 20% of the packets
void FractionalDropPackets(std::string  filename, int num, int den){

  //Open input file
  struct stat st;
  stat(filename.c_str(), &st);

  //Number of elements in sample
  int n = st.st_size/2;

  std::cout<<"Number of elements in original file: "<<n<<std::endl;

  int16_t* ADC = new int16_t[n];
  std::ifstream myFile;
  myFile.open(filename, std::ios::in | std::ios::binary);
  myFile.read( (char*)ADC, n*sizeof(ADC[0]));
  myFile.close();

  //New array to store the data with missing packets
  int16_t* ADC_droppackets = new int16_t[n];


  int packetLen = packetSize/2;
  std::cout<<"Size of the packet in time: "<< packetLen*(1./fadc) <<" us"<<std::endl;


  //Number of packets in the sample 
  int numPackets = floor(n/packetLen);

  int counter = 0;
  int counterdrop = 0;
  int countercopy=0;
 
  //Loop over packets
  for(int i = 0; i < (numPackets+1); i++){

    std::cout<<"----------Packet "<<i<<"--------"<<std::endl;
    
    if(countercopy + counterdrop + (den-num)*packetLen + (num)*packetLen > n){
      packetLen = n - countercopy - counterdrop;

      memcpy(&ADC_droppackets[countercopy], &ADC[counter], (2*packetLen));
      //Index account for packets copied
      countercopy = countercopy + packetLen;
      //Index account for packets copied and skipped
      counter = counter + packetLen ;
    }

    //Copy (den-num) packets, drop num packets
    else {
      memcpy(&ADC_droppackets[countercopy], &ADC[counter], (den-num)*(2*packetLen));

      //Index account for packets copied
      countercopy = countercopy + (den-num)*packetLen;
      //Index account for packets dropped
      counterdrop = counterdrop + (num)*packetLen;
      //Index account for packets copied and skipped
      counter = counter + (den-num)*packetLen + (num)*packetLen;
    }

    std::cout<<"counterdrop: "<<counterdrop<<std::endl;
    std::cout<<"countercopy: "<<countercopy<<std::endl;
    std::cout<<"counter: "<<counter<<std::endl;

  }

  packetLen = packetSize/2;
  std::cout<<"Total elements: "<<counterdrop+countercopy<<", should  be the same as: "<<n<<std::endl;
  std::cout<<"Total elements skipped: "<<counterdrop<<std::endl;
  std::cout<<"Total elements copied: "<<countercopy<<std::endl;
  double total_droppedpackets = (double)(counterdrop/packetLen);
  std::cout<<"Number of packets in sample: "<<numPackets<<std::endl;
  std::cout<<"Packets dropped: "<<total_droppedpackets<<std::endl;
  double percentage_double  = 100*total_droppedpackets/numPackets;
  int percentage_int = lround(percentage_double);
  std::cout<<"We are skipping: "<<percentage_double<<" %"<<std::endl;

  std::string percentage = std::to_string(percentage_int); 

  //Create new file with dropped packets
  std::ofstream outputfile;
 
  std::string output_filename = "/work/cgarcia/DATA/Claudia/GenData/MWDEfficiency_SimPoisson/DropPackets/"+filename.substr(filename.find("Genda \
ta"), 18)+"_frac1droppedpacket_"+percentage+"percent"+".bin";

  outputfile.open(output_filename, std::ios::out | std::ios::binary); 
  outputfile.write((char *)ADC_droppackets, countercopy*sizeof(ADC_droppackets[0]));
  //Open input file
  struct stat st1;
  stat(output_filename.c_str(), &st1);
  //Number of elements in sample
  int n1 = st1.st_size/2;
  std::cout<<"Elements in output file: "<<n1<<" Bytes: "<<n1*2<<std::endl; 
  

}









int main(int argc, char *argv[]){


  //argv[0]=program, argv[1]=input file with data
  std::string  input_filename = std::string(argv[1]);

  int mode = 0; //mode = 0 probability (input percentage), mode = 1 (input integer number)


  if(mode == 0){
    int percentage_dropped = atoi(argv[2]);
    //Drop packets given a random probability
    NewDropPackets(input_filename,percentage_dropped);
  }

  if(mode == 1){
    int num = atoi(argv[2]);
    int den = atoi(argv[3]);
    //Drop packets given a fractional number (num/den)
    FractionalDropPackets(input_filename, num, den);
  }

  return 0;
}
