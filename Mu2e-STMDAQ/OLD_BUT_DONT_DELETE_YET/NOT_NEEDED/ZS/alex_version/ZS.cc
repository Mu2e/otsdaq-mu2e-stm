#include <stdlib.h>
#include <stdio.h>
#include <iostream>
#include <fstream>
#include <chrono>

// Zero-suppresion header
#include "STMDAQ-TestBeam/ZS/ZS.hh" 

// Standard constructor - shouldn't be used
ZS::ZS(){} 

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
  }
  
  // Return the chunk data array
  return allchunk;
 
} 

// Read the ADC data header
void ZS::ReadInputHeader (int16_t*  allchunk){

  // Print to user 
  if (output) std::cout<<"------------Executing TOP function------------"<<std::endl;
  
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
void ZS::supalg (int16_t* data){
  
  // Parameters 
  //  tadc = tadc_(); // ADC sampling time
  // Get number ADC values to store before the trigger
  //  prenumADCstored = prenumADCstored_(tadc);
  // Get number ADC values to store after the trigger
  //  postnumADCstored = postnumADCstored_(tadc_());
  
  //chunk= chunk_();
  // Get the number of triggers per chunk to send in spill
  ntriggers_chunkSPILL = ntriggers_chunkSPILL_();

  // Get the number of ADC values per trigger in spill
  nADC_triggerSPILL = nADC_triggerSPILL_();

  // Get the number of ADC values per chunk according to triggers in spill
  nADC_chunk = nADC_chunk_(nADC_triggerSPILL,ntriggers_chunkSPILL);

  // Get number of ADC values per trigger in gap
  nADC_triggerGAP =  nADC_triggerGAP_();

  // Calculate the average of the gradient each
  // n_average ADC values to avoid fluctuations 
  int n_average = 5; // Number of points to average
  unsigned long int j = 0; // All points counter
  double av_gradient = 0; // Average gradient
  double av_ADCtime = 0; // Average time

  // Initialise last trigger to zero
  lasttrig = 0; 
  
  // If printing info...
  if (output){
    // Print for user
    std::cout << "Average of gradient taken with: "
	      << n_average << " values" << std::endl; 
  }

  // Initialise variables
  // Boolean if peak found 
  bool peak = false;

  // The counter for NEW triggers found in the chunk
  unsigned long int trig_count = 0;

  // Initialise zero-suppressed data array
  suppressed_data = new int16_t[nADC_chunk]; 

  // Initialise suppressed data array to a big number 
  init = 3000;  
  for(int i = 0; i < numADCtoZS; i++){
    suppressed_data[i] = init;
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
    
    // Loop over all average entries
    for(int k = 0; k < n_average; k++){ 
      // Calculate gradient
      int16_t grad_high = data[headersize+j+k+window]; 
      int16_t grad_low = data[headersize+j+k]; 
      int16_t gradient = grad_high - grad_low; 
      // Sum average gradient
      av_gradient += gradient; 
      // Sum average time
      double time = j+k;
      av_ADCtime += time; 
    } 

    // Calculate average gradient
    double avg_gradient = av_gradient/n_average; 
    // Calculate average trigger time
    double avg_time = av_ADCtime/n_average; 
    
    // Increase loop counter 
    j += n_average; 

    // THERE IS A BUG HERE
    // WHAT HAPPENS IF avg_gradient == threshold ??

    // If the averaged gradient is beyond the threshold
    if(avg_gradient > threshold){
      // No peak found
      peak=false;
      continue;
    } 

    // Skip the rest indexes of the peak after 
    // the trigger that has already been stored 
    if(avg_gradient < threshold && peak==true){ 
      continue;
    } 

    // If within the threshold and no peak previously found...
    if(avg_gradient < threshold && peak==false){ 

      // Peak found!
      peak=true; 

      // Store trigger time since the start of the chunk
      unsigned long int peak_trig_time = chunkstart + avg_time;
   
      // If printing info...
      if (output){
	// Print for user
	std::cout << "absolute time: " << avg_time << std::endl; 
	std::cout << "trig: " << peak_trig_time << std::endl; 
      }

      // In reality, only store the found trigger if:
      // (a) it is after the last trigger stored from previous call.
      // (b) it is separated the previous one by prenumADCstored.
      // (c) the number of adc values to store after it is within the chunk limit.
      if( (peak_trig_time > last_triggerstored) && // (a)
	  (peak_trig_time - prenumADCstored_(tadc_()) > last_triggerstored) && // (b)
	  (peak_trig_time + postnumADCstored_(tadc_()) < chunkstart + numADCtoZS)){ // (c)
	
	// Get the last trigger stored
	lasttrig = peak_trig_time; 

	// Store data if the index (realtrigger_abs[j] + k) is 
	// higher than the last ADC value stored in previous chunk 
	// (last_triggerstored + postnumADCstored_(tadc_())). 	

	// We have to convert last trigger stored + post to absolute 
	// value relative to the start of the chunk.
	double lastvaluestored = last_triggerstored + postnumADCstored_(tadc_()); 
	double limit = lastvaluestored - chunkstart; 	

	// Get number ADC values to store before the trigger
	int k = -prenumADCstored_(tadc_()); 	

	// Fill trigger positions with -pre and +post adc values 
	while(k <= postnumADCstored_(tadc_())){ 
	  		  
	  // See that there are prenumADC stored 
	  int checkprenum = int(avg_time)+k; 
	  
	  // If within limits to store...
	  if(checkprenum > limit && checkprenum >= 0){ 
	    // Store suppressed data
	    suppressed_data[(uint)avg_time+k] = data[headersize+(uint)avg_time+k];
	    //	    std::cout << headersize+(uint)avg_time+k << " ";
	    
	  } 
	  // Else if not within limits...
	  else{
	    
	    // If printing info...
	    if (output){
	      // Print for user
	      std::cout << "This value is not stored, index: " 
			<< int(avg_time)+k << std::endl;
	    }
	    
	  }
	  
	  // If the number of ADC values stored before the trigger is less zero
	  if(checkprenum < 0){
	    // If printing info...
	    if (output){
	      // Print for user
	      std::cout << "Not prenum ADC values to store in this chunk for this ZS trigger, not all values required are stored for this trigger ---wrong energy expected" << std::endl;
	    }
	  } 
	  
	  // Increment trigger position index
	  k++; 
	  
	} // End loop over trigger positions, k

	//	std::cout << std::endl;
		
	// Increment stored trigger number
	trig_count++; 

      } // End if store trigger
          
    } // End if trigger found 

  }
  
  // If printing info...
  if (output){
    // Print for user
    std::cout << "fadc= " << fADC
	      << " tadc_()= " << tadc_()
	      << " MHz, window: " << window
	      << " threshold " << threshold
	      << " tbefore: " << tbefore
	      << " us (" << prenumADCstored_(tadc_()) << " ADC Counts)"
	      << " tafter " << tafter
	      <<" us (" << postnumADCstored_(tadc_()) << " ADC Counts)"
	      << std::endl; 
  }


  // Get number of stored triggers
  ntriggers = trig_count;

  // Update last trigger stored in this function call 
  if (lasttrig != 0){  
    last_triggerstored = lasttrig; 
  } 

  // If printing info...
  if (output){
    // Print for user
    std::cout<<"Number of new triggers found in chunk: "<< trig_count<<std::endl;
    std::cout << "Last trigger stored updated: " << last_triggerstored << std::endl; 
  }

  return; 

}

// Form the output FIFO
int16_t* ZS::Form_OutputFIFO() { 

  // Initalise array for suppressed data and headers (output FIFO)
  int16_t* outputFIFO = new int16_t[nADC_chunk] (); 

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
    memcpy(outputFIFO,split64(last_triggerstored),4*sizeof(int16_t));

    // Mode
    outputFIFO[4]=mode; 

    // Input chunk number 
    memcpy(outputFIFO+5,split64(chunknum),4*sizeof(int16_t));

    // NEED TO UPDATE TO HLS VERSION IN 
    // /work/akeshavarzi/STMDAQ-TestBeam/Claudia/ZS_prelim/HLSZS

    // Input chunk start in ADC 
    memcpy(outputFIFO+9,split64(chunkstart),4*sizeof(int16_t));

    // Increase output FIFO array length 
    b += 13; 

    // For number of triggers in hardware 
    for(unsigned long int j = 0; j < ntriggers_chunkSPILL; j++){ 
   
      // Start of data relative to the hardware trigger 
      int16_t numstart_data = 0; // in ADC values
      int16_t numstart_dataplus = 0; 
      // Number of values written
      int16_t valueswritten = 0; 
      // Number of slices 
      int16_t slices = 0; 
      // Boolean to increase counter
      bool increasecounter = false; 
      // Boolean to tell you if your pulse is in the middle of a trigger
      bool datadelay = false; 
  
      // If printing info...
      if (output){
  	// Print for user
  	std::cout<<"NEW HARDWARE TRIGGER----------"<<j; 
      }
   
      // While inside the trigger length (loop over i)
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

  	// WRITE TRIGGER HEADER TO OUTPUT FIFO

  	// Trigger number 
  	outputFIFO[b]=j & 0x0000FFFF; 
  	outputFIFO[b+1] = j >> 16; 
  	outputFIFO[b+2] = j >> 32; 
  	outputFIFO[b+3] = j >> 48; 
  	// Number of slices 
  	outputFIFO[b+4] = slices; 
  	// Start of data relative to the hardware trigger 
  	outputFIFO[b+5] = numstart_data; 

  	// While inside trig length and  data != large initial value
  	while(suppressed_data[i] != init && i < triglength){ 
  	  
  	  // Increment number of values written 
  	  valueswritten++; 

  	  // Suppressed data in this trigger 
  	  outputFIFO[b+6+valueswritten] = suppressed_data[i]; 	  
  	  
  	  // Incremenet data index
  	  i++; 

  	  // Incrememnt data counter
  	  numstart_dataplus++; 

  	  // Set boolean for counter increment to true
  	  increasecounter = true; 

  	} 

  	// ADD NUMBER OF VALUES WRIOTTEN TO TRIGGER HEADER
  	outputFIFO[b+6] = valueswritten; 

  	// If the data counter has been incremented
  	if(increasecounter == true){ 

  	  // If printing info...
  	  if (output){
  	    // Print for user
  	    std::cout << "Header: trignum: " << outputFIFO[b]
  		      << " Nslices: " << outputFIFO[b+4]
  		      << " Start data: " << outputFIFO[b+5] 
  		      << " Number of sup values: " << outputFIFO[b+6] << std::endl; 
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
  	  outputFIFO[b] = j & 0x0000FFFF; 
  	  outputFIFO[b+1] = j >> 16; 
  	  outputFIFO[b+2] = j >> 32; 
  	  outputFIFO[b+3] = j >> 48; 
  	  // Number of slices 
  	  outputFIFO[b+4] = slices; 
  	  // Start of data relative to the hardware trigger 
  	  outputFIFO[b+5] = numstart_data; 
  	  //number of values written 
  	  outputFIFO[b+6] = 0; 

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
  	    std::cout << "Header: trignum: " << outputFIFO[b] 
  		      <<" Nslices: "<< outputFIFO[b+4] 
  		      << " Start data: " << outputFIFO[b+5] 
  		      << " Number of sup values: " << outputFIFO[b+6]
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
    outputFIFO[0] = last_triggerstored & 0x0000FFFF; 
    outputFIFO[1] = last_triggerstored >> 16; 
    outputFIFO[2] = last_triggerstored >> 32; 
    outputFIFO[3] = last_triggerstored >> 48; 
    // Mode 
    outputFIFO[4] = mode; 
    // Input chunk number 
    outputFIFO[5] = chunknum & 0x0000FFFF; 
    outputFIFO[6] = chunknum >> 16; 
    outputFIFO[7] = chunknum >> 32; 
    outputFIFO[8] = chunknum >> 48; 
    // Input chunk start in ADC 
    outputFIFO[9] = chunkstart & 0x0000FFFF; 
    outputFIFO[10] = chunkstart >> 16; 
    outputFIFO[11] = chunkstart >> 32; 
    outputFIFO[12] = chunkstart >> 48; 
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
  	outputFIFO[b] = trignumstart & 0x0000FFFF; 
  	outputFIFO[b+1] = trignumstart >> 16; 
  	outputFIFO[b+2] = trignumstart >> 32; 
  	outputFIFO[b+3] = trignumstart >> 48; 
  	// Number of slices 
  	outputFIFO[b+4] = slices; 
  	// Start of data relative to the hardware trigger 
  	outputFIFO[b+5] = numstart_data; 
  	// Number of values written 
  	valueswritten++; 
  	outputFIFO[b+6] = valueswritten; 
  	// Suppressed data in this trigger 
  	outputFIFO[b+6+valueswritten] = suppressed_data[i]; 

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
  		    << " Nslices: " << outputFIFO[b+4] 
  		    << " Start data: " << outputFIFO[b+5] 
  		    <<" Number of sup values: " << outputFIFO[b+6] 
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
  	  outputFIFO[b] = trignumstart & 0x0000FFFF; 
  	  outputFIFO[b+1] = trignumstart >> 16; 
  	  outputFIFO[b+2] = trignumstart >> 32; 
  	  outputFIFO[b+3] = trignumstart >> 48; 

  	  // Number of slices 
  	  outputFIFO[b+4] = slices; 
  	  
  	  // Start of data relative to the hardware trigger 
  	  outputFIFO[b+5] = numstart_data; 
  	  
  	  //number of values written 
  	  outputFIFO[b+6] = valueswritten; 
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
  		    << " Nslices: " << outputFIFO[b+4] 
  		    << " Start data: " << outputFIFO[b+5] 
  		    << " Number of sup values: " << outputFIFO[b+6] 
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

  return outputFIFO;

}

int16_t* ZS::do_ZS(int16_t* inputFIFO){

  //  std::cout << std::endl;

  auto start = std::chrono::steady_clock::now();

  // Read the input header
  ReadInputHeader(inputFIFO);

  auto end = std::chrono::steady_clock::now();
  double timeNano = std::chrono::duration_cast <std::chrono::nanoseconds> (end-start).count();
  std::cout << "ReadInputHeader = " << timeNano*1e-9 << " seconds" << std::endl;    

  start = std::chrono::steady_clock::now();

  // Call the actual zero-suppression algorithm
  supalg(inputFIFO); //slow thing                                          

  end = std::chrono::steady_clock::now();
  timeNano = std::chrono::duration_cast <std::chrono::nanoseconds> (end-start).count();
  std::cout << "supalg = " << timeNano*1e-9 << " seconds" << std::endl;    

  start = std::chrono::steady_clock::now();

  // Form the output FIFO
  int16_t* outputFIFO = Form_OutputFIFO();

  end = std::chrono::steady_clock::now();
  timeNano = std::chrono::duration_cast <std::chrono::nanoseconds> (end-start).count();
  std::cout << "outputFIFO = " << timeNano*1e-9 << " seconds" << std::endl;    

  // Return the zero-supressed data
  return outputFIFO;

}
