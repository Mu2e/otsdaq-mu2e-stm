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

#include "ZS.hh" 

//Instance of the class
ZS zsup;

int16_t* nADC;

// Call ZS routine
void CallZS(int argc, char *argv[],std::string filename){

  std::cout << "Inside CallZS" << std::endl;

  zsup.print_params();

  //Choose MODE: 0 for Spill mode (Beam), 1 for the Gap
  int16_t mode = 0;
  unsigned long int ADChardtrig;

  // Get parameters
  // ADC sampling time (us)
  double tadc= zsup.tadc_();
  // Length of data stored before trigger
  int prenumADCstored= zsup.prenumADCstored_(tadc);
  // Length of data stored after trigger
  int postnumADCstored= zsup.postnumADCstored_(tadc);
  // Maximum number of ADC values in a chunk
  unsigned long int chunk= zsup.chunk_();
  // Number of triggers per chunk in on-spill
  unsigned long int ntriggers_chunkSPILL= zsup.ntriggers_chunkSPILL_();
  // Number of ADC values per trigger in on-spill
  unsigned long int nADC_triggerSPILL= zsup.nADC_triggerSPILL_();
  // Number of ADC values per chunk in on-spill
  unsigned long int nADC_chunk= zsup.nADC_chunk_(nADC_triggerSPILL, ntriggers_chunkSPILL);
  // Number of ADC values in off-spill gap
  unsigned long int nADC_triggerGAP=  zsup.nADC_triggerGAP_();

  zsup.print_params();

  exit(0);

  // Get ADC data from binary file
  nADC = zsup.ReadBinaryFile(filename);

  // Get timers
  boost::chrono::high_resolution_clock::time_point t1 ;
  boost::chrono::high_resolution_clock::time_point t2 ;  

  // If on-spill
  if(mode==0){
    
    // If printing info...
    if (zsup.output){
      // Print for user
      std::cout << "Size of the chunk in ADC values (headers+ADC+ADC=0): "<< chunk<< std::endl;
      std::cout << "Number of triggers per chunk to send: "<< ntriggers_chunkSPILL << std::endl;
      std::cout << "Number of ADC values per trigger: "<< nADC_triggerSPILL << std::endl;
      std::cout << "Number of ADC values per chunk: "<< nADC_chunk << std::endl;
    }
    
    // Get the number of ADC values per trigger in on-spill mode
    ADChardtrig = nADC_triggerSPILL;
    
  }

  // If off-spill
  if(mode==1){
    
    // If printing info...
    if (zsup.output){
      // Print for user
      std::cout << "Size of the chunk in ADC values (headers+ADC+ADC=0): "<<chunk << std::endl;
      std::cout << "Number of ADC values per trigger: "<<nADC_triggerGAP << std::endl;
      std::cout << "Number of ADC values per chunk: "<<nADC_chunk << std::endl;
    }

    // The number of ADC values in a trigger is larger than a chunk,
    // so use the number of ADC values in a chunk
    ADChardtrig= nADC_chunk;
    
  }  
  
  // Find in percentage of the total chunk where the next overlapping chunk will start 
  double overlapped_chunk = double((nADC_chunk-1)-postnumADCstored+1-prenumADCstored)/nADC_chunk;
  
  if (zsup.output){
    std::cout << "Percentage of overlapped chunk: "<<overlapped_chunk << std::endl;
  }
  
  // In reality we have more data due to overlapping chunks so...
  // Get the data file size
  unsigned long int n = zsup.Returnfsize()/2; //in ADC
  // Number of chunks = filesize in ADC values / number of ADC values per chunk
  unsigned long int nchunks = n/nADC_chunk;
  nchunks=500000; // Hard-coded!!! WHY????
  
  // If printing info...
  if (zsup.output){
    // Print for user
    std::cout << "Number of chunks with overlapping chunks: "<< nchunks << std::endl;
  }
  
  // Find in ADC values where the next chunk starts
  double next_chunk_start = nADC_chunk*overlapped_chunk;
  
  // If printing info...
  if (zsup.output){
    // Print for userd
    std::cout << "Start of overlapping pulse with respect to the previous one: "
	      << overlapped_chunk <<" Next chunk start in: "<< next_chunk_start << std::endl;
    }

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
    
    if (zsup.output){
      std::cout << "From "<<ntriggers_chunkSPILL<<" triggers in the chunk, the next chunk starts in ADC value:  "<<next_trigger_start<<" which is trigger number: "<<next_trigger_startTrigNum << std::endl;
    }
  }


      unsigned long int nchunkspertriggerGAP=1000; //intitialise to a value !=0
      unsigned long int endchunk=0;
      unsigned long int startchunk=0;
      unsigned long int numelements_lasttrigGAP;
      if(mode==1){
	nchunkspertriggerGAP=0;
	next_chunk_startGAP= next_chunk_start;
    
	if (zsup.output){
	  std::cout << "The next chunk starts in ADC value: "<<next_chunk_startGAP << std::endl;
	}

	//Number of Chunks per trigger in Gap
	while(endchunk<nADC_triggerGAP){
	  endchunk=startchunk+(nADC_chunk-1);
	  nchunkspertriggerGAP++;
      
	  if (zsup.output){
	    std::cout << "Start of chunk number: "<<nchunkspertriggerGAP<<": "<<startchunk<<", end chunk: "<<endchunk << std::endl;
	  }

	  numelements_lasttrigGAP=nADC_triggerGAP-startchunk;
	  startchunk=startchunk+next_chunk_startGAP;}

	if (zsup.output){
	  std::cout << "One trigger ("<<nADC_triggerGAP<<" ADC values) extends over "<<nchunkspertriggerGAP<<" chunks" << std::endl;
	  std::cout << "Number of elements to write in the last chunk of the trigger "<<numelements_lasttrigGAP << std::endl;
	}
      }

      //Through warning
      if(prenumADCstored>((nADC_chunk*(1-overlapped_chunk))-(postnumADCstored-1))){
  
	if (zsup.output){
	  std::cout << "WARNING::::::: In the limit case where trigger is in (chunk-post-1), this trigger would be stored in next chunk but pre adc values chosen exceeds the start of the next chunk, PRE values stored must be lower::::::: ERROR" << std::endl;
	}

	exit(1);
      }


      unsigned long int trignumstart=0;
      unsigned long int chunkstart=0;
      unsigned long int last_triggerstored=0;
      unsigned long int ntriggers=0;
      unsigned long int totalnumtriggers=0;
      int32_t numADCtoZS=nADC_chunk;

  


      //Output Text file
      std::ofstream outFILE;
      outFILE.open ("output.dat");


      //Output Binary file
      std::string stringfile = "output.bin";
      char file_name[stringfile.size()+1];//as 1 char space for null is also required
      strcpy(file_name, stringfile.c_str());
      FILE *outBINFILE = fopen(file_name, "wb");

      //Each nchunkspertriggerGAP increase the trigger counter for the Gap
      int auxtrig=0;

      for(int64_t i=0;i<nchunks;i++){
	//increase trigger number for Gap mode last chunk of each trigger
	if(mode==1){
	  if(auxtrig==nchunkspertriggerGAP){ 

	    if (zsup.output){
	      std::cout << "INCREASE TRIGGER COUNTER FOR GAP" << std::endl;
	    }

	    trignumstart++;
	    chunkstart=trignumstart*nADC_triggerGAP;
	    auxtrig=0;}
	  if(auxtrig==(nchunkspertriggerGAP-1)){numADCtoZS=numelements_lasttrigGAP;
	    //if(numelements_lasttrigGAP>32767){std::cout << "" << std::endl; std::cout << "ERROR: The number of elements to zero suppress is: "<<numelements_lasttrigGAP<<" bigger than the int16_t range (variable in the input header, change numADCtoZS to unsigned long int)..." << std::endl;}
	  }
	  else{numADCtoZS=nADC_chunk;}
	}
    
	if (zsup.output){
	  std::cout << ""<< std::endl;
	  std::cout << "CHUNK NUMBER: "<<i << std::endl;
	  std::cout << "Chunk goes from: "<<chunkstart<<" to "<<chunkstart+(nADC_chunk-1) << std::endl;
	  //outFILE<<"Chunk goes from: "<<setprecision(9)<<chunkstart<<" to "<<setprecision(9)<<chunkstart+(nADC_chunk-1)<<endl;
	}    

	unsigned aux=chunkstart+numADCtoZS;
	if(aux>=n){
      
	  if (zsup.output){
	    std::cout << "index exceeds array size" << std::endl; 
	  }

	  break;}


	//Form FIFO structure
	//Fill array with header
	if (zsup.output){
	  std::cout << "------------INPUT FIFO" << std::endl;
	}

	int16_t* allchunk = zsup.Form_InputFIFO(nADC, mode,i, chunkstart, trignumstart, last_triggerstored, numADCtoZS);
        
	//Call suppression algorithm   
	zsup.ReadInputHeader(allchunk);

	t1 = boost::chrono::high_resolution_clock::now();
	std::cout << "t1: "<<t1 << std::endl;

	zsup.supalg(allchunk); //slow thing
	unsigned long int ntriggers =  zsup.Return_ZSTriggers();
	zsup.ZS_array();

	t2 = boost::chrono::high_resolution_clock::now();
	std::cout << "t2: "<<t2 << std::endl;
    
	std::cout <<  "Number of elements supressed: "<<numADCtoZS<< " in time " << boost::chrono::duration_cast<boost::chrono::nanoseconds>(t2-t1) << std::endl;

	zsup.Form_OutputFIFO();
	int16_t* alldata =  zsup.Return_ZSOutputFIFO();


	int zsup_size =  zsup.Return_ZSFIFOSize();

	if (zsup.output){
	  std::cout << "Zero suppressed file size in ADC Counts: "<<zsup_size << std::endl;
	  std::cout << "Number of triggers returned: "<<ntriggers << std::endl;
	}

	outFILE<<"NEW CHUNK "<<i<< std::endl;
    
	unsigned long int lasttrigstored3 = (unsigned long int) alldata[0] << 0    & (unsigned long int) 0x000000000000FFFF;
	unsigned long int lasttrigstored2 = (unsigned long int) alldata[1] << 16 & (unsigned long int) 0x00000000FFFF0000;
	unsigned long int lasttrigstored1 = (unsigned long int) alldata[2] << 32 & (unsigned long int) 0x0000FFFF00000000;
	unsigned long int lasttrigstored0 = (unsigned long int) alldata[3] << 48 & (unsigned long int) 0xFFFF000000000000;
	last_triggerstored = (unsigned long int)(lasttrigstored0 | lasttrigstored1 | lasttrigstored2 | lasttrigstored3);
    
	if (zsup.output){
	  std::cout << "Received last trigger stored: "<<last_triggerstored << std::endl;
	}

	for (int j = 0; j < zsup_size; j++) {
	  outFILE<<"index: "<<j<<", "<<alldata[j]<< std::endl;
	}
    
	fwrite(&alldata[0], sizeof(int16_t), zsup_size, outBINFILE);
    

	totalnumtriggers +=ntriggers;
    
	if (zsup.output){
	  std::cout << "Total number of triggers: "<<totalnumtriggers << std::endl;
	}

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
   
      outFILE.close();
      fclose(outBINFILE);
   
  
}

// Main function
int main(int argc, char *argv[]){

  // Get input filename as string
  std::string  input_filename = std::string(argv[1]);
 
  // Call zero-suppression routine
  CallZS(argc,argv,input_filename);


  return 0;
}
