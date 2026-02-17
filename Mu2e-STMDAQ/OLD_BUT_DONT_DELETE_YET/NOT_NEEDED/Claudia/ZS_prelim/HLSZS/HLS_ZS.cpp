#include "HLS_ZS.h" 



void HLS_ZS (uint16_t allchunk[chunk], out_stream_t &output, uint64_t &ntriggers, uint32_t &zp_size) { 


#pragma HLS reset variable=output 
#pragma HLS interface ap_fifo port=allchunk 
#pragma HLS interface ap_fifo port=output 


  out_stream_data result; 
  std::cout<<""<<std::endl;
  std::cout<<"------------Executing TOP-ZS function------------"<<std::endl; 
  
  uint64_t last_triggerstored;
  uint64_t trigstart;
  int8 channel;
  uint64_t TrigNum;
  uint16_t ADCoffset;
  uint64_t EvtwdTag;
  uint64_t EvtMode, mode;
  int8 DRingMarker;
  uint16_t beef;
  uint16_t TrigLen;
  uint64_t ntriggers_chunk;

  uint64_t realADC_hardtrig;
  uint64_t chunkstart;

  int16_t ADC[chunk];
  //gradient vector 
  int16_t gradient[chunk]; 
  //averaged gradient vector 
  double avgradient[chunk]; 
  //time in clocks ticks for each raw ADC values 
  double time[chunk]; 
  //averaged time for each averaged gradient 
  double avADCtime[chunk]; 
  //vector with each trigger time in clock ticks 
  uint64_t trigger_vect[chunk]; 
  //vector with each trigger in absolute clock ticks ie starting each chunk in ADC=0 
  uint64_t trigger_abs[chunk];

  //vector to store the length of each trigger in spill mode to form headers, size=1 for Gap mode
  uint64_t trigger_len[nADC_triggerSPILL];
  //vector to store the trigger number in spill mode to form headers, size=1 for Gap mode
  uint64_t trigger_num[nADC_triggerSPILL];

  uint64_t headersize = infifo_hdr_Len;
  uint64_t dataindex = 0;
  uint64_t datawritten = 0;

 
  //Read header
  uint64_t WORD0[4];
  WORD0[0] = (uint64_t) allchunk[infifo_hdr_lastZStrigstored1] << 0  & (uint64_t) 0x000000000000FFFF;
  WORD0[1] = (uint64_t) allchunk[infifo_hdr_lastZStrigstored2] << 16 & (uint64_t) 0x00000000FFFF0000;
  WORD0[2] = (uint64_t) allchunk[infifo_hdr_lastZStrigstored3] << 32 & (uint64_t) 0x0000FFFF00000000;
  WORD0[3] = (uint64_t) allchunk[infifo_hdr_lastZStrigstored4] << 48 & (uint64_t) 0xFFFF000000000000;
  last_triggerstored = (uint64_t)(WORD0[3] | WORD0[2] | WORD0[1] | WORD0[0]);
  std::cout<<"Check Last ZS trigger from previous chunk: "<<last_triggerstored<<std::endl;

 
  //for(uint64_t k=0; k<ntriggers_chunk; k++){
  uint64_t k=0;
  while(k < ntriggers_chunk){

    std::cout<<"------------TRIG Header: "<<k<<std::endl;
    uint64_t WORD1[4];
    WORD1[0] = (uint64_t) allchunk[infifo_thdr_TrigTime1+dataindex] << 0  & (uint64_t) 0x000000000000FFFF;
    WORD1[1] = (uint64_t) allchunk[infifo_thdr_TrigTime2+dataindex] << 16 & (uint64_t) 0x00000000FFFF0000;
    WORD1[2] = (uint64_t) allchunk[infifo_thdr_TrigTime3+dataindex] << 32 & (uint64_t) 0x0000FFFF00000000;
    WORD1[3] = (uint64_t) allchunk[infifo_thdr_ChTime4+dataindex] << 48   & (uint64_t) 0xFFFF000000000000;
    trigstart =  (uint64_t)(WORD1[3] << 8| WORD1[2] | WORD1[1] | WORD1[0]);
    channel = (int8)(WORD1[3] >> 56);
    std::cout<<"Check Trigger starts in ADC: "<<trigstart<<std::endl;
    std::cout<<"Check Channel: "<<(int16_t)channel<<std::endl;
    //Firs trigger time is chunkstart
    if(k==0){chunkstart = trigstart;}

    uint64_t WORD2[4];
    WORD2[0] = (uint64_t) allchunk[infifo_thdr_TrigNum1+dataindex] << 0    & (uint64_t) 0x000000000000FFFF;
    WORD2[1] = (uint64_t) allchunk[infifo_thdr_TrigNum2+dataindex] << 16   & (uint64_t) 0x00000000FFFF0000;
    WORD2[2] = (uint64_t) allchunk[infifo_thdr_TrigNum3+dataindex] << 32   & (uint64_t) 0x0000FFFF00000000;
    WORD2[3] = (uint64_t) allchunk[infifo_thdr_ADCoffset1+dataindex] << 48 & (uint64_t) 0xFFFF000000000000;
    TrigNum = (uint64_t)(WORD2[2] | WORD2[1] | WORD2[0]);
    ADCoffset = (uint16_t)(WORD2[3] >> 48);
    std::cout<<"Check Trigger number: "<<TrigNum<<std::endl;
    std::cout<<"Check ADCoffset: "<<ADCoffset<<std::endl;

    uint64_t WORD3[4];
    WORD3[0] = (uint64_t) allchunk[infifo_thdr_evtwtag1+dataindex] << 0  & (uint64_t) 0x000000000000FFFF;
    WORD3[1] = (uint64_t) allchunk[infifo_thdr_evtwtag2+dataindex] << 16 & (uint64_t) 0x00000000FFFF0000;
    WORD3[2] = (uint64_t) allchunk[infifo_thdr_evtwtag3+dataindex] << 32 & (uint64_t) 0x0000FFFF00000000;
    WORD3[3] = (uint64_t) allchunk[infifo_thdr_evtmode1+dataindex] << 48 & (uint64_t) 0xFFFF000000000000;

    EvtwdTag = (uint64_t)( WORD3[2] | WORD3[1] | WORD3[0]);

    uint32_t WORD4[2];
    WORD4[0] = (uint32_t) allchunk[infifo_thdr_evtmode2+dataindex] << 0             & (uint32_t) 0x0000FFFF;
    WORD4[1] = (uint32_t) allchunk[infifo_thdr_DRingMarkerevtmode3+dataindex] << 16 & (uint32_t) 0xFFFF0000;
    
    EvtMode  = (uint64_t)(WORD4[1] << 8 | WORD4[0] << 16 | WORD3[3] >> 48);
    DRingMarker = (int8)(WORD4[1] >> 24);
    std::cout<<"Check Event window tag: "<<EvtwdTag<<std::endl;
    std::cout<<"Check Event mode: "<<EvtMode<<std::endl;
    std::cout<<"Check Delivery Ring Marker: "<<(int16_t)DRingMarker<<std::endl;

    uint16_t WORD5;
    WORD5 = allchunk[infifo_thdr_TrigLen1+dataindex];

    TrigLen = WORD5;
    std::cout<<"Check Trigger Length: "<<TrigLen<<std::endl;
    
    uint16_t WORD6[1];
    WORD6[0] = allchunk[infifo_thdr_1+dataindex];
   
    beef = WORD6[0];
    std::cout<<"Check BEEF: "<<std::hex<<beef<<std::dec<<std::endl;

    if(EvtMode==0){
      ntriggers_chunk= ntriggers_chunkSPILL;
      realADC_hardtrig= TrigLen;
    }
    if(EvtMode==1){
      if(ntriggers_chunkGAP<1){ntriggers_chunk= 1;} 
       realADC_hardtrig = nADC_GAPchunk;
    }

    trigger_len[k]=realADC_hardtrig;
    trigger_num[k]=TrigNum;

    headersize = headersize + infifo_thdr_Len;

    std::cout<<"Trigger DATA"<<std::endl;
    //Just read the ADC values, not the ADC=0 
    for (uint64_t i = datawritten; i < (datawritten+realADC_hardtrig); i++) { 
      //Convert allchunk to int16_t
      ADC[i] = allchunk[headersize+i] - 65536;
      //std::cout<<i<<" "<<ADC[i]<<std::endl;
      //trigger time in clock ticks 
      time[i]=i; 
    } 


    datawritten=datawritten+realADC_hardtrig;
    dataindex=dataindex+infifo_thdr_Len+realADC_hardtrig;
    k++;
  }//for //while ntriggers_chunk  

  //Data written is numADCtoZS 
  uint64_t numADCtoZS = datawritten;

  //Calculate the gradient vector 
  for(int32_t i=0;i<(numADCtoZS-window);i++){ 
    gradient[i]=ADC[i+window]-ADC[i]; 
    //cout<<gradient[i]<<endl; 
  } 

  //Calculate the average of the gradient each n_average ADC values to avoid fluctuations 
  int n_average=5; 
  uint64_t j=0; 
  double av_gradient=0; 
  double av_ADCtime=0; 
  uint64_t h=0; 
  uint64_t lasttrig=0; 
 std::cout<<"Average of gradient taken with: "<<n_average<<" values"<<std::endl; 

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
 uint64_t triggercounter=0; 
 uint64_t size_sup; 

 std::cout<<"fadc= "<<fADC<<" tadc= "<<tadc<<" MHz, window: "<<window<<" threshold "<<threshold<<" tbefore: "<<tbefore<<" us ("<<prenumADCstored<<" ADC Counts)"<<" tafter "<<tafter<<" us ("<<postnumADCstored<<" ADC Counts)"<<std::endl; 

 //Store positions in clock ticks for the triggers found, fill trigger_vect[] 
 //h is the number of elements in the averaged gradient vector 
 for(uint64_t i=0;i<h;i++){ 
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
    std::cout<<"absolute time: "<<trigger<<std::endl; 
    std::cout<<"trig: "<<trigger_vect[triggercounter]<<std::endl; 
    //std::cout<<"avgrad: "<<avgradient[i]<<" Trigger number: "<<triggercounter<<": "<<trigger<<" Triggertime: "<<trigger*tadc<<std::endl; 
    triggercounter++; 
  } 
 } 
 std::cout<<"Number of triggers found in chunk: "<< triggercounter<<std::endl; 
 //real number of triggers and vector with triggers that will store data in each chunk 
 uint64_t realtrignum=0;
 uint64_t realtrigger_vect[chunk]; //no more triggers than adc values per chunk 
 uint64_t realtrigger_abs[chunk];
 //For now in the result we are going to store triggers in clock ticks 
 for (uint64_t i = 0; i < triggercounter; i++) { 
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
 std::cout<<"Real number of triggers without repetition: "<<ntriggers<<std::endl;

 //store suppressed data 
 int16_t suppressed_data[chunk]; 
 
 //Suppressed data array initialise to a big number 
 int16_t init=3000; 
 
 //Initialise suppressed array with number init 
 for(int i=0;i<numADCtoZS;i++){suppressed_data[i]=init;} 
 
 for(uint64_t  j=0;j< ntriggers;j++){ 
   int l=(-1)*prenumADCstored; 
   int postnumADCstoredaux = postnumADCstored;
  //Fill trigger positions with -pre and +post adc values 
  while(l<=postnumADCstoredaux){ 
     //Just store the data if the index (realtrigger_abs[j]+k) is higher than the last ADC value stored in previous chunk (last_triggerstored+postnumADCstored) 
    //We have to convert last trigger stored+post to absolute value relative to the start of the chunk 
    double lastvaluestored = last_triggerstored+postnumADCstored; 
    double limit = lastvaluestored-chunkstart; 
    //see that there are prenumADC stored 
    int checkprenum = int(realtrigger_abs[j])+l; 

    //std::cout<<"lastvaluestored "<<lastvaluestored<<" limit "<<limit <<" last_triggerstored "<<last_triggerstored<<" chunkstart "<<chunkstart<<" realtrigger_abs[j]+l "<<realtrigger_abs[j]+l<<" realtrigger_abs[j] "<< realtrigger_abs[j]<<" l "<< l<<" checkprenum "<<checkprenum<<std::endl; 

    if(((realtrigger_abs[j]+l)>limit)&&(checkprenum>=0)){ 
      suppressed_data[realtrigger_abs[j]+l]=ADC[realtrigger_abs[j]+l];} 
    else{cout<<"This value is not stored, index: "<<int(realtrigger_abs[j])+l<<endl;} 
    if(checkprenum<0){std::cout<<"Not prenum ADC values to store in this chunk for this ZS trigger, not all values required are stored for this trigger---wrong energy expected"<<std::endl;} 
    l++;
  } 
 }
 
 //Update last trigger stored in this function call 
 if(lasttrig!=0){  
   last_triggerstored=lasttrig; 
 } 
 std::cout<<"Last trigger stored updated: "<<last_triggerstored<<std::endl; 
 

 //////////////////////////////////////////////////////////////////////////////////////////////

 //for(int i=0;i<numADCtoZS;i++){cout<<i<<" sup dat: "<<suppressed_data[i]<<endl;}

 
 //Form array with data and headers 
 int16_t sendsupdata[chunk];
 //array length to send with headers and suppressed data (headers+supdata) 
 int b=0; 
 //index of just supdata in the array to send (supdata) 
 int i=0;
 uint64_t triglength = trigger_len[0];
 
 std::cout<<"------------OUTPUT FIFO"<<std::endl; 

 mode = EvtMode;
 
 if(mode==0){ 
   //Last ZS trigger found in this chunk is the 4 first elements of the output array 
   sendsupdata[0]=last_triggerstored & 0x0000FFFF; 
   sendsupdata[1]=last_triggerstored >> 16; 
   sendsupdata[2]=last_triggerstored >> 32; 
   sendsupdata[3]=last_triggerstored >> 48; 
   //Mode
   sendsupdata[4]=mode & 0x0000FFFF;
   sendsupdata[5]=mode >> 16;
   sendsupdata[6]=mode >> 32;
   sendsupdata[7]=mode >> 48;
   //Input chunk start in ADC 
   sendsupdata[8]=chunkstart & 0x0000FFFF; 
   sendsupdata[9]=chunkstart >> 16; 
   sendsupdata[10]=chunkstart >> 32; 
   sendsupdata[11]=chunkstart >> 48; 
   b=b+12; 
   //for number of triggers in hardware 
   for(uint64_t j=0;j<ntriggers_chunkSPILL;j++){ 
     int16_t numstart_data=0; 
     int16_t numstart_dataplus=0; 
     int16_t valueswritten=0; 
     int16_t slices=0; 
     bool increasecounter=false; 
     bool datadelay=false; 
     std::cout<<"NEW HARDWARE TRIGGER----------"<<j; 
   
     //for each trigger length 
     while(i<triglength){ 
       cout<<" starting at index: "<<i<<endl; 
       if(suppressed_data[i]!=init){slices++;} 
       //if values stored don't start at the beginning but at some point in the middle of the trigger, remove previous header  
       if(datadelay==true){b=b-7; std::cout<<"Removed previous header"<<std::endl;} 
       while((suppressed_data[i]!=init)&&(i<triglength)){ 
	 //triggernumber 
	 sendsupdata[b]=trigger_num[j] & 0x0000FFFF; 
	 sendsupdata[b+1]=trigger_num[j] >> 16; 
	 sendsupdata[b+2]=trigger_num[j] >> 32; 
	 sendsupdata[b+3]=trigger_num[j] >> 48;
	 //number of slices 
	 sendsupdata[b+4]=slices; 
	 //start of data relative to the hardware trigger 
	 sendsupdata[b+5]=numstart_data; 
	 //number of values written 
	 valueswritten++; 
	 sendsupdata[b+6]=valueswritten; 
	 //suppressed data in this trigger 
	 sendsupdata[b+6+valueswritten]=suppressed_data[i];
	 //cout<<"-stored: "<<i<<" "<<suppressed_data[i]<<"="<<sendsupdata[b+6+valueswritten]<<endl; 
	 i++; 
	 numstart_dataplus++; 
	 increasecounter=true; 
       } 
       if(increasecounter==true){ 
	 std::cout<<"Header: trignum: "<<trigger_num[j]<<" Nslices: "<<sendsupdata[b+4]<<" Start data: "<<sendsupdata[b+5]<<" Number of sup values: "<<sendsupdata[b+6]<<std::endl; 
	 b=b+7+valueswritten; numstart_data=numstart_dataplus;} 
       while((suppressed_data[i]==init)&&(i<triglength)&&(increasecounter==true)){i++;numstart_data++;valueswritten=0;} 
       while((suppressed_data[i]==init)&&(i<triglength)&&(increasecounter==false)){ 
	 //triggernumber 
	 sendsupdata[b]=trigger_num[j] & 0x0000FFFF; 
	 sendsupdata[b+1]=trigger_num[j] >> 16; 
	 sendsupdata[b+2]=trigger_num[j] >> 32; 
	 sendsupdata[b+3]=trigger_num[j] >> 48; 
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
	 std::cout<<"Header: trignum: "<<trigger_num[j]<<" Nslices: "<<sendsupdata[b+4]<<" Start data: "<<sendsupdata[b+5]<<" Number of sup values: "<<sendsupdata[b+6]<<std::endl; 
	 b=b+7;numstart_data=numstart_dataplus;} 
     }//while triglength
     //To not exceed trigger_len[] size
     if(j<(ntriggers_chunkSPILL-1)){triglength= triglength+trigger_len[j+1];}
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
   sendsupdata[4]=mode & 0x0000FFFF;
   sendsupdata[5]=mode >> 16;
   sendsupdata[6]=mode >> 32;
   sendsupdata[7]=mode >> 48;
   //Input chunk start in ADC 
   sendsupdata[8]=chunkstart & 0x0000FFFF; 
   sendsupdata[9]=chunkstart >> 16; 
   sendsupdata[10]=chunkstart >> 32; 
   sendsupdata[11]=chunkstart >> 48; 
   b=b+12;

   //Loop over all suppressed_data array 
   while(i<numADCtoZS){ 
     if(suppressed_data[i]!=init){slices++;} 
     //if values stored don't start at the beginning but at some point in the middle of the trigger, remove previous header 
     if(datadelay==true){b=b-7; std::cout<<"Removed previous header"<<std::endl;} 
     while((suppressed_data[i]!=init)&&(i<numADCtoZS)){
       //triggernumber 
       sendsupdata[b]=trigger_num[j] & 0x0000FFFF; 
       sendsupdata[b+1]=trigger_num[j] >> 16; 
       sendsupdata[b+2]=trigger_num[j] >> 32; 
       sendsupdata[b+3]=trigger_num[j] >> 48;
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
       std::cout<<"Header: trignum: "<<trigger_num[j]<<" Nslices: "<<sendsupdata[b+4]<<" Start data: "<<sendsupdata[b+5]<<" Number of sup values: "<<sendsupdata[b+6]<<std::endl;
       b=b+7+valueswritten;} 
     while((suppressed_data[i]==init)&&(i<numADCtoZS)){ 
       valueswritten=0; 
       if(increasecounter==false){ 
	 //triggernumber
	 sendsupdata[b]=trigger_num[j] & 0x0000FFFF; 
	 sendsupdata[b+1]=trigger_num[j] >> 16; 
	 sendsupdata[b+2]=trigger_num[j] >> 32; 
	 sendsupdata[b+3]=trigger_num[j] >> 48; 
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
   std::cout<<"Header: trignum: "<<trigger_num[j]<<" Nslices: "<<sendsupdata[b+4]<<" Start data: "<<sendsupdata[b+5]<<" Number of sup values: "<<sendsupdata[b+6]<<std::endl; 
   b=b+7; 
 } 
 numstart_data=numstart_dataplus; 
   } 
 } //IF MODE 1
 
 
 //b is the size of the suppressed array (including headers and data) 
 zp_size=b;
 for(int i=0;i<zp_size;i++){ 
  result.data=sendsupdata[i]; 
  output << result; 
  } 

 return;
}
