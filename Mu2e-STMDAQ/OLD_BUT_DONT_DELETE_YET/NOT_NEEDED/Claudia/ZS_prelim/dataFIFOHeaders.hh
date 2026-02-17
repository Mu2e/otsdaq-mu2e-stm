/////////////////////////////////////////////////////////////////////////
/// A header file containing all the FIFO header size and information
/////////////////////////////////////////////////////////////////////////

#ifndef DATAFIFOHEADERS_hh
#define DATAFIFOHEADERS_hh

#include <iostream>
#include <stdlib.h>
#include <stdio.h>
#include <bits/stdc++.h>


#ifndef DATAFIFOHEADERS_hh_DEFINED
#define DATAFIFOHEADERS_hh_DEFINED
class dataFIFOHeaders;
#endif

using namespace std;


// ****** FIFO header in bytes
const uint infifo_hdr_Size = 8;
// FIFO header length in ADC
const uint infifo_hdr_Len = infifo_hdr_Size/2;

const uint infifo_hdr_lastZStrigstored1 = 0; // Last trigger stored by ZS 1
const uint infifo_hdr_lastZStrigstored2 = 1; // Last trigger stored by ZS 2
const uint infifo_hdr_lastZStrigstored3 = 2; // Last trigger stored by ZS 3
const uint infifo_hdr_lastZStrigstored4 = 3; // Last trigger stored by ZS 4




// ****** FIFO EVENT/TRIG header in bytes
const uint infifo_thdr_Size = 32;
// FIFO EVENT/TRIG header length in ADC
const uint infifo_thdr_Len = infifo_thdr_Size/2;

const uint infifo_thdr_TrigTime1 = 4; // Trigger time 1
const uint infifo_thdr_TrigTime2 = 5; // Trigger time 2
const uint infifo_thdr_TrigTime3 = 6; // Trigger time 3
const uint infifo_thdr_ChTime4 = 7; // Channel + Trigger time 4
const uint infifo_thdr_TrigNum1 = 8; // Trigger number 1
const uint infifo_thdr_TrigNum2 = 9; // Trigger number 2
const uint infifo_thdr_TrigNum3 = 10; // Trigger number 3 
const uint infifo_thdr_ADCoffset1 = 11; // ADC offset 1
const uint infifo_thdr_evtwtag1 = 12; // Event window tag 1
const uint infifo_thdr_evtwtag2 = 13; // Event window tag 2
const uint infifo_thdr_evtwtag3 = 14; // Event window tag 3
const uint infifo_thdr_evtmode1 = 15; // Event Mode 1 
const uint infifo_thdr_evtmode2 = 16; // Event Mode 2 
const uint infifo_thdr_DRingMarkerevtmode3 = 17; // Delivery Ring RF Marker + Event Mode 3
const uint infifo_thdr_TrigLen1 = 18; //Event length (1.675-1.700 us
const uint infifo_thdr_1 = 19; // BEEF


const uint infifo_hdrs_Len=infifo_hdr_Len+infifo_thdr_Len;

//const unsigned long int n = 536870912; //filesize ADC run00109.new.bin_00
const unsigned long int n = 320052; //filesize ADC genData662keV_30kHz.bin 
//const double hardware_clockSPILL = 1.695; //us
const double hardware_shortclockSPILL = 1.675; //us
const double hardware_longclockSPILL = 1.7; //us
const double hardwareclockGAP = 100; //us
const uint NRAMblocks = 5;
const uint ADCRAM = 2048; //ADC values per RAM block
const double fADC = 370; //MHz
const uint window = 100;
const int threshold = -100;
const uint tbefore = 1; //us
const uint tafter = 2; //us



class dataFIFOHeaders{

 public:

  // Standard  constructor
  dataFIFOHeaders();

 private:

};

#endif
