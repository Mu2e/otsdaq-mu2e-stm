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


int main(void){

  std::string rate = "20";
  int numberpeaks_sim =10000;

  std::string executable = "/work/cgarcia/STMDAQ-TestBeam/build/bin/MWDROOT.exe";

  std::string filename = "/data1/cgarcia/GenDataehHPGeSim/662keV_0.32mV/genData662keV_20kHz.bin";
  std::ofstream myfile;

  int taunum = 36;

  int tau[taunum]={20000, 30000, 40000, 45000, 45500, 46000, 46500, 47000, 47500, 48000, 48500, 49000, 49500, 50000, 50500, 51000, 51500, 52000, 52500, 53000, 53500, 54000, 54500, 55000, 55500, 56000, 56500, 57000, 57500, 58000, 58500, 59000, 59500, 60000, 70000, 80000};

  myfile.open ("MWD_ML_"+rate+"kHz_tau.sh");
  myfile << "#!/bin/bash" <<std::endl;
  myfile << " " <<std::endl;

  for(int i = 0; i < taunum ;i++){
    myfile << executable << " " << filename<< " 400 200 " << numberpeaks_sim << " " << tau[i] <<" > /data1/cgarcia/DATA/Claudia/GenDataehHPGeSim/662keV_0.32mV/M400L200Tau/"+rate+"kHz/run_20kHz_M400_L200_0.32mV_tau"<< tau[i]  <<"ns.txt 2>&1 &"<<std::endl;
      myfile << "sleep 2" <<std::endl;
  }

  myfile.close();


}
