#include "supalg.h"


void supalg (int16_data allchunk[chunk], out_stream_t &output, ulongint_data &ntriggers, int &zp_size, ulongint_data ADC_hardtrig) {
#pragma HLS reset variable=output

#pragma HLS interface ap_fifo port=allchunk
#pragma HLS interface ap_fifo port=output
//pragma HLS INTERFACE axis port=output
  out_stream_data result;
 

  cout<<"------------Executing TOP function------------"<<endl;
    /*bitset<16> x0(allchunk[0]);
  bitset<16> x1(allchunk[1]);
  bitset<16> x2(allchunk[2]);
  bitset<16> x3(allchunk[3]);
  std::cout<<"all chunk function: "<< x0<<" "<<x1<<" "<<x2<<" "<<x3 << std::endl;
  */
  //Read header
  int16_t mode;
  ulongint_data chunknum, chunkstart, trignumstart, last_triggerstored;
  int32_t numADCtoZP;

  mode = allchunk[0];
  cout<<"Check Mode: "<<mode<<endl;
  ulongint_data chunknum4 = (ulongint_data) allchunk[1] << 0    & (ulongint_data) 0x000000000000FFFF;
  ulongint_data chunknum3 = (ulongint_data) allchunk[2] << 16 & (ulongint_data) 0x00000000FFFF0000;
  ulongint_data chunknum2 = (ulongint_data) allchunk[3] << 32 & (ulongint_data) 0x0000FFFF00000000;
  ulongint_data chunknum1 = (ulongint_data) allchunk[4] << 48 & (ulongint_data) 0xFFFF000000000000;
  chunknum = (ulongint_data)(chunknum1 | chunknum2 | chunknum3 | chunknum4);
  cout<<"Check Chunk Number: "<<chunknum<<endl;
  ulongint_data chunkstart8 = (ulongint_data) allchunk[5] << 0    & (ulongint_data) 0x000000000000FFFF;
  ulongint_data chunkstart7 = (ulongint_data) allchunk[6] << 16 & (ulongint_data) 0x00000000FFFF0000;
  ulongint_data chunkstart6 = (ulongint_data) allchunk[7] << 32 & (ulongint_data) 0x0000FFFF00000000;
  ulongint_data chunkstart5 = (ulongint_data) allchunk[8] << 48 & (ulongint_data) 0xFFFF000000000000;
  chunkstart = (ulongint_data)(chunkstart5 | chunkstart6 | chunkstart7 | chunkstart8);
  cout<<"Check Chunk starts in: "<<chunkstart<<" ADC values"<<endl;
  ulongint_data trignumstart12 = (ulongint_data) allchunk[9] << 0    & (ulongint_data) 0x000000000000FFFF;
  ulongint_data trignumstart11 = (ulongint_data) allchunk[10] << 16 & (ulongint_data) 0x00000000FFFF0000;
  ulongint_data trignumstart10 = (ulongint_data) allchunk[11] << 32 & (ulongint_data) 0x0000FFFF00000000;
  ulongint_data trignumstart9 = (ulongint_data) allchunk[12] << 48 & (ulongint_data) 0xFFFF000000000000;
  trignumstart = (ulongint_data)(trignumstart9 | trignumstart10 | trignumstart11 | trignumstart12);
  cout<<"Check Trig Num start: "<<trignumstart<<endl;
  ulongint_data lasttrigstored16 = (ulongint_data) allchunk[13] << 0    & (ulongint_data) 0x000000000000FFFF;
  ulongint_data lasttrigstored15 = (ulongint_data) allchunk[14] << 16 & (ulongint_data) 0x00000000FFFF0000;
  ulongint_data lasttrigstored14 = (ulongint_data) allchunk[15] << 32 & (ulongint_data) 0x0000FFFF00000000;
  ulongint_data lasttrigstored13 = (ulongint_data) allchunk[16] << 48 & (ulongint_data) 0xFFFF000000000000;
  last_triggerstored  = (ulongint_data)(lasttrigstored13 | lasttrigstored14 | lasttrigstored15 | lasttrigstored16);
  cout<<"Check last trigger stored: "<<last_triggerstored<<endl;
  int32_t numADCtoZP18= (int32_t) allchunk[17] << 0 & (int32_t) 0x0000FFFF;
  int32_t numADCtoZP17= (int32_t) allchunk[18] << 16 & (int32_t) 0xFFFF0000;
  numADCtoZP  = (int32_t)(numADCtoZP17 | numADCtoZP18 );
  cout<<"Check number of ADC values to ZS in this chunk: "<<numADCtoZP<<endl;

  int16_data ADC[nADC_chunk];//It has to know the size at compile time for the synthesis process

  //gradient vector
  int16_data gradient[nADC_chunk-window];
  //averaged gradient vector
  double_data avgradient[nADC_chunk-window];
  //time in clocks ticks for each raw ADC value
  double_data time[nADC_chunk];
  //averaged time for each averaged gradient
  double_data avADCtime[nADC_chunk-window];
  //vector with each trigger time in clock ticks
  ulongint_data trigger_vect[nADC_chunk];
  //vector with each trigger in absolute clock ticks ie starting each chunk in ADC=0
  ulongint_data trigger_abs[nADC_chunk];

  //Just read the ADC values, not the ADC=0
  for (int32_t i = 0; i < numADCtoZP; i++) {
    ADC[i] = allchunk[headersize+i]; //new array structure doesnt work
    //trigger time in clock ticks
    time[i]=i;
  }
  

  //Calculate the gradient vector
  for(int32_t i=0;i<(numADCtoZP-window);i++){
    gradient[i]=ADC[i+window]-ADC[i];
    //cout<<gradient[i]<<endl;
   }





  //Calculate the average of the gradient each n_average ADC values to avoid fluctuations
  int n_average=5;
  unsigned long int j=0;
  double av_gradient=0;
  double av_ADCtime=0;
  unsigned long int h=0;
  ulongint_data lasttrig=0;
  std::cout<<"Average of gradient taken with: "<<n_average<<" values"<<std::endl;
   


  while(j<(numADCtoZP-window)){
    //Each point of the gradient and ADCtime averaged with the (n_average-1) following points, each point is the mean of n_average points of the gradient
    if((j+n_average)>(numADCtoZP-window)){n_average= (numADCtoZP-window)-j;}
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
  std::cout<<"fadc= "<<fADC<<" tadc= "<<tadc<<" MHz, window: "<<window<<" threshold "<<threshold<<" tbefore: "<<tbefore<<" us ("<<prenumADCstored<<" ADC Counts)"<<" tafter "<<tafter<<" us ("<<postnumADCstored<<" ADC Counts)"<<std::endl;
  
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
       
      cout<<"absolute time: "<<trigger<<endl;
      cout<<"trig: "<<trigger_vect[triggercounter]<<endl;

      //std::cout<<"avgrad"<<avgradient[i]<<" Trigger number: "<<triggercounter<<": "<<trigger<<" Triggertime: "<<trigger*tadc<<std::endl;
      triggercounter++;

    }
  }

  cout<<"Number of triggers found in chunk: "<< triggercounter<<endl;


  //real number of triggers and vector with triggers that will store data in each chunk
  ulongint_data realtrignum=0;
  ulongint_data realtrigger_vect[nADC_chunk]; //no more triggers than adc values per chunk
  ulongint_data realtrigger_abs[nADC_chunk];

  //For now in the result we are going to store triggers in clock ticks
  for (unsigned long int i = 0; i < triggercounter; i++) {
    //if trigger found is little than last trigger stored from previous call, don't store it (give it a margin of window to not store twice the same trigger in different chunks). Also don't store this trigger in this chunk if the number of adc values to store after this trigger exceeds the chunk limit: trigger+postnumADCstored>=chunkstart+nADC_chunk
    //or if new ZP trigger found is closer to the previous one prenumADCstored
    if((trigger_vect[i]<=last_triggerstored)||(trigger_vect[i]-prenumADCstored<=last_triggerstored)||(trigger_vect[i]+postnumADCstored>=chunkstart+numADCtoZP)){continue;}
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

  
  //store suppressed data
  int16_data suppressed_data[nADC_chunk];

  //Suppressed data array initialise to a big number
  int16_data init=3000;

  //Initialise suppressed array with a number
  for(int i=0;i<numADCtoZP;i++){suppressed_data[i]=init;}
  
  

  for(int j=0;j< ntriggers;j++){
    int k=-prenumADCstored;
    //Fill trigger positions with -pre and +post adc values
    while(k<=postnumADCstored){
      //Just store the data if the index (realtrigger_abs[j]+k) is higher than the last ADC value stored in previous chunk (last_triggerstored+postnumADCstored)
      //We have to convert last trigger stored+post to absolute value relative to the start of the chunk
      double_data lastvaluestored= last_triggerstored+postnumADCstored;
      double_data limit = lastvaluestored-chunkstart;
      //see that there are prenumADC stored
      int checkprenum = int(realtrigger_abs[j])+k;
      
      //std::cout<<"lastvaluestored "<<lastvaluestored<<" limit "<<limit <<" last_triggerstored "<<last_triggerstored<<" chunkstart "<<chunkstart<<" realtrigger_abs[j]+k "<<realtrigger_abs[j]+k<<" realtrigger_abs[j] "<< realtrigger_abs[j]<< " k "<< k<<" checkprenum "<<checkprenum<<std::endl; 
      if(((realtrigger_abs[j]+k)>limit)&&(checkprenum>=0)){
	suppressed_data[realtrigger_abs[j]+k]=ADC[realtrigger_abs[j]+k];}
      else{cout<<"This value is not stored, index: "<<int(realtrigger_abs[j])+k<<endl;}
      if(checkprenum<0){std::cout<<"Not prenum ADC values to store in this chunk for this ZP trigger, not all values required are stored for this trigger---wrong energy expected"<<std::endl;}
      k++;
    }
  }
  
 

  //Update last trigger stored in this function call
  if(lasttrig!=0){
    last_triggerstored=lasttrig;
  }
  cout<<"Last trigger stored updated: "<<last_triggerstored<<endl;



  
  //Form array with data and headers
  int16_data sendsupdata[nADC_chunk];
  //array length to send with headers and suppressed data (headers+supdata)
  int b=0;
  //index of just supdata in the array to send (supdata)
  int i=0;
  //length of first trigger
  ulongint_data triglength=ADC_hardtrig;
  
  std::cout<<"------------OUTPUT FIFO"<<std::endl;

  if(mode==0){
    //Last ZP trigger found in this chunk is the 4 first elements of the output array
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
    for(ulongint_data j=0;j<ntriggers_chunkSPILL;j++){

      int16_data numstart_data=0;
      int16_data numstart_dataplus=0;
      int16_data valueswritten=0;
      int16_data slices=0;
      bool increasecounter=false;
      bool datadelay=false;

      cout<<"NEW HARDWARE TRIGGER----------"<<j;
      //for each trigger length
      while(i<triglength){  
	cout<<" starting at index: "<<i<<endl;
	if(suppressed_data[i]!=init){slices++;}
	//if values stored don't start at the beginning but at some point in the middle of the trigger, remove previous header  
	if(datadelay==true){b=b-7; cout<<"Removed previous header"<<std::endl;} 
    
	while((suppressed_data[i]!=init)&&(i<triglength)){
	  //cout<<"b1 "<<b<<endl;
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
       cout<<"Header: trignum: "<<sendsupdata[b]<<" Nslices: "<<sendsupdata[b+4]<<" Start data: "<<sendsupdata[b+5]<<" Number of sup values: "<<sendsupdata[b+6]<<endl;
       b=b+7+valueswritten;numstart_data=numstart_dataplus;}
     
     while((suppressed_data[i]==init)&&(i<triglength)&&(increasecounter==true)){i++;numstart_data++;valueswritten=0;}
     while((suppressed_data[i]==init)&&(i<triglength)&&(increasecounter==false)){
       //cout<<"not stored: "<<i<<" "<<suppressed_data[i]<<endl;
       //cout<<"b2 "<<b<<endl;
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
       cout<<"Header: trignum: "<<sendsupdata[b]<<" Nslices: "<<sendsupdata[b+4]<<" Start data: "<<sendsupdata[b+5]<<" Number of sup values: "<<sendsupdata[b+6]<<endl;
       b=b+7;numstart_data=numstart_dataplus;}
     
     

      }//while triglength
      
      triglength= triglength+ADC_hardtrig;
      
    }//for ntriggers
  }//IF MODE 0






  

  if(mode==1){
    
    int16_data numstart_data=0;
    int16_data numstart_dataplus=0;
    int16_data valueswritten=0; //This could give an error if we write all the trigger (37000 ADC values) because int16 range is 32000 values
    int16_data slices=0;
    bool increasecounter=false;
    bool datadelay=false;

    //Last ZP trigger found in this chunk is the 4 first elements of the output array
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
    while(i<numADCtoZP){

      if(suppressed_data[i]!=init){slices++;}
      //if values stored don't start at the beginning but at some point in the middle of the trigger, remove previous header
      if(datadelay==true){b=b-7; cout<<"Removed previous header"<<std::endl;}

      while((suppressed_data[i]!=init)&&(i<numADCtoZP)){
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
	//if(chunknum==78579){std::cout<<"Values written: "<<valueswritten<<" sup data: "<<sendsupdata[b+6+valueswritten]<<std::endl;}
      }
     
    
    if(increasecounter==true){
      cout<<"Header: trignum: "<<trignumstart<<" Nslices: "<<sendsupdata[b+4]<<" Start data: "<<sendsupdata[b+5]<<" Number of sup values: "<<sendsupdata[b+6]<<endl;
      b=b+7+valueswritten;}

    while((suppressed_data[i]==init)&&(i<numADCtoZP)){
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
      cout<<"Header: trignum: "<<trignumstart<<" Nslices: "<<sendsupdata[b+4]<<" Start data: "<<sendsupdata[b+5]<<" Number of sup values: "<<sendsupdata[b+6]<<endl;
      b=b+7;
    }

    numstart_data=numstart_dataplus;
    }
  }


  
  //b is the size of the suppressed array (including headers and data)       
  zp_size=b;
  for(int i=0;i<zp_size;i++){
    result.data=sendsupdata[i];
    output << result;
  }
  
}
