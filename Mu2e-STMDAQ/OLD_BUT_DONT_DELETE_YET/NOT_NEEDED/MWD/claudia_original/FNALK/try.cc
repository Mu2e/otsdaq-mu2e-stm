#include <iostream>
#include <vector>
#include <cstring>
#include <fstream>
#include "/home/stm_mu2e/STMDAQ-TestBeam/MWD/FNALK/calcFuncs.h"


int main (int argc, char *argv[]){

  char* fname = argv[1];

  calcFuncs fcalcFuncs;
  //Reads the file, subtract the headers and fill the data into a vector
  std::vector<int16_t> DataADC;

  std::vector<unsigned> Channel_Num;
  std::vector<unsigned long int> DTCClock_Num;
  std::vector<unsigned long int> ADCClock_Num;
  std::vector<unsigned long int> Event_Num;

  std::vector<unsigned long int> Startdata_index;

  DataADC = fcalcFuncs.readFile(fname);
  //Get  channel, DTCClock, ADCClock_Num, Event number and Startdata index per event vector
  Channel_Num = fcalcFuncs.returnChannel_Num();
  DTCClock_Num = fcalcFuncs.returnDTCClock_Num();
  ADCClock_Num = fcalcFuncs.returnADCClock_Num();
  Event_Num = fcalcFuncs.returnEvent_Num();
  Startdata_index = fcalcFuncs.returnStartdata_index();

  //Generate a binary file just with the data
  char file_name[] = "FNALsinewave_2023-11-09_18-18-46_LaBr_0.bin";
  FILE * fp = fopen(file_name, "wb");
  //unsigned long int n = DataADC.size();
  unsigned long int n = 300000000;
  fwrite(&DataADC[0], sizeof(int16_t), n, fp);                                                                          
  fclose(fp);
}
