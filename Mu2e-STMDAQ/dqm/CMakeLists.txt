# dqm/CMakeLists.txt

# ---- Sources (shared) ----                                                 
set(DQM_SOURCES
  dqm_manager.cc
)

# ============================================================               
#                         CET BUILD                                          
# ============================================================               
if(USE_CET)                                                                  
                                                                             
  cet_make_library(                                                          
	  LIBRARY_NAME DQM
    SOURCE                                                                   
     ${DQM_SOURCES}
    LIBRARIES PUBLIC
      Utils          
      Config
    LIBRARIES PRIVATE
      stmdaq_all_deps      
    )                                                                          
                                                                             
  install_headers()                                                          
  install_source()  

# ============================================================               
#                    STANDARD CMAKE BUILD                                    
# ============================================================               
else()                                                                       

add_library(DQM SHARED
  dqm_manager.cc
)

set_target_properties(DQM PROPERTIES OUTPUT_NAME "DQM")

target_link_libraries(DQM PRIVATE
  stmdaq_all_deps
)

endif()
