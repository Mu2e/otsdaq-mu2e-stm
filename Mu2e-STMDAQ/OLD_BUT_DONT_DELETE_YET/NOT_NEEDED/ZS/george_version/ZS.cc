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
int16_t* ZS::supalg (int16_t* data){
  
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
  std::vector<int16_t> vec(numADCtoZS,3000);
  
  // Initalise array for suppressed data and headers (output FIFO)
  int16_t* outputFIFO = new int16_t[nADC_chunk] (); 

  // Initialise suppressed data array to a big number 
  init = 3000;  
  for(int i = 0; i < numADCtoZS; i++){
    suppressed_data[i] = init;
  }

  //std::cout << "splits: " << split(numADCtoZS-window),627) << "\n";
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
	//std::cout << "lasttrigstored and n trigs: " << last_triggerstored << "  " << trig_count << "\n";
	// Store data if the index (realtrigger_abs[j] + k) is 
	// higher than the last ADC value stored in previous chunk 
	// (last_triggerstored + postnumADCstored_(tadc_())). 	

	// We have to convert last trigger stored + post to absolute 
	// value relative to the start of the chunk.
	double lastvaluestored = last_triggerstored + postnumADCstored_(tadc_()); 
	double limit = lastvaluestored - chunkstart; 	
	//std::cout << "last vals:  " << last_triggerstored << "   " << postnumADCstored_(tadc_()) << "\n";
	// Get number ADC values to store before the trigger
	int k = -prenumADCstored_(tadc_()); 	

	//std::cout << "Dont store: " << k << "\n";

	// Fill trigger positions with -pre and +post adc values 
	while(k <= postnumADCstored_(tadc_())){ 

	  //std::cout << "Store: " << k << "\n";
	
	  // See that there are prenumADC stored 
	  int checkprenum = int(avg_time)+k; 
	  // If within limits to store...
	  if(checkprenum > limit && checkprenum >= 0){ 
	    // Store suppressed data
	    suppressed_data[(uint)avg_time+k] = data[headersize+(uint)avg_time+k];
	    //std::cout << "STORE =  " << (uint)avg_time+k << "   " << headersize+(uint)avg_time+k << "   " << data[headersize+(uint)avg_time+k] << "\n";
	    vec[(uint)avg_time+k] = data[headersize+(uint)avg_time+k];
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

	std::cout << "k = " << k << std::endl;
		
	// Increment stored trigger number
	trig_count++; 

      } // End if store trigger
          
    } // End if trigger found 
    
  auto start = std::chrono::steady_clock::now();
  
  // Initalise array for suppressed data and headers (output FIFO)
  //  int16_t* outputFIFO = new int16_t[nADC_chunk] (); 
  
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
    int Nsplit = 16;
    int ct = 0;
    std::vector<std::vector<int16_t>> ans = split(vec, Nsplit);
    std::vector<std::vector<int16_t>> ret;
    std::vector<std::vector<int16_t>> fail;
    std::vector<std::vector<int16_t>> final;
    std::cout << ans.size() << "\n";
    for(int i=0; i<Nsplit; i++){
      if ( std::adjacent_find( ans[i].begin(), ans[i].end(), std::not_equal_to<>() ) == ans[i].end() )
	{
	  fail.push_back(ans[i]);
  	  outputFIFO[b*i] = j & 0x0000FFFF; 
  	  outputFIFO[b*i+1] = j >> 16; 
  	  outputFIFO[b*i+2] = j >> 32; 
  	  outputFIFO[b*i+3] = j >> 48; 
  	  // Number of slices 
  	  outputFIFO[b*i+4] = 0; 
  	  // Start of data relative to the hardware trigger 
  	  outputFIFO[b*i+5] = 0; 
  	  //number of values written 
  	  outputFIFO[b*i+6] = 0; 
	  std::vector<int16_t> f;
	  /*f.insert(f.begin(), split64(last_triggerstored), split64(last_triggerstored)+4);
	  f.push_back(mode);
	  f.insert(f.begin()+5, split64(chunknum), split64(chunknum)+4);
	  f.insert(f.begin()+9, split64(chunkstart), split64(chunkstart)+4);
  	  f.push_back(i & 0x0000FFFF); 
  	  f.push_back(i >> 16); 
  	  f.push_back(i >> 32); 
  	  f.push_back(i >> 48); */
  	  // Number of slices 
  	  f.push_back(0); 
  	  // Start of data relative to the hardware trigger 
  	  f.push_back(0); 
  	  //number of values written 
  	  f.push_back(0); 
	  //std::cout << "All elements are equal each other" << std::endl;
	}
      else{
	ret.push_back(ans[i]);
	outputFIFO[b*i] = j & 0x0000FFFF; 
	outputFIFO[b*i+1] = j >> 16; 
	outputFIFO[b*i+2] = j >> 32; 
	outputFIFO[b*i+3] = j >> 48; 
	// Number of slices 
	outputFIFO[b*i+4] = 0; 
	// Start of data relative to the hardware trigger 
	outputFIFO[b*i+5] = 0; 
	//number of values written 
	std::vector<int16_t> v;
	int ct = 0;
	for(auto & j : ans[i]){
	  if(j!=init){
	    ct++;
	    outputFIFO[b*i+(7+ct)] = j;
	  }
	} 
	outputFIFO[b*i+6] = ct;
	std::vector<int16_t> f;
	/*f.insert(f.begin(), split64(last_triggerstored), split64(last_triggerstored)+4);
	f.push_back(mode);
	f.insert(f.begin()+5, split64(chunknum), split64(chunknum)+4);
	f.insert(f.begin()+9, split64(chunkstart), split64(chunkstart)+4);
	f.push_back(i & 0x0000FFFF); 
	f.push_back(i >> 16); 
	f.push_back(i >> 32); 
	f.push_back(i >> 48); */
	// Number of slices 
	f.push_back(0); 
	// Start of data relative to the hardware trigger 
	int ctX = 0;
	for(auto & j : ans[i]){
	  if(j!=init){
	    ctX++;
	    f.push_back(j);
	  }
	} 
	//number of values written 
	std::cout << "ct = " << ctX << "\n";
	f.insert(f.begin()+6, ctX);
	if(i==1){
	  int d=0;
	  for(auto & a : f){
	    d++;
	    std::cout << d << "  " << a << "\n";
	  }
	}
      }
    }
    auto end = std::chrono::steady_clock::now();
    double timeNano = std::chrono::duration_cast <std::chrono::nanoseconds> (end-start).count();
    std::cout << "time to split data: " << timeNano << "\n";
    std::cout << "Number of trigs with ZS data: " << ret.size() << "\n";
    std::cout << "Number of empty trigs: " << fail.size() << "\n";
  }
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

  if(last_triggerstored != 0){
    std::cout<<"Number of new triggers found in chunk: "<< trig_count<<std::endl;
    std::cout << "Last trigger stored updated: " << last_triggerstored << std::endl; 
    std::cout << "N triggers stored: " << ntriggers << std::endl; 
  }  
  
  // If printing info...
  if (output){
    // Print for user
    std::cout<<"Number of new triggers found in chunk: "<< trig_count<<std::endl;
    std::cout << "Last trigger stored updated: " << last_triggerstored << std::endl; 
  }

  std::cout << outputFIFO << "\n";

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
  int16_t* outputFIFO = supalg(inputFIFO); //slow thing                                          
  
  end = std::chrono::steady_clock::now();
  timeNano = std::chrono::duration_cast <std::chrono::nanoseconds> (end-start).count();
  std::cout << "supalg = " << timeNano*1e-9 << " seconds" << std::endl;    

  start = std::chrono::steady_clock::now();

  // Return the zero-supressed data
  return outputFIFO;

}

