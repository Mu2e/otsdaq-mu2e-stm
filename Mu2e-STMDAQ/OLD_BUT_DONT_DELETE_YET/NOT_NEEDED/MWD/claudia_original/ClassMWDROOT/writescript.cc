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

//#define binjobs //for 1kHz, 5kHz and 10kHz

int main(void){

  //Rate in kHz 01,05...
  std::string rate = "200";
  int rateint = stoi(rate);
  std::string ratefolder = std::to_string(rateint);

  std::string executable = "/work/cgarcia/STMDAQ-TestBeam/build/bin/MWDROOT.exe";
 
  int numberpeaks_sim= 10000;

#ifdef binjobs
  for(int jobs=0; jobs<20; jobs++){

    std::string sjobs;
    if(jobs<10){sjobs= "0"+std::to_string(jobs);}
    else{sjobs=std::to_string(jobs);}

    std::string filename = "/data1/cgarcia/GenDataehHPGeSim/662keV_0.32mV/genData662keV_"+rate+"kHz_job"+sjobs+".bin";
    std::ofstream myfile;
    myfile.open ("MWD_ML_"+rate+"kHz_job"+sjobs+".sh");
    myfile << "#!/bin/bash" <<std::endl;
    myfile << " " <<std::endl;
    int Mnum = 9;
    int Lnum = 20;

    int M[Mnum]={300,400,500,600,700,800,900,1000,2000,3000};
    int L[Lnum]={10,20,30,40,50,60,70,80,90,100,200,300,400,500,600,700,800,900,1000,2000};

    for(int i = 0; i < Mnum ;i++){
      for (int j = 0; j < Lnum ; j++){
	if(L[j] > M[i]){continue;}
	myfile << executable << " " << filename<< " " << M[i] << " " << L[j] << " " << numberpeaks_sim << " > /data1/cgarcia/DATA/Claudia/GenDataehHPGeSim/662keV_0.32mV/Resolution_Efficiency_ML/"+ratefolder+"kHz/" << "run_"+rate+"_M"<<M[i]<<"_L"<<L[j]<<"_job"<<sjobs<<".txt 2>&1 &"<<std::endl;
	myfile << "sleep 2" <<std::endl;
      }    
    }

    myfile.close();
  }//for jobs 

#else

  std::string filename = "/data1/cgarcia/GenDataehHPGeSim/662keV_0.32mV/genData662keV_"+rate+"kHz.bin";
  std::ofstream myfile;
  myfile.open ("MWD_ML_"+rate+"kHz.sh");
  myfile << "#!/bin/bash" <<std::endl;
  myfile << " " <<std::endl;
  int Mnum = 9;
  int Lnum = 20;

  int M[Mnum]={300,400,500,600,700,800,900,1000,2000};
  int L[Lnum]={10,20,30,40,50,60,70,80,90,100,200,300,400,500,600,700,800,900,1000,2000};

  for(int i = 0; i < Mnum ;i++){
    for (int j = 0; j < Lnum ; j++){
      if(L[j] > M[i]){continue;}
      myfile << executable << " " << filename<< " " << M[i] << " " << L[j] << " " << numberpeaks_sim << " > /data1/cgarcia/DATA/Claudia/GenDatae\
hHPGeSim/662keV_0.32mV/Resolution_Efficiency_ML/"+ratefolder+"kHz/" << "run_"+rate+"kHz_M"<<M[i]<<"_L"<<L[j]<<".txt 2>&1 &"<<std::endl;
      myfile << "sleep 2" <<std::endl;
    }
  }

  myfile.close();


#endif



}
