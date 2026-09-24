#include <iostream>
#include <string>
#include <cstring>
#include <fstream>
#include <vector>
#include <utility> // std::pair
#include <stdexcept> // std::runtime_error
#include <sstream> // std::stringstream
#include<fstream>
#include <sys/stat.h>

#include "TH1F.h"
#include "TH2F.h"
#include "TGraph.h"
#include "TF1.h"
#include "TGraph2D.h"
#include "TLegend.h"
#include "TLine.h"
#include "TCanvas.h"
#include "TROOT.h" 
#include "TStyle.h"
#include "TAxis.h"
#include "TLatex.h"
#include "TProfile2D.h"
#include "TPaletteAxis.h"
#include "TColor.h"
#include "TGraphErrors.h"


void RemoveHeaders_Binary(){
  //char file_name[]="/data1/cgarcia/DATA/Claudia/HLSData/SoftwareClass_2048ADCperRAM/Spill/output.bin";
  //char file_name[]="/data1/cgarcia/DATA/Claudia/HLSData/SoftwareClass_2048ADCperRAM/Gap/output.bin";
 
  //char file_name[]="/data1/cgarcia/DATA/Claudia/HLSData/NewTriggerHeaders/SpillMode5RAMs2048ADC_recentversion/Spill/output.bin";
  //char file_name[]="/data1/cgarcia/DATA/Claudia/HLSData/NewTriggerHeaders/SpillMode5RAMs2048ADC_recentversion/Gap/output.bin"; 
  char file_name[]="/data1/cgarcia/DATA/Claudia/HLSData/NewTriggerHeaders/SpillMode5RAMs2048ADC_recentversion/30kHzSim/Spill/output.bin";

  FILE *myFile = fopen(file_name, "rb");
  if (myFile==NULL)
    exit(1);

  //Create a binary file just with suppressed data
  //string outfile ="/data1/cgarcia/DATA/Claudia/HLSData/SoftwareClass_2048ADCperRAM/Spill/ZPSpilldata_noheaders_run109_00.bin"; 
  //string outfile ="/data1/cgarcia/DATA/Claudia/HLSData/SoftwareClass_2048ADCperRAM/Gap/ZPGapdata_noheaders_run109_00.bin";
  //string outfile ="/data1/cgarcia/DATA/Claudia/HLSData/NewTriggerHeaders/SpillMode5RAMs2048ADC_recentversion/30kHzSim/Spill/outputSpill_noheaders.bin";
  string outfile ="hi.bin";

  int size = outfile.length();
  char outfile_name[size+1];
  //copying the contents of the string to char array
  strcpy(outfile_name, outfile.c_str());
  FILE *myoutFile = fopen(outfile_name, "wb");

  int selectmode=0;



  if(selectmode==0){
    //This will be =1 for each fifo
    //int nchunks=28541;
    //int nchunks=61160;  
    //int nchunks=14270;
    //int nchunks=65656;
    int nchunks=38;
    //Number of ADC values per trigger
    int nADC_trigger=629;
    //Number of triggers per chunk
    //int ntriggers_chunk=32;
    int ntriggers_chunk=15; 
    //int ntriggers_chunk=62;

    uint64_t* last_triggerstored = new uint64_t[1];
    uint64_t* mode = new uint64_t[1];
    uint64_t* chunkstartADC = new uint64_t[1];
    uint64_t* triggernum = new uint64_t[1];
    uint16_t* slicenum = new uint16_t[1];
    uint16_t* startdata = new uint16_t[1];
    uint16_t* numZP = new uint16_t[1];
    int16_t* ZP_data = new int16_t[nADC_trigger];
 
    for(int i=0;i<nchunks;i++){
      std::cout<<"NEW CHUNK "<<i<<std::endl;
      //Last trigger stored
      fread(&last_triggerstored[0], sizeof(uint64_t), 1, myFile);
      std::cout<<"Last trigger stored: "<< std::dec<<last_triggerstored[0]<<std::endl;
      //Mode
      fread(&mode[0], sizeof(uint64_t), 1, myFile);
      std::cout<<"Mode: "<<mode[0]<<std::endl;
      //Chunk start, trigger time
      fread(&chunkstartADC[0], sizeof(uint64_t), 1, myFile);
      std::cout<<"Chunk start/Trigg time (ADC): "<<chunkstartADC[0]<<std::endl;
      for(int j=0;j<ntriggers_chunk;j++){
	//Triggernumber 
	fread(&triggernum[0], sizeof(uint64_t), 1, myFile);
	std::cout<<"Trigger Num: "<< std::dec<<triggernum[0]<<std::endl;
	//Slicenumber
	fread(&slicenum[0], sizeof(uint16_t), 1, myFile);
	std::cout<<"Slice Num: "<< std::dec<<slicenum[0]<<std::endl;
	if(slicenum[0]==2){j=j-1;}
	//start of data relative to the hardware trigger  
	fread(&startdata[0], sizeof(uint16_t), 1, myFile);
	std::cout<<"Start data: "<< std::dec<<startdata[0]<<std::endl;
	//number of values written
	fread(&numZP[0], sizeof(uint16_t), 1, myFile);
	std::cout<<"Number ZP data: "<< std::dec<<numZP[0]<<std::endl;
	//read ZP data in this trigger
	fread(&ZP_data[0], sizeof(int16_t), numZP[0], myFile);
	//write ZP data to a binary file
	fwrite(&ZP_data[0], sizeof(int16_t), numZP[0], myoutFile);
	for(int k=0;k<numZP[0];k++){
	  //std::cout<<"ZP data: "<< std::dec<<ZP_data[k]<<std::endl;
	}

      
      }//j triggers
    }//i nchunks
  }//if mode=0




  if(selectmode==1){
    //This will be =1 for each fifo
    //int nchunks=29020;
    int nchunks=72550;
    //int nchunks=14510;
    //Number of ADC values per trigger
    int nADC_trigger=37000;
   
    bool read_slices = true;

    uint64_t* last_triggerstored = new uint64_t[1];
    uint64_t* mode = new uint64_t[1];
    uint64_t* chunkstartADC = new uint64_t[1];
    uint64_t* triggernum = new uint64_t[1];
    uint16_t* slicenum = new uint16_t[1];
    uint16_t* startdata = new uint16_t[1];
    uint16_t* numZP = new uint16_t[1];
    int16_t* ZP_data = new int16_t[nADC_trigger];

    for(int i=0;i<nchunks;i++){
      std::cout<<"NEW CHUNK "<<i<<std::endl;
      read_slices = true;


      //Last trigger stored
      fread(&last_triggerstored[0], sizeof(uint64_t), 1, myFile);
      std::cout<<"Last trigger stored: "<< std::dec<<last_triggerstored[0]<<std::endl;
      //Mode
      fread(&mode[0], sizeof(uint64_t), 1, myFile);
      std::cout<<"Mode: "<<mode[0]<<std::endl;
      //Chunk start in ADC
      fread(&chunkstartADC[0], sizeof(uint64_t), 1, myFile);
      std::cout<<"Chunk start/Trigg time (ADC): "<<chunkstartADC[0]<<std::endl;
      //Triggernumber
      fread(&triggernum[0], sizeof(uint64_t), 1, myFile);
      std::cout<<"Trigger Num: "<< std::dec<<triggernum[0]<<std::endl;
      //Slicenumber
      fread(&slicenum[0], sizeof(uint16_t), 1, myFile);
      std::cout<<"Slice Num: "<< std::dec<<slicenum[0]<<std::endl;
      //start of data relative to the hardware trigger
      fread(&startdata[0], sizeof(uint16_t), 1, myFile);
      std::cout<<"Start data: "<< std::dec<<startdata[0]<<std::endl;
      //number of values written
      fread(&numZP[0], sizeof(uint16_t), 1, myFile);
      std::cout<<"Number ZP data: "<< std::dec<<numZP[0]<<std::endl;
      //if slice number=1 read the ZP data
      if(slicenum[0]!=0){
	//read ZP data in this trigger
	fread(&ZP_data[0], sizeof(int16_t), numZP[0], myFile);
	//write ZP data to a binary file
	fwrite(&ZP_data[0], sizeof(int16_t), numZP[0], myoutFile);

	while(read_slices==true){
	  //Jump last trigger ADC stored or trigger number (8 bytes)
	  fseek ( myFile , 8 , SEEK_CUR );
	  //Slicenumber
	  fread(&slicenum[0], sizeof(uint16_t), 1, myFile);

	  std::cout<<"Slice Num read: "<< std::dec<<slicenum[0]<<std::endl;
	  	  
	  if(slicenum[0]==2 || slicenum[0]==3 || slicenum[0]==4){
	    fseek ( myFile , -10 , SEEK_CUR );
	    
	    //Triggernumber
	    fread(&triggernum[0], sizeof(uint64_t), 1, myFile);
	    std::cout<<"Trigger Num: "<< std::dec<<triggernum[0]<<std::endl;
	    //Slicenumber
	    fread(&slicenum[0], sizeof(uint16_t), 1, myFile);
	    std::cout<<"Slice Num: "<< std::dec<<slicenum[0]<<std::endl;
	    //start of data relative to the hardware trigger
	    fread(&startdata[0], sizeof(uint16_t), 1, myFile);
	    std::cout<<"Start data: "<< std::dec<<startdata[0]<<std::endl;
	    //number of values written
	    fread(&numZP[0], sizeof(uint16_t), 1, myFile);
	    std::cout<<"Number ZP data: "<< std::dec<<numZP[0]<<std::endl;

	    //read ZP data in this trigger
	    fread(&ZP_data[0], sizeof(int16_t), numZP[0], myFile);
	    //write ZP data to a binary file
	    fwrite(&ZP_data[0], sizeof(int16_t), numZP[0], myoutFile);

	  }//if

	  else{read_slices=false;
	    std::cout<<"Last slice read rejected, it's the mode"<<std::endl;
	    fseek ( myFile , -10 , SEEK_CUR );
	  }//else

	}//while
      }//slicenum[0]!=0
    }//i nchunks 
  }//if mode=1
 


  fclose(myFile);
  fclose(myoutFile);
}
