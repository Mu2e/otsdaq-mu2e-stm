#include<iostream>
#include <string>
#include <fstream>
#include <vector>
#include <utility> // std::pair
#include <stdexcept> // std::runtime_error
#include <sstream> // std::stringstream
#include<fstream>
#include <boost/chrono.hpp>
#include<cmath>
#include <sys/stat.h>
#include <cstring>

//#include "TTree.h"
//#include "TFile.h"
using namespace std;



int main(int narg, char* arg[]){

  std::string  filename  = std::string(arg[1]);
  std::cout << "filename = " << filename << std::endl;


  boost::chrono::high_resolution_clock::time_point t1 ;
  boost::chrono::high_resolution_clock::time_point t2 ;
 



  struct stat st;
  stat(filename.c_str(), &st);
  unsigned long int  n = st.st_size/2;  // get size of file (in bytes) and set number of ADC values (each vlaue is 2 bytes)
  std::ifstream myFile;

  t1 = boost::chrono::high_resolution_clock::now();
  
  //Open the binary file with data to suppress
  int16_t* ADC = new int16_t[n];
  myFile.open(filename, std::ios::in | std::ios::binary);
  myFile.read( (char*) ADC, n*sizeof(ADC[0]));
  myFile.close();

  myFile.close();
  t2 = boost::chrono::high_resolution_clock::now();

  std::cout << "Read data ... " << std::endl;

  std::cout<<"Number of elements: "<<n<< " in time " << boost::chrono::duration_cast<boost::chrono::milliseconds>(t2-t1) << std::endl;

  t1 = boost::chrono::high_resolution_clock::now();




  //----------------------- PEAKS SUPPRESSION ALGORITHM-----------------------------------------
 
  //Calculate the gradient for ADC values separated a distance window (in time would be window * tadc) 
  unsigned long int window=100;
  int16_t* gradient = new int16_t[n-window];
  double* time = new double[n-window];
  int16_t* suppressed_data = new int16_t[n-window];
  //t adc in microsec //Use the same as in the simulation
  // ADC samling frequency (Hz)
  const double fADC = 320.0520833313;
  //Sampling time of ADC (microsec)
  const double tadc=1/(fADC);

  //Calculate the gradient
  for(unsigned long int i=0;i<(n-window);i++){

    //time in microsec
    //time[i]=i*tadc;
    gradient[i]=ADC[i+window]-ADC[i];
   
  }


  //Initial values
  bool peak=false;
  int counter=0;
  int trigger=0;
  int triggercounter=0;

  //----Variable parameters----
  //Find the trigger
  int threshold=-100;


  //store tbefore microseconds of data to the left of the trigger
  double tbefore=2;
  int prenumADCstored=int(tbefore/tadc);

  //store tafter microseconds of data to the right of the trigger
  double tafter=10;
  int postnumADCstored=int(tafter/tadc);
  //---------------------------


  for(unsigned long int i=0;i<(n-window);i++){
   
    if(gradient[i]>threshold){peak=false;
      continue;}
    //skip the index of the peak after the trigger that has already been stored
    if((gradient[i]<threshold)&&(peak==true)){continue;}

    int triggerold=trigger;
    if((gradient[i]<threshold)&&(peak==false)){
      trigger=i;
      //if triggers found are closer than the window of stored data in time, rejected trigger, continue
      //it is necesary to convert the timing window to the index window: time[i]=i*tadc
      if(trigger-triggerold<int((tafter+tbefore)/tadc)){
	//If the trigger i is closer in time than tafter+tbefore, then the trigger i+1 should be compared to the trigger i-1  
        trigger=triggerold;
	continue;}

      cout<<"Trigger number: "<<triggercounter<<": "<<trigger<<" Triggertime: "<<trigger*tadc<<endl;
      triggercounter++;

      for(int k=prenumADCstored; k>0;k--){suppressed_data[counter]=ADC[trigger-k];
	peak=true;
	counter++;}

      for(int j=0; j<postnumADCstored;j++){suppressed_data[counter]=ADC[trigger+j];
	peak=true;
	counter++;}

    }
  }





  //Write ADC peaks to the binary file  
  //char file_name[] = "suppresseddata_run109_02_gradient.bin";
  string s ="/work/cgarcia/DATA/Claudia/GenData/MWDEfficiency_SimPoisson/SuppresedFiles_tbefore2us_tafter10us/Supdata"+filename.substr(filename.find("Noise"), 15);

  int size = s.length();
  char file_name[size+1];
  // copying the contents of the
  // string to char array
  strcpy(file_name, s.c_str());
  FILE * fp = fopen(file_name, "wb");                                                        
  fwrite(&suppressed_data[0], sizeof(int16_t), counter, fp );
  fclose(fp);


  t2 = boost::chrono::high_resolution_clock::now();
  std::cout << "Zero Suppression Algorithm computing time = " << boost::chrono::duration_cast<boost::chrono::milliseconds>(t2-t1) << std::endl;

  std::cout << "Number of triggers/peaks found: " <<triggercounter<<std::endl;
  std::cout<< "Number of elements in suppressed file: " <<counter<<std::endl;


}

