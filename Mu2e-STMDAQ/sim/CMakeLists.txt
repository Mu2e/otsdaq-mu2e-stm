# sim/CMakeLists.txt

# ---- Sources (shared) ----                                                 
set(SIM_SOURCES
  gen_data.cc
  send_TCP.cc
  gen_eventData.cc
)

# ============================================================               
#                         CET BUILD                                          
# ============================================================               
if(USE_CET)                                                                  
                                                                             
  cet_make_library(                                                          
	  LIBRARY_NAME Sim
    SOURCE     
      ${SIM_SOURCES}
    LIBRARIES PUBLIC
      Utils          
      Config
      Buffers
      Processing
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

  add_library(Sim SHARED
    ${SIM_SOURCES}
  )
  set_target_properties(Sim PROPERTIES OUTPUT_NAME "Sim")
  target_link_libraries(Sim PRIVATE stmdaq_all_deps)

endif()
