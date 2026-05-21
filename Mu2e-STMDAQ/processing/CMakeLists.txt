# processing/CMakeLists.txt

# ---- Sources (shared) ----                                                 
set(PROCESSING_SOURCES
  template_module.cc
  buffer_pool.cc
  udp.cc
  check_data.cc
  form_events.cc
  baseline.cc
  zero_suppress.cc
  noise.cc
  mwd.cc
  pulse_height.cc
  prescale.cc
  event_builder.cc
  write_manager.cc
  tcp.cc
  operation_manager.cc
  thread_manager.cc
  dqm_manager.cc
)

# ============================================================               
#                         CET BUILD                                          
# ============================================================               
if(USE_CET)                                                                  
                                                                             
  cet_make_library(                                                          
	  LIBRARY_NAME Processing
    SOURCE                                                                   
     ${PROCESSING_SOURCES}
    LIBRARIES PUBLIC
      Utils          
      Hardware
      Buffers
      Config
      Debug
    LIBRARIES PRIVATE
      stmdaq_all_deps      
      fftw_deps
    )                                                                          
                                                                             
  install_headers()                                                          
  install_source()  

# ============================================================               
#                    STANDARD CMAKE BUILD                                    
# ============================================================               
else()                                                                       
  add_library(Processing SHARED
     ${PROCESSING_SOURCES}
  )

  set_target_properties(Processing PROPERTIES OUTPUT_NAME "Processing")

  target_link_libraries(Processing PRIVATE
    stmdaq_all_deps
    fftw_deps
    Utils
    Hardware
    Buffers
    Config
    Debug
  )

endif()
