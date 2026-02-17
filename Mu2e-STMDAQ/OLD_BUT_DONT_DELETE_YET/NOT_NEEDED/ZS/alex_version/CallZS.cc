#include<iostream>
#include <string>
#include <fstream>
#include <vector>
#include <utility> // std::pair
#include <stdexcept> // std::runtime_error
#include <sstream> // std::stringstream
#include <fstream>
#include <cmath>
#include <sys/stat.h>
#include <cstring>

#include <chrono>
#include <unistd.h>

#include "STMDAQ-TestBeam/ZS/ZS.hh" 

// Instance of the ZS class
ZS zp;

// The binary file size
unsigned long int fsize = 0;

int16_t* nADC;

// Function to read input binary file
int16_t* ReadBinaryFile(std::string filename){

  // Open file
  std::ifstream ipfile;
  ipfile.open(filename, std::ios::in | std::ios::binary);
  ipfile.seekg(0, std::ios::end);
  fsize = ipfile.tellg();
  ipfile.seekg(0, std::ios::beg);

  // Define array
  int16_t* data = new int16_t[fsize];

  // Read file
  ipfile.read( (char*) data, fsize*sizeof(data[0]));

  // Print file info
  std::cout<<"File size: "<<fsize<<" bytes, "<<fsize/2<<" ADC"<<std::endl;

  // Return data
  return data;

}

// Call ZS routine
void CallZS(int argc, char *argv[],std::string filename){

  // Print parameter values
  if (zp.output) zp.print_params();

  //Choose MODE: 0 for Spill mode (Beam), 1 for the Gap
  int16_t mode = 0;

  // Number of ADC values per trigger for a given mode (on/off-spill)
  unsigned long int ADChardtrig;

  // Get parameters
  // ADC sampling time (us)
  double tadc= zp.tadc_();
  // Length of data stored before trigger
  int prenumADCstored= zp.prenumADCstored_(tadc);
  // Length of data stored after trigger
  int postnumADCstored= zp.postnumADCstored_(tadc);
  // Maximum number of ADC values in a chunk
  unsigned long int chunk = zp.chunk_();
  // Number of triggers per chunk in on-spill
  unsigned long int ntriggers_chunkSPILL= zp.ntriggers_chunkSPILL_();
  // Number of ADC values per trigger in on-spill
  unsigned long int nADC_triggerSPILL= zp.nADC_triggerSPILL_();
  // Number of ADC values per chunk in on-spill
  unsigned long int nADC_chunk= zp.nADC_chunk_(nADC_triggerSPILL, ntriggers_chunkSPILL);
  // Number of ADC values in off-spill gap
  unsigned long int nADC_triggerGAP=  zp.nADC_triggerGAP_();

  // Get ADC data from binary file
  nADC = ReadBinaryFile(filename);

  // If on-spill
  if(mode==0){
 
    // If printing info...
    if (zp.output){
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
    if (zp.output){
      // Print for user
      std::cout << "Size of the chunk in ADC values (headers+ADC+ADC=0): "<<chunk << std::endl;
      std::cout << "Number of ADC values per trigger: "<<nADC_triggerGAP << std::endl;
      std::cout << "Number of ADC values per chunk: "<<nADC_chunk << std::endl;
    }

    // The number of ADC values in a trigger is larger than a chunk,
    // so use the number of ADC values in a chunk
    ADChardtrig = nADC_chunk;
 
  }  

  // Find in percentage of the total chunk where the next overlapping chunk will start 
  double overlapped_chunk = double((nADC_chunk-1)-postnumADCstored+1-prenumADCstored)/nADC_chunk;

  // If printing info...
  if (zp.output){
    // Print for user
    std::cout << "Percentage of overlapped chunk: "<<overlapped_chunk << std::endl;
  }

  // In reality we have more data due to overlapping chunks so...
  // Get the data file size
  unsigned long int n = fsize/2; //in ADC
  // Number of chunks = filesize in ADC values / number of ADC values per chunk
  unsigned long int nchunks = n/nADC_chunk;
  nchunks=500000; // Hard-coded!!! WHY????

  // If printing info...
  if (zp.output){
    // Print for user
    std::cout << "Number of chunks with overlapping chunks: "<< nchunks << std::endl;
  }

  // Find in ADC values where the next chunk starts
  double next_chunk_start = nADC_chunk*overlapped_chunk;

  // If printing info...
  if (zp.output){
    // Print for userd
    std::cout << "Start of overlapping pulse with respect to the previous one: "
	      << overlapped_chunk <<" Next chunk start in: "<< next_chunk_start << std::endl;
  }
  
  // The position of the ADC value correspondong to the start of
  // the trigger containing the start of the next overlapping chunk  
  unsigned long int next_trigger_start = 0;

  // The trigger number containing the start of the next overlapping chunk  
  unsigned long int next_trigger_startTrigNum = 0;

  // If on-spill...
  if(mode==0){

    // Find the trigger number in which the next overlapping chunk starts
    next_trigger_startTrigNum = floor((double)next_chunk_start/(double)ADChardtrig);  
    
    // Set the ADC value of the start of trigger containing
    // the start of the next overlapping chunk
    next_trigger_start = next_trigger_startTrigNum*ADChardtrig;
    
    if (zp.output){
      std::cout << "From " << ntriggers_chunkSPILL 
		<< " triggers in the chunk, " 
		<< "the next chunk starts in ADC value: " 
		<< next_trigger_start 
		<<" which is trigger number: " 
		<< next_trigger_startTrigNum 
		<< std::endl;
    }
  }
   
  // The number of ADC values where the next chunk starts
  unsigned long int next_chunk_startGAP = 0;
  // The number of chunks per trigger in off-spill
  unsigned long int nchunkspertriggerGAP = 1000; // intitialise to a value !=0
  // The number of ADC values in the last chunk
  unsigned long int numelements_lasttrigGAP;

  // If off-spill
  if(mode==1){
    
    // Set number of chunks per trigger to zero
    nchunkspertriggerGAP = 0;
    
    // Get in number of ADC values where the next chunk starts
    next_chunk_startGAP = next_chunk_start;
    
    // If printing info...
    if (zp.output){
      // Print for user
      std::cout << "The next chunk starts in ADC value: "
		<< next_chunk_startGAP << std::endl;
    }

    // Find the number of chunks per trigger in double precision
    double chunksPerTrig_dbl = double(nADC_triggerGAP)/double(next_chunk_startGAP);
    // The integer number of chunks per trigger is that number rounded up
    nchunkspertriggerGAP = ceil(chunksPerTrig_dbl);

    // Find the double precision remainder of the portion of the last chunk
    double chunkRemainder = chunksPerTrig_dbl - (nchunkspertriggerGAP-1);
    // Find the integer number of ADC values in the last chunk
    numelements_lasttrigGAP = ceil(next_chunk_start*chunkRemainder);
      
    // If printing info...
    if (zp.output){
      // Print for user
      std::cout << "One trigger (" << nADC_triggerGAP 
		<< " ADC values) extends over " 
		<< nchunkspertriggerGAP << " chunks" 
		<< std::endl;
      std::cout << "Number of elements to write in the last chunk of the trigger "<<numelements_lasttrigGAP << std::endl;
    }
  }
  
  // Through warning
  if(prenumADCstored > ((nADC_chunk * (1 - overlapped_chunk)) - (postnumADCstored-1))){
    
    std::cout << "WARNING::::::: In the limit case where trigger is in (chunk-post-1), this trigger would be stored in next chunk but pre adc values chosen exceeds the start of the next chunk, PRE values stored must be lower::::::: ERROR" << std::endl;
   
    exit(1);
    
  }
  unsigned long int trignumstart=0;
  unsigned long int chunkstart=0;
  unsigned long int last_triggerstored=0;
  unsigned long int ntriggers=0;
  unsigned long int totalnumtriggers=0;
  int32_t numADCtoZS=nADC_chunk;
  
  // Output info file
  std::ofstream outFILE;
  // Open output info file
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
	
	if (zp.output){
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
    
    if (zp.output){
      std::cout << ""<< std::endl;
      std::cout << "CHUNK NUMBER: "<<i << std::endl;
      std::cout << "Chunk goes from: "<<chunkstart<<" to "<<chunkstart+(nADC_chunk-1) << std::endl;
      //outFILE<<"Chunk goes from: "<<setprecision(9)<<chunkstart<<" to "<<setprecision(9)<<chunkstart+(nADC_chunk-1)<<endl;
    }    

    unsigned aux=chunkstart+numADCtoZS;
    if(aux>=n){
   
      if (zp.output){
	std::cout << "index exceeds array size" << std::endl; 
      }

      break;}


    //Form FIFO structure
    //Fill array with header
    if (zp.output){
      std::cout << "------------INPUT FIFO" << std::endl;
    }

    // Form the input FIFO
    int16_t* inputFIFO = zp.Form_InputFIFO(nADC, mode,i, chunkstart, trignumstart, last_triggerstored, numADCtoZS);
     
    auto start = std::chrono::steady_clock::now();

    // Zero-suppress the input FIFO and return the output FIFO
    int16_t* outputFIFO = zp.do_ZS(inputFIFO);

    auto end = std::chrono::steady_clock::now();

    auto diff = end - start;
    // Get time in nanoseconds                                                                            
    double timeNano = std::chrono::duration_cast <std::chrono::nanoseconds> (diff).count();
    //    std::cout << "ZS time = " << timeNano*1e-9 << " seconds" << std::endl;
    durAvg += timeNano;


    // Get the number of triggers in the output FIFO
    unsigned long int ntriggers =  zp.Return_ZSTriggers();

    // Get the output FIFO size
    int zp_size =  zp.Return_ZSFIFOSize();

    if (zp.output){
      std::cout << "Zero suppressed file size in ADC Counts: "<<zp_size << std::endl;
      std::cout << "Number of triggers returned: "<<ntriggers << std::endl;
    }

    outFILE<<"NEW CHUNK "<<i<< std::endl;
 
    unsigned long int lasttrigstored3 = (unsigned long int) outputFIFO[0] << 0    & (unsigned long int) 0x000000000000FFFF;
    unsigned long int lasttrigstored2 = (unsigned long int) outputFIFO[1] << 16 & (unsigned long int) 0x00000000FFFF0000;
    unsigned long int lasttrigstored1 = (unsigned long int) outputFIFO[2] << 32 & (unsigned long int) 0x0000FFFF00000000;
    unsigned long int lasttrigstored0 = (unsigned long int) outputFIFO[3] << 48 & (unsigned long int) 0xFFFF000000000000;
    last_triggerstored = (unsigned long int)(lasttrigstored0 | lasttrigstored1 | lasttrigstored2 | lasttrigstored3);
 
    if (zp.output){
      std::cout << "Received last trigger stored: "<<last_triggerstored << std::endl;
    }

    for (int j = 0; j < zp_size; j++) {
      outFILE<<"index: "<<j<<", "<<outputFIFO[j]<< std::endl;
    }
 
    fwrite(&outputFIFO[0], sizeof(int16_t), zp_size, outBINFILE);
 

    totalnumtriggers +=ntriggers;
 
    if (zp.output){
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

  durAvg /= nchunks;
  std::cout << "Average ZS time = " << durAvg*1e-9 << " seconds" << std::endl;


  // Close the output info file
  outFILE.close();
  // Close the outpu binary file
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
