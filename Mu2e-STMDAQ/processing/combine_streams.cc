// Template module code
#include "Mu2e-STMDAQ/processing/combine_streams.hh"

// Constructor
CombineStreams::CombineStreams(Config& cfg_,
			   const std::shared_ptr<AsyncLogger>& logger_,
			   const std::shared_ptr<STMdata>& stm_,
			   const int CHAN_) :
  cfg(cfg_), logger(logger_), stm(stm_), CHAN(CHAN_) {

  // Instantiate any class variables in the constructor  
  
  // Initialise function map for OperationManager
  functionMap["make_data_stream"] = [this](std::shared_ptr<DataStruct>& buffer) {
    make_data_stream(buffer);
  };
  functionMap["send_combined_stream"] = [this](std::shared_ptr<DataStruct>& buffer) {
    send_combined_stream(buffer);
  };
}

void CombineStreams::make_data_stream(std::shared_ptr<DataStruct>& buffer){

  // Get raw header map
  std::vector<hdrMapTuple> raw_hdr_map = buffer->raw_header_map;
  size_t n_evts = buffer->raw_header_num;

  // Get raw ADC data
  std::vector<int16_t>* data = &buffer->raw;

  // Initialise combined stream
  std::vector<combined_stream_tuple> combined_stream(n_evts);

  // Get peak height and time data
  //std::vector<int16_t>* ph_data = &buffer->mwd_peak_heights;
  //std::vector<int16_t>* pt_data = buffer->mwd_peak_times;

  for (size_t iE=0; iE<n_evts; iE++) {

    // Get the ADC start and end index of this event
    size_t buffer_start_idx = std::get<hdrMap_adcIndex>(raw_hdr_map[iE]);
    size_t evt_length = std::get<hdrMap_dataLen>(raw_hdr_map[iE]);
    size_t buffer_end_idx = buffer_start_idx + evt_length; // Start index + length of event in samples

    // Get the actual event header from the map and the important info from it
    std::array<int16_t,eHdr_Len> raw_fw_hdr = std::get<hdrMap_hdrData>(raw_hdr_map[iE]);
    std::vector<int16_t> raw_fw_hdr_vec(raw_fw_hdr.begin(), raw_fw_hdr.end());
    uint16_t evt_num = stm->get_event_number(raw_fw_hdr_vec,0);
    uint64_t EWT = stm->get_EWT(raw_fw_hdr_vec,0);
    uint64_t evt_mode = stm->get_event_mode(raw_fw_hdr_vec,0);

    // Make the event header for art and add to stream
    std::array<uint64_t,art_evtHdr_len> evt_art_hdr = {evt_num,EWT,evt_mode};
    std::get<art_evtHdr>(combined_stream[iE]) = evt_art_hdr;
    
    // Make the raw header and add to stream
    uint64_t raw_art_hdr = 0;
    bool ps_evt = buffer->raw_ps_bool[iE];
    raw_art_hdr |= ((uint64_t)ps_evt << 16);
    raw_art_hdr |= ((uint64_t)evt_length << 24);
    std::get<art_rawHdr>(combined_stream[iE]) = raw_art_hdr;

    // If this event is not being prescaled, write the raw data
    if (buffer->raw_ps_bool[iE] != 1) {
      std::vector<int16_t> raw_data;
      raw_data.insert(raw_data.end(),
	  data->begin() + buffer_start_idx, 
	  data->begin() + buffer_end_idx);
      std::get<art_rawData>(combined_stream[iE]) = raw_data;
    }

    // Make the ZS header and add to stream
    uint64_t zs_art_hdr = 0;
    std::get<art_zsHdr>(combined_stream[iE]) = zs_art_hdr;

    // If there is a peak here (after prescaling) write out the data
    //if (buffer->ps_peak_index[iE] > buffer_start_idx && buffer->ps_peak_index[iE] < buffer_end_idx) {
      //std::vector<int16_t>* zs_data;
      //zs_data.insert(zs_data.end(), 
	//	      data->begin() + ps_peak_index - before_peak, 
	//	      data->begin()+ ps_peak_index + after_peak);
      //std::get<art_zsData>(combined_stream[iE]) = zs_data;
    //}
    
    uint64_t ph_art_hdr = 0;
    std::get<art_peakHdr>(combined_stream[iE]) = ph_art_hdr;
    //if (buffer->mwd_peak_heights[iE] != 0 ) std::get<art_peakData>(combined_stream[iE]) = buffer->mwd_peak_heights;
    
  }

}

// Second template function activated by operation manager
// and implemented by thread manager
void CombineStreams::send_combined_stream(std::shared_ptr<DataStruct>& buffer){
  
  // Do something to buffer
  
}

