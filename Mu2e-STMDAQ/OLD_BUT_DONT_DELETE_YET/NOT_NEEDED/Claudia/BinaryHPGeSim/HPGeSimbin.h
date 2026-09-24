#include <stdlib.h>
#include <stdio.h>
#include <math.h>
#include <iostream>
#include <fstream>
#include <cmath>
#include <numeric>
#include <random>
#include <list>

using namespace std;
using std::cout; using std::endl;
using std::chrono::duration_cast;
using std::chrono::milliseconds;
using std::chrono::seconds;
using std::chrono::system_clock;

// User defined variables
// -- Data
//Duration of the sample generated (microseconds)
const long double extSampleTime = 5000; // us
const long double intSampleTime = 2000; // us

// Total number of slices per packet
const int16_t sliceNum = 2;

// --  ELBE Experimental Variables

// ELBE clock
const int64_t clockELBE = 26*1e6; // 26 MHz in Hz

// Average beam (external) rate in detector
const double beamRateHz = 1e3; // Hz
// Average source (internal) rate in detector
const double sourceRateHz = 1e2; // Hz

// Micropulse frequency
const int microFreq_n = 6; // 1 - 8
const double microFreq = clockELBE/pow(2,microFreq_n); // Hz

// Macropulse width
const double macroWidth_ms = 0.1; // 0.1 - 0.36 ms - CHANGE THIS
const double macroWidth = macroWidth_ms*1e3; // us - DON'T CHANGE THIS

// Number of micropulse per macropulse
const int microPerMacro = (macroWidth*1e-6)*microFreq;


// Beam current

  
// Macropulse frequency
const double macroFreq = 1; // 1 - 25 Hz - CHANGE THIS
const double macroPeriod = 1/macroFreq*1e6; // us

// Macropulse gap wdith
const double macroGapWidth = 1/macroFreq*1e6 - macroWidth; // us

// // Registers
// const int prescale = 1;
const uint16_t ext_ADC_offset = 0; // us
// const int ext_slice_length
// const int ext_slice_num
// const int ext_trig_timeout
// const int ext_int_delay
const int int_trig_num = 10;
const uint16_t int_ADC_offset = 0; // us
// const int int_slice_length
// const int int_slice_num


// Struct containing header info
struct headerInfo{

  uint32_t macroCount;
  uint64_t macroTime;
  uint16_t mode;
  uint32_t microCount;
  uint16_t channel;
  uint16_t ADCoffset;  

};

//------------Variables of the pulses generated-----------------------

// ADC sampling frequency (Hz)
const double fADC = 320*1e6;
//Sampling time of ADC (microsec)
const double tadc=1/(fADC*1e-6);

//The amplitude of the pulse is the half of this value
const double twiceA=2370;
//The x point in where we have the pulse (50 us)
const double xshift=100;
//fall from baseline time
const double invtaufall=6;
//decaytime(rise to baseline)
const double invtaudecay=0.028;
// Standard deviation of gaussian noise
const double noiseSD = 10;
//Pulse duration (microsec)
const double pulselength=xshift+220;
//Number of ADC values in one pulse
const unsigned long int pulseLength=(xshift+220)/tadc;
//Number of bytes in one pulse
unsigned long int pulseBytes=2*pulseLength;

// Bytes in 8kb 10G packet
const uint pSize = 8198;
// Packet header size in bytes
const uint pHdrSize = 6;
// Packet header legnth
const uint pHdrLen = pHdrSize/2;
// Trigger header size in bytes
const uint tHdrSize = 32;
// Trigger header legnth
const uint tHdrLen = tHdrSize/2;
// Slice header size in bytes
const uint sHdrSize = 16;
// Slice header legnth
const uint sHdrLen = sHdrSize/2;


// Create the packet header 6 bytes (6 int16)
// Packet number 4 bytes (int16_t)
uint16_t* pHeader(uint32_t pNum){
  
  // Split packet number (4 bytes) into (2 bytes)
  uint16_t pn1 = pNum >> 16;
  uint16_t pn2 = pNum & 0x0000FFFF;

  // Store packet header
  uint16_t* p = new uint16_t[pHdrLen];
  p[0] = pn1;
  p[1] = pn2;
  p[2] = 0;
  
  // Return array
  return p;
  
}

// // Create the trigger header 32 bytes (16 int16)
uint16_t* tHeader(uint32_t macroNo, uint64_t macroTime,
		  uint16_t mode, uint32_t microNo, uint16_t channel,
		  uint16_t offset, uint16_t sliceNum){
  
  // Macropulse number (4 bytes) into (2 bytes)
  uint16_t man1 = macroNo >> 16;
  uint16_t man2 = macroNo & 0x0000FFFF;
  
  //Split macroTime (8 bytes) into (2 bytes)
  uint16_t mt1 = macroTime >> 48;
  uint16_t mt2 = macroTime >> 32;
  uint16_t mt3 = macroTime >> 16;
  uint16_t mt4 = macroTime & 0x0000FFFF;

  // Micropulse number (4 bytes) into (2 bytes)
  uint16_t min1 = microNo >> 16;
  uint16_t min2 = microNo & 0x0000FFFF;

  //Store the header into an array
  uint16_t* t = new uint16_t [tHdrLen];
  t[0] = man1;
  t[1] = man2;
  t[2] = mt1;
  t[3] = mt2;
  t[4] = mt3;
  t[5] = mt4;
  t[6] = mode;
  t[7] = min1;
  t[8] = min2;
  t[9] = channel;
  t[10] = offset;
  t[11] = sliceNum;
  t[12] = 0;
  t[13] = 0;
  t[14] = 0;
  t[15] = 0;

  // Return array
  return t;
  
}

// Create the slice header 16 bytes (8 int16)
uint16_t* sHeader(uint32_t sliceNo, uint64_t timefirstadc, uint32_t sizeslice){
  
  // SliceNum (4 bytes) into (2 bytes)
  uint16_t sn1 = sliceNo >> 16;
  uint16_t sn2 = sliceNo & 0x0000FFFF;
  
  //Time first ADC (8 bytes) into (2 bytes)
  uint16_t tfadc1 = timefirstadc >> 48;
  uint16_t tfadc2 = timefirstadc >> 32;
  uint16_t tfadc3 = timefirstadc >> 16;
  uint16_t tfadc4 = timefirstadc & 0x0000FFFF;

  // Slice slice (4 bytes) into (2 bytes)
  uint16_t ss1 = sizeslice >> 16;
  uint16_t ss2 = sizeslice & 0x0000FFFF;

  //Store the header into an array
  uint16_t* s = new uint16_t [sHdrLen];
  s[0] = sn1;
  s[1] = sn2;
  s[2] = tfadc1;
  s[3] = tfadc2;
  s[4] = tfadc3;
  s[5] = tfadc4;
  s[6] = ss1;
  s[7] = ss2;

  // Return array
  return s;     
      
}

int16_t pulseCalc(double x){

  int16_t pulse = -(twiceA / (1 + exp(-(x - xshift)*invtaufall)) ) * ( 1.0  - ( 1.0/ (1 + exp(-(x - xshift)*invtaudecay))));

  return pulse;
  
}


//////////////Create the binary file removing headers///////////////////

//Receive array with all headers and data stored in one macropulse(data[]), length of the array (dataNumTot),
//the number of packets, number of elements in one packet (packetLen)
//size of packet header (pHdrLen), trigger headers (tHdrLen)
//and ADC values

  //Skip packet headers and leave the first Trigger header

void CreateBinary (int16_t** data, unsigned long int dataNumTot, int packetNum, unsigned int packetLen, uint pHdrLen, uint tHdrLen)
{
  //For each packet
  unsigned long int n = dataNumTot-packetNum*pHdrLen-(packetNum-1)*tHdrLen;
  cout<<"n= "<<n<<endl;
  int16_t* bin_data = new int16_t[n];
  unsigned long int h=0; //from 0 to n



  for(int packetn=0;packetn<packetNum;packetn++){
    unsigned long int j=0;
    unsigned long int i=0;
    std::cout<<"Packet "<<packetn<<std::endl;
    int sizep;
    if(packetn==0){sizep=packetLen-pHdrLen;}
    else{sizep=packetLen-pHdrLen-tHdrLen;}
    while(i<sizep){
      //Remove packet header (6bytes = 3 uint16_t->Packet Header)
      if(j<pHdrLen){ //3
	j++;
	continue;}
      if(packetn==0){
        //Keep trigger header
        for(unsigned int k=0;k<(packetLen-pHdrLen);k++){bin_data[h]=data[packetn][j];
          //cout<<"j= "<<j<<" i= "<<i<<" data stored= "<<bin_data[h]<<endl;
          j++;
          i++;
          h++;}
      }
      else{j=j+tHdrLen; //16
        //Remove trigger headers
        for(unsigned int k=0;k<(packetLen-pHdrLen-tHdrLen);k++){bin_data[h]=data[packetn][j];
          //cout<<"j= "<<j<<" i= "<<i<<" data stored= "<<bin_data[h]<<endl;
          j++;
          i++;
          h++;}
      }
    }
  }


  char file_name[] = "Data.bin";
  //Open in read mode
  FILE *myFile = fopen(file_name, "rb");

  //If the binary file doesn't exist create it and write the array into the binary file
  if(!myFile){
   
    std::cout<<"First call, create and write to binary file"<<std::endl;
    FILE * fp = fopen(file_name, "wb");
    fwrite(&bin_data[0], sizeof(int16_t), n, fp );
    fclose(fp);
  }


  //Else if the binary file exists write the array at the end of the file
  if (myFile){
  
    std::cout<<"Call, open and append data to binary file"<<std::endl;
    FILE * fp = fopen(file_name, "a+");
    fwrite(&bin_data[0], sizeof(int16_t), n, fp);
    fclose(fp);
    }

  return;
}




  //////////////End function for creating binary file/////////////////









template <typename BidirectionalIterator, typename T>
BidirectionalIterator getClosest(BidirectionalIterator first, 
                                 BidirectionalIterator last, 
                                 const T & value)
{
  BidirectionalIterator before = std::lower_bound(first, last, value);

  if (before == first) return first;
  if (before == last)  return --last; // iterator must be bidirectional

  BidirectionalIterator after = before;
  --before;

  return (*after - value) < (value - *before) ? after : before;
}

template <typename BidirectionalIterator, typename T>
std::size_t getClosestIndex(BidirectionalIterator first, 
                            BidirectionalIterator last, 
                            const T & value)
{
  return std::distance(first, getClosest(first, last, value));
}
