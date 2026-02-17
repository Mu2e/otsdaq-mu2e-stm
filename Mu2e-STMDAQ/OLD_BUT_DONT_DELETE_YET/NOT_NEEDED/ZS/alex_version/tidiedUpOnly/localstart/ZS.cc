#include <stdlib.h>
#include <stdio.h>
#include <iostream>
#include <fstream>
#include <chrono>

// Zero-suppresion header
#include "ZS.hh" 

// Standard constructor - shouldn't be used
ZS::ZS(){} 

// Function to read input binary file
int16_t* ZS::ReadBinaryFile(std::string filename){

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

// Functon to form input FIFO
int16_t* ZS::Form_InputFIFO(int16_t* nADC, int16_t _mode,
			    unsigned long int _chunknumber,
			    unsigned long int _chunkstart,
			    unsigned long int _trignumstart,
			    unsigned long int _last_triggerstored,
			    int32_t _numADCtoZS){

  // Get maximum ADC values (No. block RAMs * ADCs per block RAM)
  unsigned long int _chunk= chunk_();
  
  // Initialise data array for all chunks
  int16_t*  allchunk = new int16_t[_chunk] ();
  
  // Mode 
  allchunk[0] = _mode; 
  
  // Chunk number 
  allchunk[1] = _chunknumber & 0x0000FFFF; 
  allchunk[2] = _chunknumber >> 16; 
  allchunk[3] = _chunknumber >> 32; 
  allchunk[4] = _chunknumber >> 48; 
 
  // Chunk start 
  allchunk[5] = _chunkstart & 0x0000FFFF; 
  allchunk[6] = _chunkstart >> 16; 
  allchunk[7] = _chunkstart >> 32; 
  allchunk[8] = _chunkstart >> 48; 

  // First trigger number 
  allchunk[9] = _trignumstart & 0x0000FFFF; 
  allchunk[10] = _trignumstart >> 16; 
  allchunk[11] = _trignumstart >> 32; 
  allchunk[12] = _trignumstart >> 48; 

  //Last trigger stored 
  allchunk[13] = _last_triggerstored & 0x0000FFFF; 
  allchunk[14] = _last_triggerstored >> 16; 
  allchunk[15] = _last_triggerstored >> 32; 
  allchunk[16] = _last_triggerstored >> 48; 

  //number of ADC values to ZS in this chunk 
  allchunk[17] = _numADCtoZS & 0x0000FFFF; 
  allchunk[18] = _numADCtoZS >> 16; 

  // If printing info...
  if (output){
    // Print for user
    std::cout<<"mode "<<_mode<<std::endl;
    std::cout<<"Chunk number: "<<_chunknumber<<std::endl;
    std::cout<<"Chunk start in ADC: "<<_chunkstart<<std::endl;
    std::cout<<"1st trigger number in this chunk "<<_trignumstart<<std::endl;
    std::cout<<"Last ZS trigger from previous chunk: "<<_last_triggerstored<<std::endl;
    std::cout<<"Number of ADC values to ZS in this chunk: "<<_numADCtoZS<<std::endl;
  }

  // nADC_chunk ADC values
  // Loop over header + ADC to zero-suppress
  for (unsigned j = headersize; j < (_numADCtoZS+headersize); j++) {
    // Add data to array
    allchunk[j]=nADC[_chunkstart+j-headersize]; 
    //std::cout<<j<<" data "<<inFIFO[j]<<std::endl; 
  }
 
  // Set the rest of ADC values in the chunk array to zero 
  for (unsigned j = (_numADCtoZS+headersize); j < _chunk; j++) { 
    allchunk[j]=0; 
    //std::cout<<j<<" data "<<inFIFO[j]<<std::endl; 
  }
 
  // Return the chunk data array
  return allchunk;
 
} 

// Read the ADC data header
void ZS::ReadInputHeader (int16_t*  allchunk){

  // Print to user 
  std::cout<<"------------Executing TOP function------------"<<std::endl;
  
  // Get the mode
  mode = allchunk[0]; 

  // Get the chunk  number
  unsigned long int chunknum4 = (unsigned long int) allchunk[1] << 0
    & (unsigned long int) 0x000000000000FFFF; 
  unsigned long int chunknum3 = (unsigned long int) allchunk[2] << 16
    & (unsigned long int) 0x00000000FFFF0000; 
  unsigned long int chunknum2 = (unsigned long int) allchunk[3] << 32
    & (unsigned long int) 0x0000FFFF00000000; 
  unsigned long int chunknum1 = (unsigned long int) allchunk[4] << 48
    & (unsigned long int) 0xFFFF000000000000; 
  chunknum = (unsigned long int)(chunknum1 | chunknum2 | chunknum3 | chunknum4); 

  // Get the start of the chunk
  unsigned long int chunkstart8 = (unsigned long int) allchunk[5] << 0
    & (unsigned long int) 0x000000000000FFFF; 
  unsigned long int chunkstart7 = (unsigned long int) allchunk[6] << 16
    & (unsigned long int) 0x00000000FFFF0000; 
  unsigned long int chunkstart6 = (unsigned long int) allchunk[7] << 32
    & (unsigned long int) 0x0000FFFF00000000; 
  unsigned long int chunkstart5 = (unsigned long int) allchunk[8] << 48
    & (unsigned long int) 0xFFFF000000000000; 
  chunkstart = (unsigned long int)(chunkstart5 | chunkstart6 | chunkstart7 | chunkstart8); 

  // Get the starting trigger number
  unsigned long int trignumstart12 = (unsigned long int) allchunk[9] << 0
    & (unsigned long int) 0x000000000000FFFF; 
  unsigned long int trignumstart11 = (unsigned long int) allchunk[10] << 16
    & (unsigned long int) 0x00000000FFFF0000; 
  unsigned long int trignumstart10 = (unsigned long int) allchunk[11] << 32
    & (unsigned long int) 0x0000FFFF00000000; 
  unsigned long int trignumstart9 = (unsigned long int) allchunk[12] << 48
    & (unsigned long int) 0xFFFF000000000000; 
  trignumstart = (unsigned long int)(trignumstart9 | trignumstart10 | trignumstart11 | trignumstart12); 

  // Get the last trigger stored
  unsigned long int lasttrigstored16 = (unsigned long int) allchunk[13] << 0
    & (unsigned long int) 0x000000000000FFFF; 
  unsigned long int lasttrigstored15 = (unsigned long int) allchunk[14] << 16 & (unsigned long int) 0x00000000FFFF0000; 
  unsigned long int lasttrigstored14 = (unsigned long int) allchunk[15] << 32 & (unsigned long int) 0x0000FFFF00000000; 
  unsigned long int lasttrigstored13 = (unsigned long int) allchunk[16] << 48 & (unsigned long int) 0xFFFF000000000000; 
  last_triggerstored  = (unsigned long int)(lasttrigstored13 | lasttrigstored14 | lasttrigstored15 | lasttrigstored16); 

  // Get the number of ADC values to zero-suprress
  int32_t numADCtoZS18= (int32_t) allchunk[17] << 0 & (int32_t) 0x0000FFFF; 
  int32_t numADCtoZS17= (int32_t) allchunk[18] << 16 & (int32_t) 0xFFFF0000;
  numADCtoZS  = (int32_t)(numADCtoZS17 | numADCtoZS18 ); 

  // If printing info...
  if (output){
    // Print for user
    std::cout<<"Check Mode: "<<mode<<std::endl;
    std::cout<<"Check Chunk Number: "<<chunknum<<std::endl;
    std::cout<<"Check Chunk starts in: "<<chunkstart<<" ADC values"<<std::endl;
    std::cout<<"Check Trig Num start: "<<trignumstart<<std::endl;
    std::cout<<"Check last trigger stored: "<<last_triggerstored<<std::endl;
    std::cout<<"Check number of ADC values to ZS in this chunk: "<<numADCtoZS<<std::endl;
  }

  return; 
}

// The zero-suppression algorithm
void ZS::supalg (int16_t* allchunk){
  
  // Parameters 
  tadc= tadc_(); // ADC sampling time
  // Get number ADC values to store before the trigger
  prenumADCstored= prenumADCstored_(tadc);
  // Get number ADC values to store after the trigger
  postnumADCstored= postnumADCstored_(tadc);
  
  //chunk= chunk_();
  // Get the number of triggers per chunk to send in spill
  ntriggers_chunkSPILL= ntriggers_chunkSPILL_();
  // Get the number of ADC values per trigger in spill
  nADC_triggerSPILL= nADC_triggerSPILL_();
  // Get the number of ADC values per chunk according to triggers in spill
  nADC_chunk= nADC_chunk_(nADC_triggerSPILL, ntriggers_chunkSPILL);
  // Get number of ADC values per trigger in gap
  nADC_triggerGAP=  nADC_triggerGAP_();

  // Define ADC array
  ADC=new int16_t[nADC_chunk];
  // Gradient vector 
  int16_t gradient[nADC_chunk-window]; 
  // Averaged gradient vector 
  double avgradient[nADC_chunk-window]; 
  // Time in clocks ticks for each raw ADC values 
  double time[nADC_chunk]; 
  // Averaged time for each averaged gradient 
  double avADCtime[nADC_chunk-window]; 
  // Vector with each trigger time in clock ticks 
  unsigned long int trigger_vect[nADC_chunk]; 
  // Vector with each trigger in absolute clock ticks
  // ie starting each chunk in ADC=0 
  unsigned long int trigger_abs[nADC_chunk]; 

  // Read only ADC values, not the ADC=0 
  for (int32_t i = 0; i < numADCtoZS; i++) { 
    ADC[i] = allchunk[headersize+i]; 
    // Get trigger time in clock ticks 
    time[i] = i; 
  } 

  // Calculate the gradient vector 
  for(int32_t i = 0; i < (numADCtoZS-window); i++){ 
    gradient[i] = ADC[i+window] - ADC[i]; 
    //cout<<gradient[i]<<endl; 
  }
  
  // Calculate the average of the gradient each
  // n_average ADC values to avoid fluctuations 
  int n_average = 5; // Number of points to average
  unsigned long int j = 0; // All points counter
  double av_gradient = 0; // Average gradiet
  double av_ADCtime = 0; // Average time
  unsigned long int h = 0; // Number of elements in averaged gradient vector

  // Initialise last trigger to zero
  lasttrig = 0; 
  
  // If printing info...
  if (output){
    // Print for user
    std::cout << "Average of gradient taken with: "
	      << n_average << " values" << std::endl; 
  }

  // Loop over points to average
  while(j<(numADCtoZS-window)){ 
    
    // Each point of the gradient and ADCtime averaged
    // with the (n_average-1) following points, each
    // point is the mean of n_average points of the gradient     
    if((j+n_average)>(numADCtoZS-window)){
      n_average= (numADCtoZS-window)-j;
    } 
    // Initialise average gradient / time variables
    av_gradient = 0; 
    av_ADCtime = 0; 
    // Loope over all average entries
    for(int k = 0; k < n_average; k++){ 
      // Sum average gradient
      av_gradient += gradient[j+k]; 
      // Sum average time
      av_ADCtime += time[j+k]; 
    } 
    // Calculate average gradient
    avgradient[h] = av_gradient/n_average; 
    // Calculate average time
    avADCtime[h] = av_ADCtime/n_average; 
    // Increase loop counter 
    j += n_average; 
    // Increase average points counter
    h++; 

  }
  
  // Initialise varaibles
  bool peak=false; // Boolean if peak found 
  //  int counter = 0 ; // Counter - DELETE IF NOT USED
  int trigger = 0; // Peak trigger time
  unsigned long int triggercounter = 0; // Trigger counter

  // If printing info...
  if (output){
    // Print for user
    std::cout << "fadc= " << fADC
	      << " tadc= " << tadc
	      << " MHz, window: " << window
	      << " threshold " << threshold
	      << " tbefore: " << tbefore
	      << " us (" << prenumADCstored << " ADC Counts)"
	      << " tafter " << tafter
	      <<" us (" << postnumADCstored << " ADC Counts)"
	      << std::endl; 
  }

  // Store positions in clock ticks for the triggers found, fill trigger_vect[] 
  // Loop over number of elements in the averaged gradient vector (h)
  for(unsigned long int i = 0; i < h; i++){ 
    // If the averaged gradient is beyond the threshold
    if(avgradient[i] > threshold){
      // No peak found
      peak=false;
      continue;
    } 
    // Skip the rest indexes of the peak after the trigger that has already been stored 
    if(avgradient[i] < threshold && peak==true){ 
      continue;
    } 
    // If within the threshold and no peak previously found...
    if(avgradient[i] < threshold && peak==false){ 
      // Peak found!
      peak=true; 
      // store the trigger time
      trigger=avADCtime[i]; 
      // Store trigger trigger in absolute clock ticks                                             
      trigger_abs[triggercounter] = avADCtime[i]; 
      // Store trigger time since the start of the chunk
      trigger_vect[triggercounter] = chunkstart+trigger;
   
      // If printing info...
      if (output){
	// Print for user
	std::cout << "absolute time: " << trigger << std::endl; 
	std::cout << "trig: " << trigger_vect[triggercounter] << std::endl; 
      }

      // Increment the trigger count
      triggercounter++; 
   
    } // End if trigger found 
    
  } // End loop over averages entries

  // If printing info...
  if (output){
    // Print for user
    std::cout<<"Number of triggers found in chunk: "<< triggercounter<<std::endl;
  }

  // Get real number of triggers and vector with
  // triggers that will store data in each chunk 
  unsigned long int realtrignum = 0;

  // No more triggers than adc values per chunk  
  realtrigger_vect = new unsigned long int[nADC_chunk]; 
  realtrigger_abs = new unsigned long int[nADC_chunk];
  
  // Store triggers in clock ticks (for now)
  // Loop over number of triggers found
  for (unsigned long int i = 0; i < triggercounter; i++) { 
    
    // Do not store trigger if:
    // (a) the trigger found is before the last trigger stored from previous call, 
    // don't store it (give it a margin of window to not store twice the same 
    // trigger in different chunks). 
    // (b) the new ZS trigger is closer to the previous one by prenumADCstored.
    // (c) the number of adc values to store after this trigger exceeds the chunk limit.
    if( (trigger_vect[i] <= last_triggerstored) || // (a)
	(trigger_vect[i] - prenumADCstored <= last_triggerstored) || // (b)
	(trigger_vect[i] + postnumADCstored >= chunkstart + numADCtoZS)){ // (c)
      continue;
    } 
    // Else, store trigger
    else{ 
      // Get the last tirgger stored
      lasttrig = trigger_vect[i]; 
      // Fill with real triggers to store data 
      realtrigger_vect[realtrignum] = trigger_vect[i]; 
      realtrigger_abs[realtrignum] = trigger_abs[i]; 
      // Increment stored tirgger number
      realtrignum++; 
    } // End if store trigger
    
  } // End loop over triggers
  
  // Get final umber of triggers without repetition 
  ntriggers = realtrignum;
  
  return; 
} 

// Form zero-suppressed data array
void ZS::ZS_array(){ 
  
  // Initialise zero-suppressed data array
  suppressed_data = new int16_t[nADC_chunk]; 

  // Initialise suppressed data array to a big number 
  init = 3000;  
  for(int i = 0; i < numADCtoZS; i++){
    suppressed_data[i] = init;
  }

  // Loop over number of triggers
  for(unsigned long int j = 0; j < ntriggers; j++){ 

    // Get number ADC values to store before the trigger
    int k = -prenumADCstored; 
    
    // Fill trigger positions with -pre and +post adc values 
    while(k <= postnumADCstored){ 
      
      // Store data if the index (realtrigger_abs[j] + k) is 
      // higher than the last ADC value stored in previous chunk 
      // (last_triggerstored + postnumADCstored). 

      // We have to convert last trigger stored + post to absolute 
      // value relative to the start of the chunk.
      double lastvaluestored = last_triggerstored + postnumADCstored; 
      double limit = lastvaluestored - chunkstart; 

      // See that there are prenumADC stored 
      int checkprenum = int(realtrigger_abs[j])+k; 

      // If within limits to store...
      if((realtrigger_abs[j] + k) > limit && checkprenum >= 0){ 
	// Store suppressed data
	suppressed_data[realtrigger_abs[j]+k] = ADC[realtrigger_abs[j]+k];

      } 
      // Else if not within limits...
      else{

	// If printing info...
	if (output){
	  // Print for user
	  std::cout << "This value is not stored, index: " 
		    << int(realtrigger_abs[j])+k << std::endl;
	}

      }
      
      // If the number of ADC values stored before the trigger is less zero
      if(checkprenum<0){
	// If printing info...
	if (output){
	  // Print for user
	  std::cout << "Not prenum ADC values to store in this chunk for this ZS trigger, not all values required are stored for this trigger ---wrong energy expected" << std::endl;
	}
      } 

      // Increment trigger position index
      k++; 

    } // End loop over trigger positions, k
    
  } // End loop over number of triggers, j

  // Update last trigger stored in this function call 
  if (lasttrig != 0){  
    last_triggerstored = lasttrig; 
  } 

  // If printing info...
  if (output){
    // Print for user
    std::cout << "Last trigger stored updated: " << last_triggerstored << std::endl; 
  }
  
  return; 
  
} 

// Form the output FIFO
void ZS::Form_OutputFIFO() { 

  // Initalise array for suppressed data and headers (output FIFO)
  sendsupdata = new int16_t[nADC_chunk]; 

  // Get the number of ADC values per trigger in spill - DUPLICATED
  unsigned long int ADC_hardtrig = nADC_triggerSPILL; 

  // Array length to send with headers and suppressed data (headers + supdata) 
  int b = 0; 
  // Index of just supdata in the array to send (supdata) 
  int i = 0; 
  // Length of first trigger 
  unsigned long int triglength = ADC_hardtrig; 
 
  // If printing info...
  if (output){
    // Print for user
    std::cout<<"------------OUTPUT FIFO"<<std::endl; 
  }

  // If on-spill
  if(mode == 0){ 
    // Last ZS trigger found in this chunk is the 4 first elements of the output array 
    sendsupdata[0]=last_triggerstored & 0x0000FFFF; 
    sendsupdata[1]=last_triggerstored >> 16; 
    sendsupdata[2]=last_triggerstored >> 32; 
    sendsupdata[3]=last_triggerstored >> 48; 

    //    memcpy(sendsupdata,

    // Mode 
    sendsupdata[4]=mode; 
    // Input chunk number 
    sendsupdata[5]=chunknum & 0x0000FFFF; 
    sendsupdata[6]=chunknum >> 16; 
    sendsupdata[7]=chunknum >> 32; 
    sendsupdata[8]=chunknum >> 48; 
    // Input chunk start in ADC 
    sendsupdata[9]=chunkstart & 0x0000FFFF; 
    sendsupdata[10]=chunkstart >> 16; 
    sendsupdata[11]=chunkstart >> 32; 
    sendsupdata[12]=chunkstart >> 48; 
    // Increase output FIFO array length 
    b += 13; 

    // For number of triggers in hardware 
    for(unsigned long int j = 0; j < ntriggers_chunkSPILL; j++){ 
      
      // Start of data relative to the hardware trigger 
      int16_t numstart_data = 0; 
      int16_t numstart_dataplus = 0; 
      // Number of values written
      int16_t valueswritten = 0; 
      // Number of slices 
      int16_t slices = 0; 
      // Boolean to increase counter
      bool increasecounter = false; 
      // Boolean to remove previous header
      bool datadelay = false; 
   
      // If printing info...
      if (output){
	// Print for user
	std::cout<<"NEW HARDWARE TRIGGER----------"<<j; 
      }
      
      // While inside the trigger length 
      while(i < triglength){

	// If printing info...
	if (output){
	  // Print for user
	  std::cout<<" starting at index: "<<i<<std::endl; 
	}
 
	// If the data != the large initial value
	if(suppressed_data[i] != init){
	  // Increase the number of slices
	  slices++;
	} 

	// If values stored don't start at the beginning 
	// but at some point in the middle of the trigger... 
	if(datadelay == true){
	  // Remove previous header decrease output FIFO array index)
	  b -= 7; 
	  // If printing info...
	  if (output){
	    std::cout << "Removed previous header" << std::endl;
	  }	  
	} 

	// While inside trig length and  data != large initial value
	while(suppressed_data[i] != init && i < triglength){ 
	  
	  // Trigger number 
	  sendsupdata[b]=j & 0x0000FFFF; 
	  sendsupdata[b+1] = j >> 16; 
	  sendsupdata[b+2] = j >> 32; 
	  sendsupdata[b+3] = j >> 48; 
	  // Number of slices 
	  sendsupdata[b+4] = slices; 
	  // Start of data relative to the hardware trigger 
	  sendsupdata[b+5] = numstart_data; 
	  // Number of values written 
	  valueswritten++; 
	  sendsupdata[b+6] = valueswritten; 

	  // Suppressed data in this trigger 
	  sendsupdata[b+6+valueswritten] = suppressed_data[i]; 	  
	  // Incremenet data index
	  i++; 

	  // Incrememnt data counter
	  numstart_dataplus++; 
	  // Set boolean for counter increment to true
	  increasecounter = true; 
	} 

	// If the data counter has been incremented
	if(increasecounter == true){ 

	  // If printing info...
	  if (output){
	    // Print for user
	    std::cout << "Header: trignum: " << sendsupdata[b]
		      << " Nslices: " << sendsupdata[b+4]
		      << " Start data: " << sendsupdata[b+5] 
		      << " Number of sup values: " << sendsupdata[b+6] << std::endl; 
	  }
      
	  // Increase output FIFO array length 
	  b += 7 + valueswritten;
	  
	  // Increase the start index
	  numstart_data = numstart_dataplus;
	}
 
	// WHILE data is inside trig length = large initial value
	// AND counter increase is TRUE
	while(suppressed_data[i] == init && i < triglength && increasecounter == true){
	  // Increment data counter
	  i++;
	  // Incrememnt start of data 
	  numstart_data++;
	  // Set values written to zero
	  valueswritten=0;
	} 
	
	// WHILE data inside trig length = large initial value
	// AND counter increase is FALSE	
	while(suppressed_data[i] == init && i < triglength && increasecounter==false){ 

	  // Trigger number 
	  sendsupdata[b] = j & 0x0000FFFF; 
	  sendsupdata[b+1] = j >> 16; 
	  sendsupdata[b+2] = j >> 32; 
	  sendsupdata[b+3] = j >> 48; 
	  // Number of slices 
	  sendsupdata[b+4] = slices; 
	  // Start of data relative to the hardware trigger 
	  sendsupdata[b+5] = numstart_data; 
	  //number of values written 
	  sendsupdata[b+6] = 0; 

	  // Increment data counter
	  i++; 

	  // Incremement data start index
	  numstart_dataplus++; 

	  // Set increase counter to false - NOT NEEDED
	  increasecounter=false;

	} 
	
	// If increase counter is false
	if(increasecounter == false){ 
	  // Set remove previous header to true
	  datadelay = true; 
	  
	  // If printing info...
	  if (output){
	    // Print for user
	    std::cout << "Header: trignum: " << sendsupdata[b] 
		      <<" Nslices: "<< sendsupdata[b+4] 
		      << " Start data: " << sendsupdata[b+5] 
		      << " Number of sup values: " << sendsupdata[b+6]
		      << std::endl; 
	  }

	  // Increase output FIFO array length 
	  b += 7;
	  // Increase the start index
	  numstart_data = numstart_dataplus;

	} 

      } // End while i < triglength 
      
      // Increase tirgger length by number of ADC values per trigger in spill
      triglength += ADC_hardtrig; 

    } // End loop over triggers

  } // End iF MODE 0 
  
  // If off-spill
  if(mode==1){ 
    
    // Start of data relative to the hardware trigger 
    int16_t numstart_data = 0; 
    int16_t numstart_dataplus = 0; 
    // Number of values written
    // This could give an error if we write all the trigger 
    // (37000 ADC values) because int16 range is 32000 values 
    int16_t valueswritten = 0; 
    // Number of slices 
    int16_t slices = 0; 
    // Boolean to increase counter
    bool increasecounter = false; 
    // Boolean to remove previous header
    bool datadelay = false; 
    
    // Last ZS trigger found in this chunk is the 4 first elements of the output array 
    sendsupdata[0] = last_triggerstored & 0x0000FFFF; 
    sendsupdata[1] = last_triggerstored >> 16; 
    sendsupdata[2] = last_triggerstored >> 32; 
    sendsupdata[3] = last_triggerstored >> 48; 
    // Mode 
    sendsupdata[4] = mode; 
    // Input chunk number 
    sendsupdata[5] = chunknum & 0x0000FFFF; 
    sendsupdata[6] = chunknum >> 16; 
    sendsupdata[7] = chunknum >> 32; 
    sendsupdata[8] = chunknum >> 48; 
    // Input chunk start in ADC 
    sendsupdata[9] = chunkstart & 0x0000FFFF; 
    sendsupdata[10] = chunkstart >> 16; 
    sendsupdata[11] = chunkstart >> 32; 
    sendsupdata[12] = chunkstart >> 48; 
    // Increase output FIFO array length 
    b=b+13; 
    
    // Loop over all suppressed_data array 
    while(i < numADCtoZS){ 

      // If the data != the large initial value
      if(suppressed_data[i] != init){
	// Increase the number of slices
	slices++;
      } 
      
      
      // If values stored don't start at the beginning 
      // but at some point in the middle of the trigger... 
      if(datadelay == true){
	// Remove previous header decrease output FIFO array index)
	b -= 7; 
	// If printing info...
	if (output){
	  std::cout << "Removed previous header" << std::endl;
	}	  
      } 
      
      // WHILE data is inside number to ZS != large initial value      
      while(suppressed_data[i] != init && i < numADCtoZS){ 
	
	// Reconstruct trigger number from input 
	sendsupdata[b] = trignumstart & 0x0000FFFF; 
	sendsupdata[b+1] = trignumstart >> 16; 
	sendsupdata[b+2] = trignumstart >> 32; 
	sendsupdata[b+3] = trignumstart >> 48; 
	// Number of slices 
	sendsupdata[b+4] = slices; 
	// Start of data relative to the hardware trigger 
	sendsupdata[b+5] = numstart_data; 
	// Number of values written 
	valueswritten++; 
	sendsupdata[b+6] = valueswritten; 
	// Suppressed data in this trigger 
	sendsupdata[b+6+valueswritten] = suppressed_data[i]; 

	// Incremement data start index	
	numstart_dataplus++; 
	// Set increase counter to true
	increasecounter = true; 
	// Set remove header to false
	datadelay = false; 
	// Increment data counter
	i++; 
      } 

      // If increase counter
      if(increasecounter == true){
      
	// If printing info...
	if (output){
	  // Print for user
	  std::cout << "Header: trignum: " << trignumstart 
		    << " Nslices: " << sendsupdata[b+4] 
		    << " Start data: " << sendsupdata[b+5] 
		    <<" Number of sup values: " << sendsupdata[b+6] 
		    << std::endl; 
	}

	// Increase output FIFO array length 
	b=b+7+valueswritten;
      }

      // WHILE data is inside number to ZS == large initial value      
      while(suppressed_data[i] == init && i < numADCtoZS){ 
	
	// Set number of values written to zero
	valueswritten=0; 

	// If counter has not been increased
	if(increasecounter==false){ 
	  
	  // Reconstruct trigger number from input 
	  sendsupdata[b] = trignumstart & 0x0000FFFF; 
	  sendsupdata[b+1] = trignumstart >> 16; 
	  sendsupdata[b+2] = trignumstart >> 32; 
	  sendsupdata[b+3] = trignumstart >> 48; 

	  // Number of slices 
	  sendsupdata[b+4] = slices; 
	  
	  // Start of data relative to the hardware trigger 
	  sendsupdata[b+5] = numstart_data; 
	  
	  //number of values written 
	  sendsupdata[b+6] = valueswritten; 
	} 

	// Increment data start index
	numstart_dataplus++; 

	// Increment data counter
	i++; 

      } 

      // If counter has not been increased
      if(increasecounter==false){ 
	datadelay=true;
  
	// If printing info...
	if (output){
	  // Print for user
	  std::cout << "Header: trignum: " << trignumstart 
		    << " Nslices: " << sendsupdata[b+4] 
		    << " Start data: " << sendsupdata[b+5] 
		    << " Number of sup values: " << sendsupdata[b+6] 
		    << std::endl; 
	}
	// Increase output FIFO counter
	b += 7; 
      } 
      
      // Increase the start index
      numstart_data=numstart_dataplus; 
      
    } // End loop over data to ZS

  } // End if off-psill 

  // Store size of output FIFO (array including headers and data) 
  zp_size = b;

}
