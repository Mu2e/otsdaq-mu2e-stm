#ifndef ZS_H
#define ZS_H

#include<iostream>
#include <string>
#include <fstream>
#include <vector>
#include <utility> // std::pair
#include <stdexcept> // std::runtime_error
#include <sstream> // std::stringstream
#include<fstream>
#include<cmath>
#include <sys/stat.h>
#include <cstring>


#include "STMDAQ-TestBeam/utils/xml.hh"
#include "STMDAQ-TestBeam/utils/EnvVars.hh"

using namespace std;


class ZS {
 public:

  //Constructors
  ZS();
  
  ZS(double _hardwareclockSPILL, double _hardwareclockGAP, double _NRAMblocks, double _ADCRAM, unsigned _headersize, double _fADC, int _window, double _threshold, double _tbefore, double _tafter);
  
  ZS(Xml* xml);
  
  //ZS methods
  std::string print();
  int16_t* ReadBinaryFile(std::string filename);
  unsigned long int Returnfsize(){return fsize;}
  int16_t* Form_InputFIFO(int16_t* nADC, int16_t _mode, unsigned long int _chunknumber, unsigned long int _chunkstart, unsigned long int _trignumstart, unsigned long int _last_triggerstored, int32_t _numADCtoZS);
  void ReadInputHeader (int16_t*  allchunk);
  void supalg(int16_t* inFIFO);
  unsigned long int Return_ZSTriggers(){return ntriggers;}
  void ZS_array();
  void Form_OutputFIFO();
  int16_t* Return_ZSOutputFIFO(){return sendsupdata;}
  int Return_ZSFIFOSize(){return zp_size;}
  
  //ZS Parameters
  //Sampling time of ADC (microsec) 
  double tadc_ (){return 1.0/(fADC);}
  //store tbefore microseconds of data to the left of the trigger 
  int prenumADCstored_ (double tadc){return int(tbefore/tadc);}
  //store tafter microseconds of data to the right of the trigger  
  int postnumADCstored_ (double tadc){return int(tafter/tadc);}
  //Theoretical number of ADC values per chunk
  unsigned long int chunk_ (){return (NRAMblocks*ADCRAM);}
  //Number of triggers per chunk to send in Spill
  unsigned long int ntriggers_chunkSPILL_ (){double chunkdoub=NRAMblocks*ADCRAM; return ((chunkdoub/fADC)/hardwareclockSPILL);}
  //Number of ADC values per trigger in Spill  
  unsigned long int nADC_triggerSPILL_ (){return (fADC*hardwareclockSPILL);}
  //Number of ADC values per chunk according to triggers in Spill 
  unsigned long int nADC_chunk_(unsigned long int nADC_triggerSPILL, unsigned long int ntriggers_chunkSPILL) {return (nADC_triggerSPILL*ntriggers_chunkSPILL);}
  //Number of ADC values per trigger in Gap 
  unsigned long int nADC_triggerGAP_ (){return (fADC*hardwareclockGAP);}

 private:
  //initialized in constructor
  double hardwareclockSPILL, hardwareclockGAP, NRAMblocks, ADCRAM, fADC, threshold, tbefore, tafter;
  unsigned headersize;
  int window;

  //initialized in ZS::ReadBinaryFile
  unsigned long int fsize;

  //initialized in ZS::ReadInputHeader
  int16_t mode;
  unsigned long int chunknum, chunkstart, trignumstart, last_triggerstored;
  int32_t numADCtoZS; 

  //initialized in ZS::supalg
  double tadc; 
  int prenumADCstored, postnumADCstored; 
  unsigned long int ntriggers_chunkSPILL, nADC_triggerSPILL, nADC_chunk, nADC_triggerGAP, ntriggers, lasttrig;
  int16_t* ADC;
  unsigned long int* realtrigger_vect;
  unsigned long int* realtrigger_abs;

  //initialized in ZS::ZS_array
  int16_t* suppressed_data;
  int16_t init;
 
  //initialized in ZS::Form_OutputFIFO
  int16_t* sendsupdata;
  int zp_size;
};






#endif
