# utils/CMakeLists.txt

# ---- Sources (shared) ----                                                 
set(UTILS_SOURCES                                                            
  cpu_utils.cc                                                               
  Logger.cc                                                                  
  async_logger.cc                                                            
  EnvVars.cc                                                                 
  signal_handler.cc                                                          
  shm_manager.cc                                                             
)

# ============================================================               
#                         CET BUILD                                          
# ============================================================               
if(USE_CET)                                                                  
                                                                             
  cet_make_library(                                                          
    LIBRARY_NAME Utils
    SOURCE                                                                   
      ${UTILS_SOURCES}                                                       
    LIBRARIES PRIVATE
      stmdaq_all_deps      
    )                                                                          
                                                                             
  install_headers()                                                          
  install_source()  

# ============================================================               
#                    STANDARD CMAKE BUILD                                    
# ============================================================               
else()                                                                       
                                                                             
  add_library(Utils SHARED                                                   
    ${UTILS_SOURCES}                                                         
  )                                                                          
                                                                             
  # Produce libUtils.so (case-sensitive name)                                
  set_target_properties(Utils PROPERTIES                                     
    OUTPUT_NAME "Utils"                                                      
  )                                                                          
                                                                             
  target_link_libraries(Utils PRIVATE                                        
    stmdaq_all_deps                                                          
  )                                                                          
                                                                             
endif()
