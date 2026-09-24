# hardware/CMakeLists.txt

# ---- Sources (shared) ----                                                 
set(HARDWARE_SOURCES
  hw_manager.cc
)

# ============================================================               
#                         CET BUILD                                          
# ============================================================               
if(USE_CET)                                                                  
                                                                             
  cet_make_library(                                                          
	  LIBRARY_NAME Hardware
    SOURCE                                                                   
      ${HARDWARE_SOURCES}
    LIBRARIES PUBLIC
      Utils          
      Config
    LIBRARIES PRIVATE
      stmdaq_all_deps      
      stmdaq_python_deps
    )                                                                          
                                                                             
  install_headers()                                                          
  install_source()  

# ============================================================               
#                    STANDARD CMAKE BUILD                                    
# ============================================================               
else()                                                                       
                                                                             
  add_library(Hardware SHARED   
    ${HARDWARE_SOURCES}                                                         
  )                                                                          
                                                                             
  # Produce libConfig.so (case-sensitive name)                                
  set_target_properties(Hardware PROPERTIES                                     
    OUTPUT_NAME "Hardware"                                                      
  )                                                                          
                                                                             
  target_link_libraries(Hardware PRIVATE 
    stmdaq_all_deps                                                      
    stmdaq_python_deps
  )                                                                          
                                                                             
endif()

