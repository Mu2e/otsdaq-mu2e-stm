# config/CMakeLists.txt

# ---- Sources (shared) ----                                                 
set(CONFIG_SOURCES
  stm_data.cc
)

# ============================================================               
#                         CET BUILD                                          
# ============================================================               
if(USE_CET)                                                                  
                                                                             
  cet_make_library(                                                          
    LIBRARY_NAME Config
    SOURCE                                                                   
    ${CONFIG_SOURCES}
    LIBRARIES PUBLIC
      Utils          
    LIBRARIES PRIVATE
      stmdaq_all_deps      
    )                                                                          
                                                                             
  install_headers()                                                          
  install_source()  

# ============================================================               
#                    STANDARD CMAKE BUILD                                    
# ============================================================               
else()                                                                       
                                                                             
  add_library(Config SHARED                                                   
    ${CONFIG_SOURCES}                                                         
  )                                                                          
                                                                             
  # Produce libConfig.so (case-sensitive name)                                
  set_target_properties(Config PROPERTIES                                     
    OUTPUT_NAME "Config"                                                      
  )                                                                          
                                                                             
  target_link_libraries(Config PRIVATE                                        
    stmdaq_all_deps                                                          
  )                                                                          
                                                                             
endif()



