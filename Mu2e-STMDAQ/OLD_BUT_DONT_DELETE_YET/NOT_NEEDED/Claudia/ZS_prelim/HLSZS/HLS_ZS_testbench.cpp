#include "HLS_ZS.h"
//#include <bitset>
#include <limits>


int main () {

  //in_stream_t s_in;
  //OUTPUT FIFO
  out_stream_t s_out;
  uint32_t zp_size=0; 
  uint64_t ntriggers=0;

  /*---CONFIG PARAMETERS---*/
  //INPUT FIFO
  //Choose MODE: 0 for Spill mode (Beam), 1 for the Gap
  uint64_t EvtMode = 0;
  //Choose CHANNEL: 0 for HPGe channel, 1 for LaBr3 channel
  uint8_t channel = 0; //uint8_t just valid for test_bench
  //ADC  Offset: set to 0
  uint16_t ADCoffset = 0;
  /*----------------------*/

  uint64_t mode = EvtMode;

  //Fixed
  uint64_t chunkstart=0;
  uint64_t trigstart=0;
  uint64_t last_triggerstored=0;
  uint64_t TrigNum=0;
  uint64_t EvtwdTag=0;
  uint8_t DRingMarker=0;
  uint16_t TrigLen=0;

  uint64_t totalnumtriggers=0;
  uint64_t numADCtoZS=0;
  uint64_t evttag_counter=0;

  uint64_t ADC_hardtrig;
  uint64_t ntriggers_chunk;


  if(mode==0){
    std::cout<<"Size of the chunk in ADC values (headers+ADC+ADC=0): "<<chunk<<std::endl;
    std::cout<<"Number of triggers per chunk to send: "<<ntriggers_chunkSPILL<<std::endl;  
    std::cout<<"Number of ADC values per trigger: "<<nADC_triggerSPILL<<std::endl;
  
    ADC_hardtrig = nADC_triggerSPILL;
    TrigLen = nADC_triggerSPILL;
    ntriggers_chunk = ntriggers_chunkSPILL;
  }

  if(mode==1){
    std::cout<<"Size of the chunk in ADC values (headers+ADC+ADC=0): "<<chunk<<std::endl;
    std::cout<<"Number of ADC values per trigger: "<<nADC_triggerGAP<<std::endl;
    std::cout<<"Number of ADC values per chunk: "<<nADC_GAPchunk<<std::endl;

    ADC_hardtrig = nADC_GAPchunk;
    TrigLen = nADC_triggerGAP;
    if(ntriggers_chunkGAP<1){ //gap triggers go over different chunks
      ntriggers_chunk = 1;
    }
  }
  
  uint64_t auxchunk = chunk;
  //Check that headers+data fit in chunk size
  if((infifo_hdr_Len+ntriggers_chunk*(infifo_thdr_Len+ADC_hardtrig))>auxchunk){std::cout<<"Length of data written bigger than input FIFO size----EXIT----"<<std::endl;exit(0);}


  //Percentage where the overlapped chunk has to start at least
  double overlapped_chunk = double((nADC_SPILLchunk-1)-postnumADCstored+1-prenumADCstored)/nADC_SPILLchunk;
  std::cout<<"Percentage of overlapped chunk: "<<overlapped_chunk<<std::endl;

  //In reality we have more data due to overlapping chunks so:
  uint64_t nchunks=500000;

  std::cout<<"Number of chunks with overlapping chunks: "<<nchunks<<std::endl;

  //The next chunk starts in trigger:
  double next_chunk_start = nADC_SPILLchunk*overlapped_chunk;
  std::cout<<"Start of overlapping pulse with respect to the previous one: "<<overlapped_chunk<<" Next chunk start in: "<<next_chunk_start<<std::endl;
  
  //Find the start of next trigger closer and below next_trigger_aux for Spill mode
  uint64_t next_trigger_aux=0;
  uint64_t next_trigger_start=0;
  uint64_t next_trigger_startTrigNum=0;
  uint64_t next_chunk_startGAP=0;
  
  if(mode==0){
  for(int i=0;i<ntriggers_chunkSPILL;i++){
    next_trigger_aux = next_trigger_aux+ADC_hardtrig;
    //cout<<next_trigger_aux<<" "<<next_trigger_start<<endl;
    if(next_trigger_aux<next_chunk_start){next_trigger_start=next_trigger_aux;}
  }
  next_trigger_startTrigNum=next_trigger_start/ADC_hardtrig; //divide by trigger size  
  std::cout<<"From "<<ntriggers_chunkSPILL<<" triggers in the chunk, the next chunk starts in ADC value:  "<<next_trigger_start<<" which is trigger number: "<<next_trigger_startTrigNum<<std::endl;
  }

  uint64_t nchunkspertriggerGAP=1000; //intitialise to a value !=0
  uint64_t endchunk=0;
  uint64_t startchunk=0;
  uint64_t numelements_lasttrigGAP;

  if(mode==1){
    nchunkspertriggerGAP=0;
    next_chunk_startGAP= next_chunk_start;
    std::cout<<"The next chunk starts in ADC value: "<<next_chunk_startGAP<<std::endl;
     
    //Number of Chunks per trigger in Gap
    while(endchunk<nADC_triggerGAP){
      endchunk=startchunk+(nADC_GAPchunk-1);
      nchunkspertriggerGAP++;
      std::cout<<"Start of chunk number: "<<nchunkspertriggerGAP<<": "<<startchunk<<", end chunk: "<<endchunk<<std::endl;
      numelements_lasttrigGAP=nADC_triggerGAP-startchunk;
      startchunk=startchunk+next_chunk_startGAP;}
    std::cout<<"One trigger ("<<nADC_triggerGAP<<" ADC values) extends over "<<nchunkspertriggerGAP<<" chunks"<<std::endl;
    std::cout<<"Number of elements to write in the last chunk of the trigger "<<numelements_lasttrigGAP<<std::endl;
  } 

  //Through warning
  if(prenumADCstored>((nADC_SPILLchunk*(1-overlapped_chunk))-(postnumADCstored-1))){std::cout<<"WARNING::::::: In the limit case where trigger is in (chunk-post-1), this trigger would be stored in next chunk but pre adc values chosen exceeds the start of the next chunk, PRE values stored must be lower::::::: ERROR"<<std::endl;exit(1);}
  

  //input binary file
  uint16_t* nADC = new uint16_t[n];
  uint16_t allchunk[chunk];
  
  //FILE *inFILE = fopen("run00109.new.bin_00", "rb");
  FILE *inFILE = fopen("genData662keV_30kHz.bin", "rb"); 
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
  uint32_t auxtrig=0;

  for(uint64_t i=0;i<nchunks;i++){
    
    //test
    //if(i==12){exit(0);}
   
    trigstart = chunkstart;    

    //increase trigger number for Gap mode last chunk of each trigger
    if(mode==1){
      if(auxtrig==nchunkspertriggerGAP){ std::cout<<"INCREASE TRIGGER COUNTER FOR GAP"<<std::endl;
        TrigNum++;
	EvtwdTag++;
        chunkstart=TrigNum*nADC_triggerGAP;
	auxtrig=0;}
      if(auxtrig==(nchunkspertriggerGAP-1)){numADCtoZS=numelements_lasttrigGAP;
      }
      else{numADCtoZS=ADC_hardtrig;}
    }

    if(mode==0){numADCtoZS=ADC_hardtrig;}

    std::cout<<""<<endl;
    std::cout<<"CHUNK NUMBER: "<<i<<std::endl;
    std:cout<<"Chunk goes from ADC index: "<<chunkstart<<" to "<<chunkstart+(ntriggers_chunk*ADC_hardtrig)-1<<std::endl;
    //outFILE<<"Chunk goes from: "<<setprecision(9)<<chunkstart<<" to "<<setprecision(9)<<chunkstart+(nADC_chunk-1)<<endl;    

   
    uint64_t aux=chunkstart+infifo_hdr_Len+ntriggers_chunk*(infifo_thdr_Len+numADCtoZS);
    if(aux>=n){std::cout<<"index exceeds array size"<<std::endl; break;}


    //Form FIFO structure
    //Fill array with header
    std::cout<<"------------INPUT FIFO"<<std::endl;
       
    uint64_t dataindex = 0;

    //Last trigger stored
    uint64_t WORD0 = last_triggerstored;
    allchunk[infifo_hdr_lastZStrigstored1+dataindex]= WORD0 & 0x0000FFFF;
    allchunk[infifo_hdr_lastZStrigstored2+dataindex]= WORD0 >> 16; 
    allchunk[infifo_hdr_lastZStrigstored3+dataindex]= WORD0 >> 32;
    allchunk[infifo_hdr_lastZStrigstored4+dataindex]= WORD0 >> 48;
    std::cout<<"Last ZS trigger from previous chunk: "<<last_triggerstored<<std::endl;
   
    uint64_t datawritten = infifo_hdr_Len;

    for(uint32_t k=0; k<ntriggers_chunk; k++){

      if(mode==0){TrigNum = trigstart/ADC_hardtrig; EvtwdTag = evttag_counter;}

      std::cout<<"------------TRIG Header: "<<k<<std::endl;      
      uint64_t WORD1 = (uint64_t) channel << 56 | (uint64_t) trigstart;
      allchunk[infifo_thdr_TrigTime1+dataindex]= WORD1 & 0x0000FFFF;
      allchunk[infifo_thdr_TrigTime2+dataindex]= WORD1 >> 16;
      allchunk[infifo_thdr_TrigTime3+dataindex]= WORD1 >> 32;
      allchunk[infifo_thdr_ChTime4+dataindex]= WORD1 >> 48;
      std::cout<<"Trigger starts in ADC: "<<trigstart<<std::endl;    
      std::cout<<"Channel: "<<(uint16_t)channel<<std::endl;
      
      uint64_t WORD2 = (uint64_t) ADCoffset << 48 | (uint64_t) TrigNum;
      allchunk[infifo_thdr_TrigNum1+dataindex]= WORD2 & 0x0000FFFF;
      allchunk[infifo_thdr_TrigNum2+dataindex]= WORD2 >> 16;
      allchunk[infifo_thdr_TrigNum3+dataindex]= WORD2 >> 32;
      allchunk[infifo_thdr_ADCoffset1+dataindex]= WORD2 >> 48;
      std::cout<<"Trigger number: "<<TrigNum<<std::endl;
      std::cout<<"ADC Offset: "<<ADCoffset<<std::endl;
      
      uint64_t WORD3 = (uint64_t) EvtMode << 48 | (uint64_t) EvtwdTag; 
      allchunk[infifo_thdr_evtwtag1+dataindex]= WORD3 & 0x0000FFFF;
      allchunk[infifo_thdr_evtwtag2+dataindex]= WORD3 >> 16;
      allchunk[infifo_thdr_evtwtag3+dataindex]= WORD3 >> 32;
      allchunk[infifo_thdr_evtmode1+dataindex]= WORD3 >> 48;
      
      uint32_t WORD4 = (uint32_t) DRingMarker << 24 | (uint32_t) EvtMode >> 16; 
      allchunk[infifo_thdr_evtmode2+dataindex]= WORD4 & 0x0000FFFF;
      allchunk[infifo_thdr_DRingMarkerevtmode3+dataindex]= WORD4 >> 16;
      std::cout<<"Event window tag: "<<EvtwdTag<<std::endl;
      std::cout<<"Event mode: "<<EvtMode<<std::endl;
      std::cout<<"Delivery Ring Marker: "<<(uint16_t)DRingMarker<<std::endl;

      allchunk[infifo_thdr_TrigLen1+dataindex]= TrigLen;
      std::cout<<"Trigger Length: "<<TrigLen<<std::endl;

      uint8_t W0[2] = {0xBE,0xEF};
      uint16_t WORD5 =  (uint16_t)W0[0] << 8 | (uint16_t) W0[1];
      allchunk[infifo_thdr_1+dataindex]= WORD5;
      std::cout<<"BEEF: "<<std::hex<<allchunk[infifo_thdr_1]<<std::dec<<std::endl;

      datawritten=datawritten+infifo_thdr_Len;
      
      //ADC values
      for (uint64_t j = datawritten; j < (datawritten+numADCtoZS); j++) {
	allchunk[j]=nADC[trigstart+j-datawritten];
	//std::cout<<j<<" "<<allchunk[j]<<std::endl;
      }
      
      std::cout<<"CHECK data to suppress: "<<numADCtoZS<<std::endl; 
      trigstart=trigstart+numADCtoZS;
      datawritten=datawritten+numADCtoZS;
      dataindex=dataindex+infifo_thdr_Len+numADCtoZS;
	
    }//for ntriggers_chunk

    uint64_t rest0ADC = auxchunk - datawritten;
    std::cout<<"chunk: "<<auxchunk<<" datawritten: "<<datawritten<<" rest0ADC: "<<rest0ADC<<std::endl;
    if(rest0ADC!=0){
      //rest of ADC values=0
      for (uint64_t j = datawritten; j < auxchunk; j++) {
	allchunk[j]=0;
	//std::cout<<j<<" "<<allchunk[j]<<std::endl;
      }
    }
    

    //Call suppression algorithm
    HLS_ZS(allchunk,s_out,ntriggers,zp_size);

    std::cout<<"Zero suppressed file size in ADC Counts: "<<zp_size<<std::endl;

    std::cout<<"Number of triggers returned: "<<ntriggers<<std::endl;
    
    //number of triggers
    out_stream_data result;

    //Read and reconstruct last trigger stored
    int16_t alldata[zp_size];

    outFILE<<"NEW CHUNK "<<i<<endl;

    for (uint j = 0; j < 4; j++) {
      result =s_out.read();
      alldata[j]= result.data;
      outFILE<<"index: "<<j<<", "<<alldata[j]<<endl;
      //bitset<16> x0(alldata[j]);
    }

    fwrite(&alldata[0], sizeof(int16_t), 4, outBINFILE);

    uint64_t lasttrigstored3 = (uint64_t) alldata[0] << 0  & (uint64_t) 0x000000000000FFFF;
    uint64_t lasttrigstored2 = (uint64_t) alldata[1] << 16 & (uint64_t) 0x00000000FFFF0000;
    uint64_t lasttrigstored1 = (uint64_t) alldata[2] << 32 & (uint64_t) 0x0000FFFF00000000;
    uint64_t lasttrigstored0 = (uint64_t) alldata[3] << 48 & (uint64_t) 0xFFFF000000000000;
    last_triggerstored = (uint64_t)(lasttrigstored0 | lasttrigstored1 | lasttrigstored2 | lasttrigstored3);
    std::cout<<"Received last trigger stored: "<<last_triggerstored<<std::endl;

    //Read rest of header and data
    for (uint j = 4; j < zp_size; j++) {
      result =s_out.read();    
      int16_t data_zp= result.data;
      alldata[j]= data_zp;
      outFILE<<"index: "<<j<<", "<<data_zp<<endl; 
      //outFILE<<"Trigger time "<<setprecision(9)<<time_zp<<endl;
    }

    uint32_t numelements = zp_size-4;
    fwrite(&alldata[4], sizeof(int16_t), numelements, outBINFILE);


    totalnumtriggers +=ntriggers;
    std::cout<<"Total number of triggers ZS: "<<totalnumtriggers<<std::endl;
    
    //Spill
    if(mode==0){
      chunkstart=chunkstart+next_trigger_start;
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
