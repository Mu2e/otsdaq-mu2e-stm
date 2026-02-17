#include "supalg.h"






void supalg (in_stream_t &input, out_stream_t &output, ulongint_data &ntriggers, ulongint_data &last_triggerstored, ulongint_data chunkstart, int16_data ADC_hardtrig[ntriggers_chunk]) {
#pragma HLS reset variable=output
#pragma HLS reset variable=input 

#pragma HLS INTERFACE axis port=input
#pragma HLS INTERFACE axis port=output

  cout<<"Executing top function using chunk: "<<nADC_chunk<<" ADC values"<<endl;
  cout<<"Chunk starts in: "<<chunkstart<<" ADC values"<<endl;
  int16_data ADC[nADC_chunk];//It has to know the size at compile time (#define) for the synthesis process
  //int16_data* ADC=new int16_data[n];
  out_stream_data result;
  

  //gradient vector
  int16_data gradient[nADC_chunk-window];
  //int16_data* gradient=new int16_data[n-window];
  //averaged gradient vector
  double_data avgradient[nADC_chunk-window];
  //double_data* avgradient=new double_data[n-window];
  //time in clocks ticks for each raw ADC value
  double_data time[nADC_chunk];
  //double_data* time=new double_data[n];
  //time in us for each raw ADC value
  double_data timeus[nADC_chunk];
  //double_data* timeus=new double_data[n];
  //averaged time for each averaged gradient
  double_data avADCtime[nADC_chunk-window];
  //double_data* avADCtime=new double_data[n-window];
  //store suppressed data
  int16_data suppressed_data[nADC_chunk];
  //int16_data* suppressed_data=new int16_data[n-window];
  //vector with times for each ADC value suppressed
  double_data suppressed_time[nADC_chunk-window];
  //double_data* suppressed_time=new double_data[n];
  //vector with each trigger time in clock ticks
  ulongint_data trigger_vect[nADC_chunk-window];
  //ulongint_data* trigger_vect=new ulongint_data[n-window];
  //vector with each trigger in absolute clock ticks ie starting each chunk in ADC=0
  ulongint_data trigger_abs[nADC_chunk-window];


  int16_data ADCread;
  
  
  for (unsigned long int i = 0; i < nADC_chunk; i++) {
    ADC[i] = input.read(); //new array structure doesnt work
    //time in microsec
    timeus[i]=i*tadc;
    //trigger time in clock ticks
    time[i]=i;
  }
  

  //Calculate the gradient vector
  for(unsigned long int i=0;i<(nADC_chunk-window);i++){
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
   


  while(j<(nADC_chunk-window)){
    //Each point of the gradient and ADCtime averaged with the (n_average-1) following points, each point is the mean of n_average points of the gradient
    if((j+n_average)>(nADC_chunk-window)){n_average= (nADC_chunk-window)-j;}
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
  int triggercounter=0;
  



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
    if((trigger_vect[i]<=last_triggerstored)||(trigger_vect[i]-prenumADCstored<=last_triggerstored)||(trigger_vect[i]+postnumADCstored>=chunkstart+nADC_chunk)){continue;}
    else{
      lasttrig=trigger_vect[i];
      //fill with real triggers to store data
      realtrigger_vect[realtrignum]=trigger_vect[i];
      realtrigger_abs[realtrignum]=trigger_abs[i];
      

      //comment this to return the suppressed data instead of triggers
      result.time=trigger_vect[i];
      result.data=trigger_vect[i];
      output << result;
      ///////////////////////
      realtrignum++;
    }
  }
  
  //number of triggers without repetition
  ntriggers=realtrignum;
  
  
  //Suppressed data array initialise to a big number
  int16_data init=3000;

  //Initialise suppressed array with a number
  for(int i=0;i<nADC_chunk;i++){suppressed_data[i]=init;}
  
  

  bool print=false;
  for(int j=0;j<realtrignum;j++){
    int k=-prenumADCstored;
    //Fill trigger positions with -pre and +post adc values
    while(k<=postnumADCstored){
      //Just store the data if the index (realtrigger_abs[j]+k) is higher than the last ADC value stored in previous chunk (last_triggerstored+postnumADCstored)
      //We have to convert last trigger stored+post to absolute value relative to the start of the chunk
      double_data lastvaluestored= last_triggerstored+postnumADCstored;
      double_data limit = lastvaluestored-chunkstart;

      if((realtrigger_abs[j]+k)>limit){
	suppressed_data[realtrigger_abs[j]+k]=ADC[realtrigger_abs[j]+k];}
      else{cout<<"This value is not stored, index: "<<realtrigger_abs[j]+k<<", value: "<<suppressed_data[realtrigger_abs[j]+k]<<endl;}
      k++;
    }
    //print=true;
  }
  
  /* if(print==true){
     for(int j=0;j<nADC_chunk;j++){cout<<j<<" "<<suppressed_data[j]<<endl;}
     }*/
 

  //Update last trigger stored in this function call
  last_triggerstored=lasttrig;
  cout<<"Last trigger stored: "<<last_triggerstored<<endl;



  
  //Form array with data and headers
  int16_data sendsupdata[nADC_chunk];
  //array length to send with headers and suppressed data (headers+supdata)
  int b=0;
  //index of just supdata in the array to send (supdata)
  int i=0;
  //length of first trigger
  ulongint_data triglength=ADC_hardtrig[0];
  
  //for number of triggers in hardware
  for(int j=0;j<ntriggers_chunk;j++){
   int numstart_data=0;
   int numstart_dataplus=0;
   int valueswritten=0;
   int slices=0;
   bool increasecounter=false;
   
   cout<<"NEW HARDWARE TRIGGER----------"<<j;
   //for each trigger length
   while(i<triglength){  
     cout<<" starting at index: "<<i<<endl;
     if(suppressed_data[i]!=init){slices++;}
     
     while((suppressed_data[i]!=init)&&(i<triglength)){
       
       //triggernumber
       sendsupdata[b]=j;
       //number of slices
       sendsupdata[b+1]=slices;
       //start of data relative to the hardware trigger
       sendsupdata[b+2]=numstart_data;
       //number of values written
       valueswritten++;
       sendsupdata[b+3]=valueswritten;
       //suppressed data in this trigger
       sendsupdata[b+3+valueswritten]=suppressed_data[i];
       // cout<<"-stored: "<<i<<" "<<suppressed_data[i]<<"="<<sendsupdata[b+3+valueswritten]<<endl;
       i++;
       numstart_dataplus++;
       increasecounter=true;
     }
 
     if(increasecounter==true){
       cout<<"Header: trignum: "<<sendsupdata[b]<<" Nslices: "<<sendsupdata[b+1]<<" Start data: "<<sendsupdata[b+2]<<" Number of sup values: "<<sendsupdata[b+3]<<endl;
       b=b+4+valueswritten;numstart_data=numstart_dataplus;}
     
     while((suppressed_data[i]==init)&&(i<triglength)&&(increasecounter==true)){i++;numstart_data++;valueswritten=0;}
     while((suppressed_data[i]==init)&&(i<triglength)&&(increasecounter==false)){
       //  cout<<"not stored: "<<i<<" "<<suppressed_data[i]<<endl;
       //triggernumber
       sendsupdata[b]=j;
       //number of slices
       sendsupdata[b+1]=slices;
       //start of data relative to the hardware trigger
       sendsupdata[b+2]=numstart_data;
       //number of values written 
       sendsupdata[b+3]=0;
       i++;
       numstart_dataplus++;
       increasecounter=false;}
     
     if(increasecounter==false){
       cout<<"Header: trignum: "<<sendsupdata[b]<<" Nslices: "<<sendsupdata[b+1]<<" Start data: "<<sendsupdata[b+2]<<" Number of sup values: "<<sendsupdata[b+3]<<endl;
	b=b+4;numstart_data=numstart_dataplus;}
     
     

   }//while triglength
   
   triglength= triglength+ADC_hardtrig[j+1];
   
  }//for ntriggers
  


}
