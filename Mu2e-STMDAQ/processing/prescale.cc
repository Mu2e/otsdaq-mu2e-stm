// Prescale code
#include "Mu2e-STMDAQ/processing/prescale.hh"

// Constructor
Prescale::Prescale(const std::shared_ptr<AsyncLogger>& logger_,
                             const std::shared_ptr<STMdata>& stm_) :
  logger(logger_), stm(stm_),
  rng(static_cast<uint64_t>(std::time(nullptr))),
  p_cfg(stm->prescale_config) {

  // Prescale IDs and values for checks
  std::vector<std::pair<string,int>> ps = {{"Raw on-spill",p_cfg.raw.on_spill.prescale},
                                           {"Raw off-spill",p_cfg.raw.off_spill.prescale},
                                           {"ZS on-spill",p_cfg.zs.on_spill.prescale},
                                           {"ZS off-spill",p_cfg.zs.off_spill.prescale}};

  for (int i = 0; i < ps.size(); i++){
    string ID = ps[i].first;
    int val = ps[i].second;
    // Check that prescales values are prime numbers
    if (val!= 1 && !is_prime(val)){
      logger->log("Prescale: Error! " +
                  ID + " prescale =  " +
                  std::to_string(val) +
                  ". Must be a prime number!",0);
      std::this_thread::sleep_for(std::chrono::seconds(1));
    }
    // Check prescale number are within max values
    if (val > 0x7F){
      logger->log("Prescale: Error! " +
                  ID + " prescale =  " +
                  std::to_string(val) +
                  ". Prescale numbers must be 0–127!",0);
      std::this_thread::sleep_for(std::chrono::seconds(1));
    }
  }
    
  // Set header base values (prescale values don't change during running...)
  hdr_on_base = (static_cast<uint16_t>(p_cfg.raw.on_spill.prescale) & 0x7F) << 8;
  hdr_on_base |= (static_cast<uint16_t>(p_cfg.zs.on_spill.prescale) & 0x7F);
  hdr_off_base = (static_cast<uint16_t>(p_cfg.raw.off_spill.prescale) & 0x7F) << 8;
  hdr_off_base |= (static_cast<uint16_t>(p_cfg.zs.off_spill.prescale) & 0x7F);
  
  // Register operations for OperationManager
  register_operation("prescale_data", [this](auto& b){ prescale_data(b); });

}

// Prescale data
void Prescale::prescale_data(std::shared_ptr<DataStruct>& buffer){

  // Get raw and zs prescales
  const prescale_t& raw_prescale  = p_cfg.raw;
  const prescale_t& zs_prescale  = p_cfg.zs;
  
  // Loop over EWTs
  for (int i = 0; i < buffer->EWT_count; ++i) {

    // Get this EWT
    EWT_info& ewt = buffer->EWTs[i];
    uint64_t EWT = ewt.EWT;

    // Get EWT header
    sw_event_header& ewt_hdr = ewt.hdr;

    // Is this EWT on- or off-spill?
    const bool on_spill = true; // GET FROM EVENT MODE LATER

    // Get spill struct
    spill& s = sp[on_spill]; // [0]=off, [1]=on

    // Lambda function to apply prescale
    auto apply = [&](int dtype, // Data type (RAW/ZS)
                     const prescale_t& ps, // RAW/ZS Prescale values
                     bool& prescale_flag, // Prescale header value
                     uint16_t& header) { // Prescale header

      // Get prescale values if on/off spill
      const spill_t& p = on_spill ? ps.on_spill : ps.off_spill;

      // If at the start of new effective prescale window
      if (p.eff_prescale > 0 && (s.count % p.eff_prescale == 0)) {
        // Get the starting index of the prescale selection window
        s.selection_start[dtype] =
            s.count + uint64_t(rng.range(p.prescale)) * uint64_t(p.consecutive);
        // std::cout << "------" << std::endl;
        // std::cout << ((dtype == 0) ? "RAW" : "ZS") << " " << EWT << " " << s.selection_start[dtype] << std::endl;
      }

      // Are we in window to store data
      const bool in_window = 
          (s.count >= s.selection_start[dtype]) &&
          (s.count <  s.selection_start[dtype] + uint64_t(p.consecutive));

      // If outside window, prescale
      if (!in_window) prescale_flag = true;

      // std::cout << "\t" << EWT << " " << ((dtype == 0) ? "RAW" : "ZS") << " " << s.count << " " << prescale_flag << " " << header << std::endl;
      
      // Pack EWT header with prescale flag
      pack_hdr(dtype,prescale_flag,header);

      // std::cout << "\t" << EWT << " " << ((dtype == 0) ? "RAW" : "ZS") << " " << s.count << " " << prescale_flag << " " << header << std::endl;      
      
    };

    // Get header prescale flag
    uint16_t hdr = on_spill ? hdr_on_base : hdr_off_base;

    // Do Raw prescale
    apply(RAW, raw_prescale, ewt.raw.prescale, hdr);
    // Do ZS prescale
    apply(ZS, zs_prescale, ewt.zs.prescale, hdr);

    //    std::cout << EWT << " " << ewt.hdr[sw_eHdr.PRESCALE] << std::endl;
    
    // Store EWT header
    ewt.hdr[sw_eHdr.PRESCALE] = static_cast<int16_t>(hdr);

    //    std::cout << EWT << " " << ewt.hdr[sw_eHdr.PRESCALE] << std::endl;
    
    // Increment ONCE per EWT per spill stream (shared for RAW+ZS)
    ++s.count;
  }
}
