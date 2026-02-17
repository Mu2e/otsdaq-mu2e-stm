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
#include <boost/chrono.hpp>

#include <chrono>
#include <unistd.h>

#include "STMDAQ-TestBeam/ZS/ZS.hh" 


void CallZS(int argc, char *argv[],std::string filename){


  int headersize=19;
  
  //Choose MODE: 0 for Spill mode (Beam), 1 for the Gap
  int16_t mode = 0;
  unsigned long int ADChardtrig;

  //Instance of the class
  ZS *zp = new ZS();

  //Parameters
  double tadc= zp->tadc_();
  int prenumADCstored= zp->prenumADCstored_(tadc);
  int postnumADCstored= zp->postnumADCstored_(tadc);
  unsigned long int chunk= zp->chunk_();
  unsigned long int ntriggers_chunkSPILL= zp->ntriggers_chunkSPILL_();
  unsigned long int nADC_triggerSPILL= zp->nADC_triggerSPILL_();
  unsigned long int nADC_chunk= zp->nADC_chunk_(nADC_triggerSPILL, ntriggers_chunkSPILL);
  unsigned long int nADC_triggerGAP=  zp->nADC_triggerGAP_();

 
  
  int16_t* nADC = zp->ReadBinaryFile(filename);

  boost::chrono::high_resolution_clock::time_point t1 ;
  boost::chrono::high_resolution_clock::time_point t2 ;  

  
  if(mode==0){
    
    #ifdef PRINT_COUT
    std::cout<<"Size of the chunk in ADC values (headers+ADC+ADC=0): "<<chunk<<std::endl;
    std::cout<<"Number of triggers per chunk to send: "<<ntriggers_chunkSPILL<<std::endl;
    std::cout<<"Number of ADC values per trigger: "<<nADC_triggerSPILL<<std::endl;
    std::cout<<"Number of ADC values per chunk: "<<nADC_chunk<<std::endl;
    #endif

    ADChardtrig= nADC_triggerSPILL;
  }

  if(mode==1){
    
    #ifdef PRINT_COUT
    std::cout<<"Size of the chunk in ADC values (headers+ADC+ADC=0): "<<chunk<<std::endl;
    std::cout<<"Number of ADC values per trigger: "<<nADC_triggerGAP<<std::endl;
    std::cout<<"Number of ADC values per chunk: "<<nADC_chunk<<std::endl;
    #endif

    ADChardtrig= nADC_chunk;
  }

  
  //Percentage where the overlapped chunk has to start at least
  double overlapped_chunk =double((nADC_chunk-1)-postnumADCstored+1-prenumADCstored)/nADC_chunk;
  
  #ifdef PRINT_COUT
  std::cout<<"Percentage of overlapped chunk: "<<overlapped_chunk<<std::endl;
  #endif

  //In reality we have more data due to overlapping chunks so:
  //Number of chunks
  unsigned long int n= zp->Returnfsize()/2; //in ADC
  unsigned long int nchunks=n/nADC_chunk;
  //cout<<"Number of chunks without overlapping chunks: "<<nchunks<<endl;
  //unsigned long int nchunks=3;
  nchunks=500000;
  
  #ifdef PRINT_COUT
  std::cout<<"Number of chunks with overlapping chunks: "<<nchunks<<std::endl;
  #endif


  //The next chunk starts in trigger:
  double next_chunk_start = nADC_chunk*overlapped_chunk;
   
  #ifdef PRINT_COUT
  std::cout<<"Start of overlapping pulse with respect to the previous one: "<<overlapped_chunk<<" Next chunk start in: "<<next_chunk_start<<std::endl;
  #endif

  //Find the start of next trigger closer and below next_trigger_aux for Spill mode
  unsigned long int next_trigger_aux=0;
  unsigned long int next_trigger_start=0;
  unsigned long int next_trigger_startTrigNum=0;
  unsigned long int next_chunk_startGAP=0;

  if(mode==0){
    for(int i=0;i<ntriggers_chunkSPILL;i++){
      next_trigger_aux = next_trigger_aux+ADChardtrig;
      //cout<<next_trigger_aux<<" "<<next_trigger_start<<endl;
      if(next_trigger_aux<next_chunk_start){next_trigger_start=next_trigger_aux;}
    }
    next_trigger_startTrigNum=next_trigger_start/ADChardtrig; //divide by trigger size
  
    #ifdef PRINT_COUT
    std::cout<<"From "<<ntriggers_chunkSPILL<<" triggers in the chunk, the next chunk starts in ADC value:  "<<next_trigger_start<<" which is trigger number: "<<next_trigger_startTrigNum<<std::endl;
    #endif
  }


  unsigned long int nchunkspertriggerGAP=1000; //intitialise to a value !=0
  unsigned long int endchunk=0;
  unsigned long int startchunk=0;
  unsigned long int numelements_lasttrigGAP;
  if(mode==1){
    nchunkspertriggerGAP=0;
    next_chunk_startGAP= next_chunk_start;
    
    #ifdef PRINT_COUT
    std::cout<<"The next chunk starts in ADC value: "<<next_chunk_startGAP<<std::endl;
    #endif

    //Number of Chunks per trigger in Gap
    while(endchunk<nADC_triggerGAP){
      endchunk=startchunk+(nADC_chunk-1);
      nchunkspertriggerGAP++;
      
      #ifdef PRINT_COUT
      std::cout<<"Start of chunk number: "<<nchunkspertriggerGAP<<": "<<startchunk<<", end chunk: "<<endchunk<<std::endl;
      #endif

      numelements_lasttrigGAP=nADC_triggerGAP-startchunk;
      startchunk=startchunk+next_chunk_startGAP;}

    #ifdef PRINT_COUT
    std::cout<<"One trigger ("<<nADC_triggerGAP<<" ADC values) extends over "<<nchunkspertriggerGAP<<" chunks"<<std::endl;
    std::cout<<"Number of elements to write in the last chunk of the trigger "<<numelements_lasttrigGAP<<std::endl;
    #endif
   }

  //Through warning
  if(prenumADCstored>((nADC_chunk*(1-overlapped_chunk))-(postnumADCstored-1))){
  
     #ifdef PRINT_COUT
     std::cout<<"WARNING::::::: In the limit case where trigger is in (chunk-post-1), this trigger would be stored in next chunk but pre adc values chosen exceeds the start of the next chunk, PRE values stored must be lower::::::: ERROR"<<std::endl;
     #endif

   exit(1);
   }


  unsigned long int trignumstart=0;
  unsigned long int chunkstart=0;
  unsigned long int last_triggerstored=0;
  unsigned long int ntriggers=0;
  unsigned long int totalnumtriggers=0;
  int32_t numADCtoZS=nADC_chunk;

  


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

  double durAvg = 0;

  for(int64_t i=0;i<nchunks;i++){
    //increase trigger number for Gap mode last chunk of each trigger
    if(mode==1){
      if(auxtrig==nchunkspertriggerGAP){ 

        #ifdef PRINT_COUT
        std::cout<<"INCREASE TRIGGER COUNTER FOR GAP"<<std::endl;
        #endif

	trignumstart++;
        chunkstart=trignumstart*nADC_triggerGAP;
        auxtrig=0;}
      if(auxtrig==(nchunkspertriggerGAP-1)){numADCtoZS=numelements_lasttrigGAP;
        //if(numelements_lasttrigGAP>32767){std::cout<<""<<std::endl; std::cout<<"ERROR: The number of elements to zero suppress is: "<<numelements_lasttrigGAP<<" bigger than the int16_t range (variable in the input header, change numADCtoZS to unsigned long int)..."<<std::endl;}
      }
      else{numADCtoZS=nADC_chunk;}
    }
    
    #ifdef PRINT_COUT
    std::cout<<""<<endl;
    std::cout<<"CHUNK NUMBER: "<<i<<std::endl;
    std::cout<<"Chunk goes from: "<<chunkstart<<" to "<<chunkstart+(nADC_chunk-1)<<std::endl;
    //outFILE<<"Chunk goes from: "<<setprecision(9)<<chunkstart<<" to "<<setprecision(9)<<chunkstart+(nADC_chunk-1)<<endl;
    #endif    

    unsigned aux=chunkstart+numADCtoZS;
    if(aux>=n){
      
      #ifdef PRINT_COUT
      std::cout<<"index exceeds array size"<<std::endl; 
      #endif

      break;}


    //Form FIFO structure
    //Fill array with header
    #ifdef PRINT_COUT
    std::cout<<"------------INPUT FIFO"<<std::endl;
    #endif

    int16_t* allchunk = zp->Form_InputFIFO(nADC, mode,i, chunkstart, trignumstart, last_triggerstored, numADCtoZS);
        
    auto start = std::chrono::steady_clock::now();

    //Call suppression algorithm   
    zp->ReadInputHeader(allchunk);

    // t1 = boost::chrono::high_resolution_clock::now();
    // std::cout<<"t1: "<<t1<<std::endl;

    zp->supalg(allchunk); //slow thing
    unsigned long int ntriggers =  zp->Return_ZSTriggers();
    zp->ZS_array();

    // t2 = boost::chrono::high_resolution_clock::now();
    // std::cout<<"t2: "<<t2<<std::endl;
    
    //    std::cout<< "Number of elements supressed: "<<numADCtoZS<< " in time " << boost::chrono::duration_cast<boost::chrono::nanoseconds>(t2-t1) << std::endl;

    zp->Form_OutputFIFO();
    int16_t* alldata =  zp->Return_ZSOutputFIFO();

    auto end = std::chrono::steady_clock::now();

    auto diff = end - start;
    // Get time in nanoseconds                                                                            
    double timeNano = std::chrono::duration_cast <std::chrono::nanoseconds> (diff).count();
    //    std::cout << "ZS time = " << timeNano*1e-9 << " seconds" << std::endl;
    durAvg += timeNano;

    int zp_size =  zp->Return_ZSFIFOSize();

    #ifdef PRINT_COUT
    std::cout<<"Zero suppressed file size in ADC Counts: "<<zp_size<<std::endl;
    std::cout<<"Number of triggers returned: "<<ntriggers<<std::endl;
    #endif

    outFILE<<"NEW CHUNK "<<i<<endl;
    
    unsigned long int lasttrigstored3 = (unsigned long int) alldata[0] << 0    & (unsigned long int) 0x000000000000FFFF;
    unsigned long int lasttrigstored2 = (unsigned long int) alldata[1] << 16 & (unsigned long int) 0x00000000FFFF0000;
    unsigned long int lasttrigstored1 = (unsigned long int) alldata[2] << 32 & (unsigned long int) 0x0000FFFF00000000;
    unsigned long int lasttrigstored0 = (unsigned long int) alldata[3] << 48 & (unsigned long int) 0xFFFF000000000000;
    last_triggerstored = (unsigned long int)(lasttrigstored0 | lasttrigstored1 | lasttrigstored2 | lasttrigstored3);
    
    #ifdef PRINT_COUT
    std::cout<<"Received last trigger stored: "<<last_triggerstored<<std::endl;
    #endif

    for (int j = 0; j < zp_size; j++) {
      outFILE<<"index: "<<j<<", "<<alldata[j]<<endl;
    }
    
    fwrite(&alldata[0], sizeof(int16_t), zp_size, outBINFILE);
    

    totalnumtriggers +=ntriggers;
    
    #ifdef PRINT_COUT
    std::cout<<"Total number of triggers: "<<totalnumtriggers<<std::endl;
    #endif

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


  durAvg /= nchunks;
  std::cout << "Average ZS time = " << durAvg*1e-9 << " seconds" << std::endl;

  outFILE.close();
  fclose(outBINFILE);
   
  
}




int main(int argc, char *argv[]){

  //argv[0]=program, argv[1]=filename            
  std::string  input_filename = std::string(argv[1]);
 

  CallZS(argc,argv,input_filename);


  return 0;
}
