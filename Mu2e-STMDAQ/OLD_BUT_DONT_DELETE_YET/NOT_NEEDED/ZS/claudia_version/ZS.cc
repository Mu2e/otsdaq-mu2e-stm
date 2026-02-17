#include "STMDAQ-TestBeam/ZS/ZS.hh"

ZS::ZS(){ 
 hardwareclockSPILL = 1.695; //us 
 hardwareclockGAP = 100; //us 
 NRAMblocks = 5;  
 ADCRAM = 2048; //ADC values per RAM block  
 headersize = 19; //input header size in ADC values  
 fADC = 370; //MHz 
 window = 100; 
 threshold = -100; 
 tbefore = 1; //us 
 tafter = 2;//us 
} 


ZS::ZS(double _hardwareclockSPILL, double _hardwareclockGAP, double _NRAMblocks, double _ADCRAM, unsigned _headersize, double _fADC, int _window, double _threshold, double _tbefore, double _tafter){ 
 hardwareclockSPILL = _hardwareclockSPILL; 
 hardwareclockGAP = _hardwareclockGAP; 
 NRAMblocks = _NRAMblocks; 
 ADCRAM = _ADCRAM; 
 headersize = _headersize; 
 fADC = _fADC; 
 window = _window; 
 threshold = _threshold; 
 tbefore = _tbefore; 
 tafter = _tafter; 
} 


ZS::ZS(Xml* xml_file){ 
 hardwareclockSPILL = (double) xml_file->int_value("stm.hardwareclockSPILL",1.695); 
 hardwareclockGAP = (double) xml_file->int_value("stm.hardwareclockGAP",100); 
 NRAMblocks = (double) xml_file->float_value("stm.NRAMblocks",5); 
 ADCRAM = (double) xml_file->float_value("stm.ADCRAM",2048); 
 headersize =  xml_file->float_value("stm.headersize",19); 
 fADC = (double) xml_file->float_value("stm.adc_time_clock",370.0); 
 window = (double) xml_file->int_value("stm.window",100); 
 threshold = (double) xml_file->float_value("stm.threshold",-100); 
 tbefore = (double) xml_file->float_value("stm.tbefore",1); 
 tafter = (double) xml_file->float_value("stm.tafter",2); 
} 


std::string ZS::print() { 
 std::stringstream ss; 
 ss << "hardwareclockSPILL     = " << hardwareclockSPILL << "\n"  
 << "hardwareclockGAP       = " << hardwareclockGAP << "\n"  
 << "NRAMblocks             = " << NRAMblocks << "\n"  
 << "ADCRAM                 = " << ADCRAM << "\n"  
 << "headersize             = " << headersize << "\n"  
 << "fADC [MHz]             = " << fADC << "\n"  
 << "window                 = " << window << "\n"  
 << "threshold              = " << threshold << "\n"  
 << "tbefore                = " << tbefore << "\n"  
 << "tafter                 = " << tafter << "\n";  
 return ss.str(); 
} 


int16_t* ZS::ReadBinaryFile(std::string filename){ 
 std::ifstream ipfile; 
 ipfile.open(filename, std::ios::in | std::ios::binary); 
 ipfile.seekg(0, std::ios::end); 
 fsize = ipfile.tellg(); 
 ipfile.seekg(0, std::ios::beg); 
 int16_t* data = new int16_t[fsize]; 
 ipfile.read( (char*) data, fsize*sizeof(data[0])); 
 std::cout<<"File size: "<<fsize<<" bytes, "<<fsize/2<<" ADC"<<std::endl; 
 return data; 
} 


int16_t* ZS::Form_InputFIFO(int16_t* nADC, int16_t _mode, unsigned long int _chunknumber, unsigned long int _chunkstart, unsigned long int _trignumstart, unsigned long int _last_triggerstored, int32_t _numADCtoZS){ 
 unsigned long int _chunk= chunk_(); 
 int16_t*  allchunk = new int16_t[_chunk]; 
 //Mode 
 allchunk[0]=_mode; 

 //Chunk number 
 allchunk[1]=_chunknumber & 0x0000FFFF; 
 allchunk[2]=_chunknumber >> 16; 
 allchunk[3]=_chunknumber >> 32; 
 allchunk[4]=_chunknumber >> 48; 
 
 //Chunk start 
 allchunk[5]=_chunkstart & 0x0000FFFF; 
 allchunk[6]=_chunkstart >> 16; 
 allchunk[7]=_chunkstart >> 32; 
 allchunk[8]=_chunkstart >> 48; 

 //First trigger number 
 allchunk[9]=_trignumstart & 0x0000FFFF; 
 allchunk[10]=_trignumstart >> 16; 
 allchunk[11]=_trignumstart >> 32; 
 allchunk[12]=_trignumstart >> 48; 

 //Last trigger stored 
 allchunk[13]=_last_triggerstored & 0x0000FFFF; 
 allchunk[14]=_last_triggerstored >> 16; 
 allchunk[15]=_last_triggerstored >> 32; 
 allchunk[16]=_last_triggerstored >> 48; 

 //number of ADC values to ZS in this chunk 
 allchunk[17]=_numADCtoZS & 0x0000FFFF; 
 allchunk[18]=_numADCtoZS >> 16; 

 #ifdef PRINT_COUT
 std::cout<<"mode "<<_mode<<std::endl;
 std::cout<<"Chunk number: "<<_chunknumber<<std::endl;
 std::cout<<"Chunk start in ADC: "<<_chunkstart<<std::endl;
 std::cout<<"1st trigger number in this chunk "<<_trignumstart<<std::endl;
 std::cout<<"Last ZS trigger from previous chunk: "<<_last_triggerstored<<std::endl;
 std::cout<<"Number of ADC values to ZS in this chunk: "<<_numADCtoZS<<std::endl;
 #endif

 //nADC_chunk ADC values 
 for (unsigned j = headersize; j < (_numADCtoZS+headersize); j++) { 
  allchunk[j]=nADC[_chunkstart+j-headersize]; 
  //std::cout<<j<<" data "<<inFIFO[j]<<std::endl; 
 } 
 //rest of ADC values=0 
 for (unsigned j = (_numADCtoZS+headersize); j < _chunk; j++) { 
  allchunk[j]=0; 
  //std::cout<<j<<" data "<<inFIFO[j]<<std::endl; 
 } 
 return allchunk; 
} 


void ZS::ReadInputHeader (int16_t*  allchunk){ 
  // std::cout<<"------------Executing TOP function------------"<<std::endl; 
 //Read header 
 mode = allchunk[0]; 
 
 unsigned long int chunknum4 = (unsigned long int) allchunk[1] << 0    & (unsigned long int) 0x000000000000FFFF; 
 unsigned long int chunknum3 = (unsigned long int) allchunk[2] << 16 & (unsigned long int) 0x00000000FFFF0000; 
 unsigned long int chunknum2 = (unsigned long int) allchunk[3] << 32 & (unsigned long int) 0x0000FFFF00000000; 
 unsigned long int chunknum1 = (unsigned long int) allchunk[4] << 48 & (unsigned long int) 0xFFFF000000000000; 
 chunknum = (unsigned long int)(chunknum1 | chunknum2 | chunknum3 | chunknum4); 
 
 unsigned long int chunkstart8 = (unsigned long int) allchunk[5] << 0    & (unsigned long int) 0x000000000000FFFF; 
 unsigned long int chunkstart7 = (unsigned long int) allchunk[6] << 16 & (unsigned long int) 0x00000000FFFF0000; 
 unsigned long int chunkstart6 = (unsigned long int) allchunk[7] << 32 & (unsigned long int) 0x0000FFFF00000000; 
 unsigned long int chunkstart5 = (unsigned long int) allchunk[8] << 48 & (unsigned long int) 0xFFFF000000000000; 
 chunkstart = (unsigned long int)(chunkstart5 | chunkstart6 | chunkstart7 | chunkstart8); 
 
 unsigned long int trignumstart12 = (unsigned long int) allchunk[9] << 0    & (unsigned long int) 0x000000000000FFFF; 
 unsigned long int trignumstart11 = (unsigned long int) allchunk[10] << 16 & (unsigned long int) 0x00000000FFFF0000; 
 unsigned long int trignumstart10 = (unsigned long int) allchunk[11] << 32 & (unsigned long int) 0x0000FFFF00000000; 
 unsigned long int trignumstart9 = (unsigned long int) allchunk[12] << 48 & (unsigned long int) 0xFFFF000000000000; 
 trignumstart = (unsigned long int)(trignumstart9 | trignumstart10 | trignumstart11 | trignumstart12); 
 
 unsigned long int lasttrigstored16 = (unsigned long int) allchunk[13] << 0    & (unsigned long int) 0x000000000000FFFF; 
 unsigned long int lasttrigstored15 = (unsigned long int) allchunk[14] << 16 & (unsigned long int) 0x00000000FFFF0000; 
 unsigned long int lasttrigstored14 = (unsigned long int) allchunk[15] << 32 & (unsigned long int) 0x0000FFFF00000000; 
 unsigned long int lasttrigstored13 = (unsigned long int) allchunk[16] << 48 & (unsigned long int) 0xFFFF000000000000; 
 last_triggerstored  = (unsigned long int)(lasttrigstored13 | lasttrigstored14 | lasttrigstored15 | lasttrigstored16); 
 
 int32_t numADCtoZS18= (int32_t) allchunk[17] << 0 & (int32_t) 0x0000FFFF; 
 int32_t numADCtoZS17= (int32_t) allchunk[18] << 16 & (int32_t) 0xFFFF0000; 
 numADCtoZS  = (int32_t)(numADCtoZS17 | numADCtoZS18 ); 
 
 #ifdef PRINT_COUT
 std::cout<<"Check Mode: "<<mode<<std::endl;
 std::cout<<"Check Chunk Number: "<<chunknum<<std::endl;
 std::cout<<"Check Chunk starts in: "<<chunkstart<<" ADC values"<<std::endl;
 std::cout<<"Check Trig Num start: "<<trignumstart<<std::endl;
 std::cout<<"Check last trigger stored: "<<last_triggerstored<<std::endl;
 std::cout<<"Check number of ADC values to ZS in this chunk: "<<numADCtoZS<<std::endl;
 #endif

 return; 
}


void ZS::supalg (int16_t* allchunk){ 
 //Parameters 
 tadc= tadc_(); 
 prenumADCstored= prenumADCstored_(tadc); 
 postnumADCstored= postnumADCstored_(tadc); 
 //chunk= chunk_(); 
 ntriggers_chunkSPILL= ntriggers_chunkSPILL_(); 
 nADC_triggerSPILL= nADC_triggerSPILL_(); 
 nADC_chunk= nADC_chunk_(nADC_triggerSPILL, ntriggers_chunkSPILL); 
 nADC_triggerGAP=  nADC_triggerGAP_(); 
 ADC=new int16_t[nADC_chunk];
 //gradient vector 
 int16_t gradient[nADC_chunk-window]; 
 //averaged gradient vector 
 double avgradient[nADC_chunk-window]; 
 //time in clocks ticks for each raw ADC values 
 double time[nADC_chunk]; 
 //averaged time for each averaged gradient 
 double avADCtime[nADC_chunk-window]; 
 //vector with each trigger time in clock ticks 
 unsigned long int trigger_vect[nADC_chunk]; 
 //vector with each trigger in absolute clock ticks ie starting each chunk in ADC=0 
 unsigned long int trigger_abs[nADC_chunk]; 
 //Just read the ADC values, not the ADC=0 
  for (int32_t i = 0; i < numADCtoZS; i++) { 
    ADC[i] = allchunk[headersize+i]; 
    //trigger time in clock ticks 
    time[i]=i; 
  } 
 //Calculate the gradient vector 
  for(int32_t i=0;i<(numADCtoZS-window);i++){ 
    gradient[i]=ADC[i+window]-ADC[i]; 
    //cout<<gradient[i]<<endl; 
  } 
 //Calculate the average of the gradient each n_average ADC values to avoid fluctuations 
 int n_average=5; 
 unsigned long int j=0; 
 double av_gradient=0; 
 double av_ADCtime=0; 
 unsigned long int h=0; 
 lasttrig=0; 
 
 #ifdef PRINT_COUT
 std::cout<<"Average of gradient taken with: "<<n_average<<" values"<<std::endl; 
 #endif

 while(j<(numADCtoZS-window)){ 
 //Each point of the gradient and ADCtime averaged with the (n_average-1) following points, each point is the mean of n_average points of the gradient 
 if((j+n_average)>(numADCtoZS-window)){n_average= (numADCtoZS-window)-j;} 
  av_gradient=0; 
  av_ADCtime=0; 
  for(int k=0;k<n_average;k++){ 
   av_gradient= av_gradient+gradient[j+k]; 
   av_ADCtime=av_ADCtime+time[j+k]; 
  } 
  avgradient[h]=av_gradient/n_average; 
  avADCtime[h]=av_ADCtime/n_average; 
  j=j+n_average; 
  h++; 
 } 
 //Initial values 
 bool peak=false; 
 int counter=0; 
 int trigger=0; 
 unsigned long int triggercounter=0; 
 unsigned long int size_sup;

 #ifdef PRINT_COUT 
 std::cout<<"fadc= "<<fADC<<" tadc= "<<tadc<<" MHz, window: "<<window<<" threshold "<<threshold<<" tbefore: "<<tbefore<<" us ("<<prenumADCstored<<" ADC Counts)"<<" tafter "<<tafter<<" us ("<<postnumADCstored<<" ADC Counts)"<<std::endl; 
 #endif

 //Store positions in clock ticks for the triggers found, fill trigger_vect[] 
 //h is the number of elements in the averaged gradient vector 
 for(unsigned long int i=0;i<h;i++){ 
  if(avgradient[i]>threshold){peak=false; 
   continue;} 
  //skip the rest indexes of the peak after the trigger that has already been stored 
  if((avgradient[i]<threshold)&&(peak==true)){ 
   continue;} 
  if((avgradient[i]<threshold)&&(peak==false)){ 
   peak=true; 
   trigger=avADCtime[i]; 
   trigger_abs[triggercounter]=avADCtime[i]; 
   trigger_vect[triggercounter]=chunkstart+trigger;
   
   #ifdef PRINT_COUT
   std::cout<<"absolute time: "<<trigger<<std::endl; 
   std::cout<<"trig: "<<trigger_vect[triggercounter]<<std::endl; 
   #endif

   //std::cout<<"avgrad"<<avgradient[i]<<" Trigger number: "<<triggercounter<<": "<<trigger<<" Triggertime: "<<trigger*tadc<<std::endl; 
   triggercounter++; 
  } 
 } 

 #ifdef PRINT_COUT
 std::cout<<"Number of triggers found in chunk: "<< triggercounter<<std::endl;
 #endif

 //real number of triggers and vector with triggers that will store data in each chunk 
 unsigned long int realtrignum=0;
 realtrigger_vect=new unsigned long int[nADC_chunk]; //no more triggers than adc values per chunk  
 realtrigger_abs=new unsigned long int[nADC_chunk];
 //For now in the result we are going to store triggers in clock ticks 
 for (unsigned long int i = 0; i < triggercounter; i++) { 
 //if trigger found is little than last trigger stored from previous call, don't store it (give it a margin of window to not store twice the same trigger in different chunks). Also don't store this trigger in this chunk if the number of adc values to store after this trigger exceeds the chunk limit: trigger+postnumADCstored>=chunkstart+nADC_chunk 
 //or if new ZS trigger found is closer to the previous one prenumADCstored 
  if((trigger_vect[i]<=last_triggerstored)||(trigger_vect[i]-prenumADCstored<=last_triggerstored)||(trigger_vect[i]+postnumADCstored>=chunkstart+numADCtoZS)){continue;} 
  else{ 
   lasttrig=trigger_vect[i]; 
   //fill with real triggers to store data 
   realtrigger_vect[realtrignum]=trigger_vect[i]; 
   realtrigger_abs[realtrignum]=trigger_abs[i]; 
   realtrignum++; 
  } 
 } 
 //number of triggers without repetition 
 ntriggers=realtrignum;

  return; 
 } 


void ZS::ZS_array(){ 
 //store suppressed data 
 suppressed_data=new int16_t[nADC_chunk]; 
 //Suppressed data array initialise to a big number 
 init=3000; 
 //Initialise suppressed array with a number 
 for(int i=0;i<numADCtoZS;i++){suppressed_data[i]=init;} 
 for(unsigned long int  j=0;j< ntriggers;j++){ 
  int k=-prenumADCstored; 
  //Fill trigger positions with -pre and +post adc values 
  while(k<=postnumADCstored){ 
   //Just store the data if the index (realtrigger_abs[j]+k) is higher than the last ADC value stored in previous chunk (last_triggerstored+postnumADCstored) 
   //We have to convert last trigger stored+post to absolute value relative to the start of the chunk 
   double lastvaluestored= last_triggerstored+postnumADCstored; 
   double limit = lastvaluestored-chunkstart; 
   //see that there are prenumADC stored 
   int checkprenum = int(realtrigger_abs[j])+k; 
   //std::cout<<"lastvaluestored "<<lastvaluestored<<" limit "<<limit <<" last_triggerstored "<<last_triggerstored<<" chunkstart "<<chunkstart<<" realtrigger_abs[j]+k "<<realtrigger_abs[j]+k<<" realtrigger_abs[j] "<< realtrigger_abs[j]<<" k "<< k<<" checkprenum "<<checkprenum<<std::endl; 
    if(((realtrigger_abs[j]+k)>limit)&&(checkprenum>=0)){ 
     suppressed_data[realtrigger_abs[j]+k]=ADC[realtrigger_abs[j]+k];} 
    else{
      #ifdef PRINT_COUT
      std::cout<<"This value is not stored, index: "<<int(realtrigger_abs[j])+k<<std::endl;
      #endif
    }
    if(checkprenum<0){
      #ifdef PRINT_COUT
      std::cout<<"Not prenum ADC values to store in this chunk for this ZS trigger, not all values required are stored for this trigger---wrong energy expected"<<std::endl;
      #endif
    } 
   k++; 
  } 
 } 
 //Update last trigger stored in this function call 
  if(lasttrig!=0){  
  last_triggerstored=lasttrig; 
 } 

 #ifdef PRINT_COUT
 std::cout<<"Last trigger stored updated: "<<last_triggerstored<<std::endl; 
 #endif
 
 return; 
 } 


void ZS::Form_OutputFIFO() { 
 //Form array with data and headers 
 sendsupdata = new int16_t[nADC_chunk]; 
 unsigned long int ADC_hardtrig = nADC_triggerSPILL; 
 //array length to send with headers and suppressed data (headers+supdata) 
 int b=0; 
 //index of just supdata in the array to send (supdata) 
 int i=0; 
 //length of first trigger 
 unsigned long int triglength=ADC_hardtrig; 
 
 #ifdef PRINT_COUT
 std::cout<<"------------OUTPUT FIFO"<<std::endl; 
 #endif

 if(mode==0){ 
  //Last ZS trigger found in this chunk is the 4 first elements of the output array 
  sendsupdata[0]=last_triggerstored & 0x0000FFFF; 
  sendsupdata[1]=last_triggerstored >> 16; 
  sendsupdata[2]=last_triggerstored >> 32; 
  sendsupdata[3]=last_triggerstored >> 48; 
  //Mode 
  sendsupdata[4]=mode; 
  //Input chunk number 
  sendsupdata[5]=chunknum & 0x0000FFFF; 
  sendsupdata[6]=chunknum >> 16; 
  sendsupdata[7]=chunknum >> 32; 
  sendsupdata[8]=chunknum >> 48; 
  //Input chunk start in ADC 
  sendsupdata[9]=chunkstart & 0x0000FFFF; 
  sendsupdata[10]=chunkstart >> 16; 
  sendsupdata[11]=chunkstart >> 32; 
  sendsupdata[12]=chunkstart >> 48; 
  b=b+13; 
  //for number of triggers in hardware 
  for(unsigned long int j=0;j<ntriggers_chunkSPILL;j++){ 
   int16_t numstart_data=0; 
   int16_t numstart_dataplus=0; 
   int16_t valueswritten=0; 
   int16_t slices=0; 
   bool increasecounter=false; 
   bool datadelay=false; 
   
   #ifdef PRINT_COUT
   std::cout<<"NEW HARDWARE TRIGGER----------"<<j; 
   #endif

   //for each trigger length 
   while(i<triglength){

   #ifdef PRINT_COUT
   std::cout<<" starting at index: "<<i<<std::endl; 
   #endif
 
   if(suppressed_data[i]!=init){slices++;} 
   //if values stored don't start at the beginning but at some point in the middle of the trigger, remove previous header  
   if(datadelay==true){b=b-7; 
   
   #ifdef PRINT_COUT
   std::cout<<"Removed previous header"<<std::endl;
   #endif

   } 
   while((suppressed_data[i]!=init)&&(i<triglength)){ 
     //triggernumber 
     sendsupdata[b]=j & 0x0000FFFF; 
     sendsupdata[b+1]=j >> 16; 
     sendsupdata[b+2]=j >> 32; 
     sendsupdata[b+3]=j >> 48; 
     //number of slices 
     sendsupdata[b+4]=slices; 
     //start of data relative to the hardware trigger 
     sendsupdata[b+5]=numstart_data; 
     //number of values written 
     valueswritten++; 
     sendsupdata[b+6]=valueswritten; 
     //suppressed data in this trigger 
     sendsupdata[b+6+valueswritten]=suppressed_data[i]; 
     // cout<<"-stored: "<<i<<" "<<suppressed_data[i]<<"="<<sendsupdata[b+3+valueswritten]<<endl; 
     i++; 
     numstart_dataplus++; 
     increasecounter=true; 
    } 
    if(increasecounter==true){ 
    
      #ifdef PRINT_COUT
      std::cout<<"Header: trignum: "<<sendsupdata[b]<<" Nslices: "<<sendsupdata[b+4]<<" Start data: "<<sendsupdata[b+5]<<" Number of sup values: "<<sendsupdata[b+6]<<std::endl; 
      #endif
      
      b=b+7+valueswritten;numstart_data=numstart_dataplus;} 
      while((suppressed_data[i]==init)&&(i<triglength)&&(increasecounter==true)){i++;numstart_data++;valueswritten=0;} 
      while((suppressed_data[i]==init)&&(i<triglength)&&(increasecounter==false)){ 
      //triggernumber 
      sendsupdata[b]=j & 0x0000FFFF; 
      sendsupdata[b+1]=j >> 16; 
      sendsupdata[b+2]=j >> 32; 
      sendsupdata[b+3]=j >> 48; 
      //number of slices 
      sendsupdata[b+4]=slices; 
      //start of data relative to the hardware trigger 
      sendsupdata[b+5]=numstart_data; 
      //number of values written 
      sendsupdata[b+6]=0; 
      i++; 
      numstart_dataplus++; 
      increasecounter=false;} 
      if(increasecounter==false){ 
      datadelay=true; 

      #ifdef PRINT_COUT
      std::cout<<"Header: trignum: "<<sendsupdata[b]<<" Nslices: "<<sendsupdata[b+4]<<" Start data: "<<sendsupdata[b+5]<<" Number of sup values: "<<sendsupdata[b+6]<<std::endl; 
      #endif

      b=b+7;numstart_data=numstart_dataplus;} 
   }//while triglength 
   triglength= triglength+ADC_hardtrig; 
  }//for ntriggers 
 }//IF MODE 0 
 if(mode==1){ 
  int16_t numstart_data=0; 
  int16_t numstart_dataplus=0; 
  int16_t valueswritten=0; //This could give an error if we write all the trigger (37000 ADC values) because int16 range is 32000 values 
  int16_t slices=0; 
  bool increasecounter=false; 
  bool datadelay=false; 
  //Last ZS trigger found in this chunk is the 4 first elements of the output array 
  sendsupdata[0]=last_triggerstored & 0x0000FFFF; 
  sendsupdata[1]=last_triggerstored >> 16; 
  sendsupdata[2]=last_triggerstored >> 32; 
  sendsupdata[3]=last_triggerstored >> 48; 
  //Mode 
  sendsupdata[4]=mode; 
  //Input chunk number 
  sendsupdata[5]=chunknum & 0x0000FFFF; 
  sendsupdata[6]=chunknum >> 16; 
  sendsupdata[7]=chunknum >> 32; 
  sendsupdata[8]=chunknum >> 48; 
  //Input chunk start in ADC 
  sendsupdata[9]=chunkstart & 0x0000FFFF; 
  sendsupdata[10]=chunkstart >> 16; 
  sendsupdata[11]=chunkstart >> 32; 
  sendsupdata[12]=chunkstart >> 48; 
  b=b+13; 
 //Loop over all suppressed_data array 
 while(i<numADCtoZS){ 
  if(suppressed_data[i]!=init){slices++;} 
  //if values stored don't start at the beginning but at some point in the middle of the trigger, remove previous header 
  if(datadelay==true){b=b-7;
      
      #ifdef PRINT_COUT
      std::cout<<"Removed previous header"<<std::endl;
      #endif

    } 
  while((suppressed_data[i]!=init)&&(i<numADCtoZS)){ 
   //Reconstruct Trigger number from input 
   sendsupdata[b]=trignumstart & 0x0000FFFF; 
   sendsupdata[b+1]=trignumstart >> 16; 
   sendsupdata[b+2]=trignumstart >> 32; 
   sendsupdata[b+3]=trignumstart >> 48; 
   //number of slices 
   sendsupdata[b+4]=slices; 
   //start of data relative to the hardware trigger 
   sendsupdata[b+5]=numstart_data; 
   //number of values written 
   valueswritten++; 
   sendsupdata[b+6]=valueswritten; 
   //suppressed data in this trigger 
   sendsupdata[b+6+valueswritten]=suppressed_data[i]; 
   numstart_dataplus++; 
   increasecounter=true; 
   datadelay=false; 
   i++; 
  } 
 if(increasecounter==true){
      
      #ifdef PRINT_COUT
      std::cout<<"Header: trignum: "<<trignumstart<<" Nslices: "<<sendsupdata[b+4]<<" Start data: "<<sendsupdata[b+5]<<" Number of sup values: "<<sendsupdata[b+6]<<std::endl; 
      #endif

      b=b+7+valueswritten;} 
 while((suppressed_data[i]==init)&&(i<numADCtoZS)){ 
      valueswritten=0; 
      if(increasecounter==false){ 
      //Reconstruct Trigger number from input 
      sendsupdata[b]=trignumstart & 0x0000FFFF; 
      sendsupdata[b+1]=trignumstart >> 16; 
      sendsupdata[b+2]=trignumstart >> 32; 
      sendsupdata[b+3]=trignumstart >> 48; 
      //number of slices 
      sendsupdata[b+4]=slices; 
      //start of data relative to the hardware trigger 
      sendsupdata[b+5]=numstart_data; 
   //number of values written 
      sendsupdata[b+6]=valueswritten; 
    } 
      numstart_dataplus++; 
      i++; 
    } 
 if(increasecounter==false){ 
  datadelay=true;
  
  #ifdef PRINT_COUT
  std::cout<<"Header: trignum: "<<trignumstart<<" Nslices: "<<sendsupdata[b+4]<<" Start data: "<<sendsupdata[b+5]<<" Number of sup values: "<<sendsupdata[b+6]<<std::endl; 
  #endif

  b=b+7; 
 } 
 numstart_data=numstart_dataplus; 
  } 
 } 
 //b is the size of the suppressed array (including headers and data) 
 zp_size=b;
}
