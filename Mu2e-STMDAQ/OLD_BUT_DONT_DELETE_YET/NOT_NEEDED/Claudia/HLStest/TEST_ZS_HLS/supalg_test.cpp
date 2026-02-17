#include "supalg.h"
//#include <bitset>
#include <limits>
int main () {

  //in_stream_t s_in;
  out_stream_t s_out;
  int zp_size=0; 

  //Choose MODE: 0 for Spill mode (Beam), 1 for the Gap
  int16_t mode = 0;
  
  unsigned long int ADC_hardtrig;

  if(mode==0){
    cout<<"Size of the chunk in ADC values (headers+ADC+ADC=0): "<<chunk<<endl;
    cout<<"Number of triggers per chunk to send: "<<ntriggers_chunkSPILL<<endl;  
    cout<<"Number of ADC values per trigger: "<<nADC_triggerSPILL<<endl;
    cout<<"Number of ADC values per chunk: "<<nADC_chunk<<endl;
  
    ADC_hardtrig= nADC_triggerSPILL;
  }

  if(mode==1){
    cout<<"Size of the chunk in ADC values (headers+ADC+ADC=0): "<<chunk<<endl;
    cout<<"Number of ADC values per trigger: "<<nADC_triggerGAP<<endl;
    cout<<"Number of ADC values per chunk: "<<nADC_chunk<<endl;

    ADC_hardtrig= nADC_chunk;
  }


  
  //Percentage where the overlapped chunk has to start at least
  double overlapped_chunk = double((nADC_chunk-1)-postnumADCstored+1-prenumADCstored)/nADC_chunk;
  std::cout<<"Percentage of overlapped chunk: "<<overlapped_chunk<<std::endl;

  //In reality we have more data due to overlapping chunks so:
  //Number of chunks
  unsigned long int nchunks=n/nADC_chunk;
  cout<<"Number of chunks without overlapping chunks: "<<nchunks<<endl;
  //nchunks=3;
  nchunks=500000;
  cout<<"Number of chunks with overlapping chunks: "<<nchunks<<endl;



  //The next chunk starts in trigger:
  double next_chunk_start = nADC_chunk*overlapped_chunk;
  cout<<"Start of overlapping pulse with respect to the previous one: "<<overlapped_chunk<<" Next chunk start in: "<<next_chunk_start<<endl;
  
  //Find the start of next trigger closer and below next_trigger_aux for Spill mode
  unsigned long int next_trigger_aux=0;
  unsigned long int next_trigger_start=0;
  unsigned long int next_trigger_startTrigNum=0;
  unsigned long int next_chunk_startGAP=0;
  
  if(mode==0){
  for(int i=0;i<ntriggers_chunkSPILL;i++){
    next_trigger_aux = next_trigger_aux+ADC_hardtrig;
    //cout<<next_trigger_aux<<" "<<next_trigger_start<<endl;
    if(next_trigger_aux<next_chunk_start){next_trigger_start=next_trigger_aux;}
  }
  next_trigger_startTrigNum=next_trigger_start/ADC_hardtrig; //divide by trigger size  
  cout<<"From "<<ntriggers_chunkSPILL<<" triggers in the chunk, the next chunk starts in ADC value:  "<<next_trigger_start<<" which is trigger number: "<<next_trigger_startTrigNum<<endl;
  }

  
  unsigned long int nchunkspertriggerGAP=1000; //intitialise to a value !=0
  unsigned long int endchunk=0;
  unsigned long int startchunk=0;
  unsigned long int numelements_lasttrigGAP;
  if(mode==1){
    nchunkspertriggerGAP=0;
     next_chunk_startGAP= next_chunk_start;
     cout<<"The next chunk starts in ADC value: "<<next_chunk_startGAP<<endl;
     
     //Number of Chunks per trigger in Gap
     while(endchunk<nADC_triggerGAP){
       endchunk=startchunk+(nADC_chunk-1);
       nchunkspertriggerGAP++;
       cout<<"Start of chunk number: "<<nchunkspertriggerGAP<<": "<<startchunk<<", end chunk: "<<endchunk<<endl;
       numelements_lasttrigGAP=nADC_triggerGAP-startchunk;
       startchunk=startchunk+next_chunk_startGAP;}
     std::cout<<"One trigger ("<<nADC_triggerGAP<<" ADC values) extends over "<<nchunkspertriggerGAP<<" chunks"<<std::endl;
     std::cout<<"Number of elements to write in the last chunk of the trigger "<<numelements_lasttrigGAP<<std::endl;
  } 

  //Through warning
  if(prenumADCstored>((nADC_chunk*(1-overlapped_chunk))-(postnumADCstored-1))){std::cout<<"WARNING::::::: In the limit case where trigger is in (chunk-post-1), this trigger would be stored in next chunk but pre adc values chosen exceeds the start of the next chunk, PRE values stored must be lower::::::: ERROR"<<std::endl;exit(1);}


  unsigned long int trignumstart=0;
  unsigned long int chunkstart=0;
  unsigned long int last_triggerstored=0;
  unsigned long int ntriggers=0;
  unsigned long int totalnumtriggers=0;
  int32_t   numADCtoZP=nADC_chunk;
  
  //input binary file
  int16_t* nADC = new int16_t[n];
  int16_t allchunk[chunk];
  
  FILE *inFILE = fopen("run00109.new.bin_00", "rb");
  fread(nADC,n,2,inFILE);
  
  //Output Text file
  ofstream outFILE;
  outFILE.open ("output.dat");
  
  //Output Binary file
  std::string stringfile = "output.bin";
  char file_name[stringfile.size()+1];//as 1 char space for null is also required
  strcpy(file_name, stringfile.c_str());
  FILE *outBINFILE = fopen(file_name, "wb");

  //Each nchunkspertriggerGAP increase the trigger counter for the Gap
  int auxtrig=0;

  for(unsigned long int i=0;i<nchunks;i++){
    //increase trigger number for Gap mode last chunk of each trigger
    if(mode==1){
      if(auxtrig==nchunkspertriggerGAP){ cout<<"INCREASE TRIGGER COUNTER FOR GAP"<<std::endl;
        trignumstart++;
        chunkstart=trignumstart*nADC_triggerGAP;
	auxtrig=0;}
      if(auxtrig==(nchunkspertriggerGAP-1)){numADCtoZP=numelements_lasttrigGAP; 
	//if(numelements_lasttrigGAP>32767){std::cout<<""<<std::endl; std::cout<<"ERROR: The number of elements to zero suppress is: "<<numelements_lasttrigGAP<<" bigger than the int16_t range (variable in the input header, change numADCtoZP to unsigned long int)..."<<std::endl;}
      }
      else{numADCtoZP=nADC_chunk;}
    }
    std::cout<<""<<endl;
    cout<<"CHUNK NUMBER: "<<i<<endl;
    cout<<"Chunk goes from: "<<chunkstart<<" to "<<chunkstart+(nADC_chunk-1)<<endl;
    //outFILE<<"Chunk goes from: "<<setprecision(9)<<chunkstart<<" to "<<setprecision(9)<<chunkstart+(nADC_chunk-1)<<endl;    

   
    unsigned aux=chunkstart+numADCtoZP;
    if(aux>=n){cout<<"index exceeds array size"<<endl; break;}


    //Form FIFO structure
    //Fill array with header
    std::cout<<"------------INPUT FIFO"<<std::endl;
    //Mode
    allchunk[0]=mode;
    std::cout<<"mode "<<mode<<std::endl;
    //Chunk number
    allchunk[1]=i & 0x0000FFFF;
    allchunk[2]=i >> 16;
    allchunk[3]=i >> 32;
    allchunk[4]=i >> 48;
    std::cout<<"Chunk number: "<<i<<std::endl;
    //Chunk start 
    allchunk[5]=chunkstart & 0x0000FFFF;
    allchunk[6]=chunkstart >> 16;
    allchunk[7]=chunkstart >> 32;
    allchunk[8]=chunkstart >> 48;
    std::cout<<"Chunk start in ADC: "<<chunkstart<<std::endl;
    //First trigger number
    allchunk[9]=trignumstart & 0x0000FFFF;
    allchunk[10]=trignumstart >> 16;
    allchunk[11]=trignumstart >> 32;
    allchunk[12]=trignumstart >> 48;
    std::cout<<"1st trigger number in this chunk "<<trignumstart<<std::endl;
    //Last trigger stored
    allchunk[13]=last_triggerstored & 0x0000FFFF;
    allchunk[14]=last_triggerstored >> 16;
    allchunk[15]=last_triggerstored >> 32;
    allchunk[16]=last_triggerstored >> 48;
    std::cout<<"Last ZS trigger from previous chunk: "<<last_triggerstored<<std::endl;
    //Number of ADC values to ZP in this chunk 
    allchunk[17]=numADCtoZP & 0x0000FFFF;
    allchunk[18]=numADCtoZP >> 16;
    std::cout<<"Number of ADC values to ZS in this chunk: "<<numADCtoZP<<std::endl;



      //nADC_chunk ADC values
      for (unsigned j = headersize; j < (numADCtoZP+headersize); j++) {
	allchunk[j]=nADC[chunkstart+j-headersize];
	//cout<<j<<" data "<<allchunk[j]<<std::endl;
      }

      //rest of ADC values=0
      for (unsigned j = (numADCtoZP+headersize); j < chunk; j++) {
	allchunk[j]=0;
	//cout<<j<<" data "<<allchunk[j]<<std::endl;
      }


    //Call suppression algorithm
    supalg(allchunk,s_out,ntriggers,zp_size,ADC_hardtrig);

    std::cout<<"Zero suppressed file size in ADC Counts: "<<zp_size<<std::endl;

    //if(i==8000) {exit(0);}
    cout<<"Number of triggers returned: "<<ntriggers<<endl;
    
    //number of triggers
    out_stream_data result;
    //Read and reconstruct last trigger stored
    int16_t alldata[zp_size];

    outFILE<<"NEW CHUNK "<<i<<endl;
    for (unsigned j = 0; j < 4; j++) {
      result =s_out.read();
      alldata[j]= result.data;
      outFILE<<"index: "<<j<<", "<<alldata[j]<<endl;
      bitset<16> x0(alldata[j]);
    }
    fwrite(&alldata[0], sizeof(int16_t), 4, outBINFILE);


    unsigned long int lasttrigstored3 = (unsigned long int) alldata[0] << 0    & (unsigned long int) 0x000000000000FFFF;
    unsigned long int lasttrigstored2 = (unsigned long int) alldata[1] << 16 & (unsigned long int) 0x00000000FFFF0000;
    unsigned long int lasttrigstored1 = (unsigned long int) alldata[2] << 32 & (unsigned long int) 0x0000FFFF00000000;
    unsigned long int lasttrigstored0 = (unsigned long int) alldata[3] << 48 & (unsigned long int) 0xFFFF000000000000;
    last_triggerstored = (unsigned long int)(lasttrigstored0 | lasttrigstored1 | lasttrigstored2 | lasttrigstored3);
    cout<<"Received last trigger stored: "<<last_triggerstored<<endl;

    //Read rest of header and data
    for (int j = 4; j < zp_size; j++) {
      result =s_out.read();    
      int16_t data_zp= result.data;
      alldata[j]= data_zp;
      outFILE<<"index: "<<j<<", "<<data_zp<<endl; 
      //outFILE<<"Trigger time "<<setprecision(9)<<time_zp<<endl;
    }
    int numelements = zp_size-4;
    fwrite(&alldata[4], sizeof(int16_t), numelements, outBINFILE);



  totalnumtriggers +=ntriggers;
  cout<<"Total number of triggers: "<<totalnumtriggers<<endl;
 
  //Spill
  if(mode==0){
    chunkstart=chunkstart+next_trigger_start;
    trignumstart=trignumstart+next_trigger_startTrigNum;
  }
  //Gap
  if(mode==1){
    chunkstart=chunkstart+next_chunk_startGAP;
    auxtrig++;
    
  }

  
  }//nchunks
  
  fclose(inFILE);
  outFILE.close();
  fclose(outBINFILE);  
  







return 0;
  
  
}
