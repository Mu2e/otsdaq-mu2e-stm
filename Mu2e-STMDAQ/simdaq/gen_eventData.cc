// Gen Events code                                                       
#include "Mu2e-STMDAQ/simdaq/gen_eventData.hh"
#include <random>


// Constructor
GenEvents::GenEvents(const std::shared_ptr<STMdata>& stm_, size_t event_num_, size_t event_period_, size_t prescale_, int adcCountsAroundPeakZS_) :
  stm(stm_),
  fADC(stm->fw_config.fADC*1e6),// Hz
  tADC(stm->fw_config.tADC), // s
  event_num(static_cast<double>(event_num_)),
  event_period(event_period_),
  event_len((double)event_period*1e-9 * fADC),
  event_size(event_len*sizeof(int16_t)),
  prescale(prescale_),
  adcCountsAroundPeakZS(adcCountsAroundPeakZS_),
  data_len(event_len*event_num),
  data_size(data_len*sizeof(int16_t)),
  buffer_arr_len(max_events_per_struct*event_len) {
  
  // Notify user
  std::cout << "Event len = " << event_period << " ns = " << event_len << " ADC values = " << event_size << " bytes." << std::endl;
  std::cout << "Event num = " << event_num << " events = " << data_len << " ADC values = " << (double)data_len*sizeof(int16_t)*1e-9 << " GB." << std::endl;
  std::cout << "Raw prescale value = " << prescale << "." << std::endl;
   
}


//calculate peak
double GenEvents::calculate_peak(uint16_t time, uint16_t Amp, uint16_t tshift, uint16_t tfall, uint16_t tdecay){

  double fallexp = -2*Amp*1.0/(1+exp(-1.0*(time-tshift)/(tfall)));
  double riseexp = -2*Amp*(1-1.0/(1+exp(-1.0*(time-tshift)/tdecay)));

  if (time < tshift){
    return fallexp;}
  else{
    return riseexp;}
}


// Generate event data
void GenEvents::gen_events(const std::shared_ptr<BufferQueue<std::vector<int16_t>>>& queue,
			   const std::shared_ptr<SignalHandler>& signal,
			   std::atomic<bool>& finished){

  //  std::random_device rd{};
  //  std::mt19937 gen{rd()};

  // The EWT
  int16_t EWT = 0;

  // ADC count
  size_t adc_count = 0;

  // mersenne twister with rd seed for noise and peak position generation     
  std::random_device rd{};
  std::mt19937 gen{rd()};


  // Number of peaks to generate per event
  int npeaks = 0;

  //true for using rate, false for #of peaks
  bool peaksFromRate = false;

  //Only using the one that matches the mode!
  double peakrate = 100e3; //Hz
  size_t maxpeaks = 10;

  if (peaksFromRate == true){
  //if rate is set, then use a fixed # of peaks 
  npeaks = peakrate*1e-9*event_len;
  }
 

  size_t totalcount = 0;
  
  // Vector of peak times for this event
  //std::vector<size_t> peakposes(maxpeaks,0);

  // Header variables
  uint64_t ADCclock = -200;
  uint64_t Ch_DTCclk = -200;
  uint64_t EM = -200;
    
  // Get empty software event header
//  const sw_event_header hdr_temp = stm->create_sw_eHdr(EWT,ADCclock,Ch_DTCclk,EM);
  
  // Empty event vector
  //std::shared_ptr<std::vector<int16_t>> event_data = std::make_shared<std::vector<int16_t>>(event_num*event_len+adcCountsAroundPeakZS*maxpeaks+2*maxpeaks,0);
//  std::shared_ptr<std::vector<int16_t>> event_data = std::make_shared<std::vector<int16_t>>(sw_eHdr.len+event_len,0);
  


  //noise distribution, mean 0, rms 20
  std::normal_distribution<float> d{0,20};
  
  // While not the max EWT and no stop signal
  //while (EWT < event_num && !stop::should_stop()){
  for (int j=0; j < event_num; j++){

    const sw_event_header hdr_temp = stm->create_sw_eHdr(EWT,ADCclock,Ch_DTCclk,EM);

    std::shared_ptr<std::vector<int16_t>> event_data = std::make_shared<std::vector<int16_t>>(sw_eHdr.len+event_len+2*maxpeaks+(adcCountsAroundPeakZS+2)*maxpeaks,0);

    // Reference event_data
    auto& event = *event_data;
    //std::cout<<"Current EWT: "<<EWT<<std::endl;
    // Copy header into event 
    std::copy(hdr_temp.begin(), hdr_temp.end(), event.begin());
    
    // Event data count
    int16_t event_data_count = sw_eHdr.len;
    
    // Get EWT split into 4 int16_ts
    //std::vector<int16_t> split_EWT = stm->split_uint64_t(EWT); 
    
    // Change EWT in event header (is copy 4 x int16_t - should be 3 x int16_t)
    //std::copy(split_EWT.begin(), split_EWT.end(), event.begin() + sw_eHdr.EWT_0);
    event[sw_eHdr.EWT_0] = EWT;
    //std::cout<<"ewt: "<<EWT<<" written number in header: "<<event[sw_eHdr.EWT_0]<<std::endl;
    
    //fix to stop < 10 events defaulting to 10
    size_t max_events_requested;
    if (max_events_per_struct >= event_num) max_events_requested = event_num;
    else max_events_requested = max_events_per_struct;

    // mersenne twister with rd seed for noise and peak position generation     
    //std::random_device rd{};
    //std::mt19937 gen{rd()};


    if (peaksFromRate == false){
    	// Randomly generate actual number of peaks for this event
    	std::uniform_int_distribution<size_t> dp{0,maxpeaks};
    	npeaks = dp(gen);
    }


//    size_t npeaks = 2;
   

    std::vector<size_t> peakposes(npeaks,0);

    // Peak amplitude
    //(kept this as double for precision but may be overkill?)
    double peakAmp = 1000.;


//    peakposes[0] = 15000;
//    peakposes[1] = 14400;

    
    // Loop over all peaks in event and get times
    for (int i = 0; i < npeaks; i++){
      std::uniform_int_distribution<size_t> di{sw_eHdr.len+adc_count,sw_eHdr.len+adc_count+event_len};
      peakposes[i] = di(gen);
      //
      
      std::cout<<"Generating peak at: "<<peakposes[i]<<std::endl;
    }


    std::sort(peakposes.begin(),peakposes.end());

   // for (int i = 0; i < npeaks; i++){
   //	    std::cout<<peakposes[i]<<std::endl;
   // }

    //  ----------
    // RAW DATA
    //  ----------
    
    // Loop over raw event length
    for (int i = 0; i < event_len; i++){
       
      // Get noise sample
      event[sw_eHdr.len+i] = std::round(d(gen)); //round to int
    //  event[sw_eHdr.len+i] = 0;
   //   std::cout<<"(GenEvents) "<<event[sw_eHdr.len+i]<<std::endl;

    }


      //loop over all peaks      
      for (int p = 0; p < npeaks; p++){


	          // Loop over raw event length
    for (int i = 0; i < event_len; i++){

	
	//todo: user-defined differences in peaks so they're not all identical
	
	// Get peak discontinuity
	const size_t tshift = peakposes[p];


	// Get peak sample
 	double peak = GenEvents::calculate_peak(sw_eHdr.len+i,peakAmp,tshift,10,1000);
	int16_t peak_int = std::round(peak);
	 //std::cout<<"calc peak :"<<tshift<<" value: "<<peak<<std::endl;


     
	//rewrite each point with the old value plus the peak, for every peak 	
//	event[sw_eHdr.len+i] += peak_int;
	event[sw_eHdr.len+i] += peak;
//	std::cout<<"(GenEvents) "<<i<<" : "<<peak<<std::endl;

       }
	
      }
 
     


    
    
    // Change raw len in header
    event[sw_eHdr.RAW_LEN] = event_len;
    //std::cout<<"total event size: "<<event.size()<<std::endl;
    
    // increase event data count by event_len
    event_data_count += event_len;

    queue->push(event_data);

    // Increase the EWT
    ++EWT;

    // Increase the adc count by event length
    adc_count += event_len;
    totalcount += event_len;



    
    //  ----------
    // ZS DATA
    //  ----------

    // Last ZS region ending index
    size_t last_start_index = 0;
    size_t last_end_index = 0;


    size_t overlap_offset = 0;
    size_t overlap_regions = 20;

    // Number of ZS regions
    size_t zs_regions = 0;

    // Keep track of last length index for overlapping regions
    size_t last_len_index = 0;

    // Total ZS length
    size_t tot_zs_len = 0;
    
    //track ZS header length index
    size_t lenindex = 0;

    //total length added by overlaps of ZS regions
    size_t totallen;
  


    // Loop over all peaks
    for (int p = 0; p < npeaks; p++){      
 
      // Find peak centre
      size_t tshift = peakposes[p];
     
      // Find peak start
      int startindex = tshift - adcCountsAroundPeakZS/2;
      // Find peak end
      int endindex = tshift + adcCountsAroundPeakZS/2;
      // ZS data len
      size_t len = adcCountsAroundPeakZS;


      //remove any header elements from the ZS region
      if (startindex < sw_eHdr.len) {
	      len = len - (sw_eHdr.len-startindex);
	      startindex = sw_eHdr.len;
      }

      //remove overflow elements 
      if (endindex > sw_eHdr.len+event_len) {
              len = len - (endindex-(sw_eHdr.len+event_len));
	      endindex = sw_eHdr.len+event_len;
      }



      //if this peak overlaps with a previous one in this loop      
      if (last_end_index > startindex){
	   
	   //only want to write what's new
	   startindex = last_end_index;
	   
	   //length of extra data being added
	   len = endindex-last_end_index; 
	   
           //overlap_offset = len;
	 
	   totallen += len;
	   	   
      }
  

     //otherwise it's a new peak and new ZS region
     else {
     
	//Count number of ZS regions
	++zs_regions;        
       
        //write start index
	event[event_data_count] = startindex;
        ++event_data_count;
	// Store the length
	event[event_data_count] = len;
	//update this for final region length update
	lenindex = event_data_count;
	++event_data_count;
	
     }

	// Keep track of last stored length index
	last_len_index = event_data_count;
	last_start_index = startindex;

	//update the length in the header to the full region length
        event[lenindex] += totallen;

      //write the ZS into the event vector	
      for (int i = startindex; i < startindex+ len; i++){
          event[event_data_count+i-startindex] = event[i];
      }

   //   std::cout<<"Write position: "<<event_data_count<<" to "<< event_data_count + len<<std::endl;
      //std::cout<<"Len: "<<len<<std::endl; 

      // Increase event data counters
      event_data_count += len;
      tot_zs_len += len;

  //    std::cout<<"Event data count (end of peak loop): "<<event_data_count<<std::endl;
      
      // Update last length index
      last_end_index = endindex;
      totallen = 0; //reset for next peak     
     
    } // End loop over peaks


    // Update header
    event[sw_eHdr.ZS_REGIONS] = zs_regions;
    event[sw_eHdr.ZS_LEN] = tot_zs_len;    
    
    //  ----------
    // PULSE HEIGHTS
    //  ----------
      
    // Loop over all peaks
    for (int p = 0; p < npeaks; p++){      
      // Pulse time
      event[event_data_count] = peakposes[p];
      std::cout<<"gen peakposes (us): "<<peakposes[p]*10.0/3000<<std::endl;;
      ++event_data_count;
      // Pulse height
      event[event_data_count] = peakAmp;
      ++event_data_count;
    }

    // Update header
    event[sw_eHdr.PH_NUM] = npeaks;
//    std::cout<<"Should have n peaks: "<<npeaks<<std::endl;


   
  } // End for over events
  
  // Signal finish
  finished = true;
  
  // Notify user
  std::cout << "Generated " << EWT << "EWTs." << std::endl;
  
}

