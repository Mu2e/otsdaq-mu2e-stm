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

  cet_make_exec(
    NAME run_sim.exe
    SOURCE 
      run_sim.cc
    LIBRARIES
    PRIVATE
      Sim
      stmdaq_all_deps
  )
                                                                             
  cet_make_exec(
    NAME run_eventSim.exe
    SOURCE 
      run_eventSim.cc
    LIBRARIES
    PRIVATE
      Sim
      stmdaq_all_deps
  )

  cet_make_exec(
    NAME recieveTCP.exe
    SOURCE 
    recieveTCP.cc
  )

  install_headers()                                                          
  install_source()  

# ============================================================               
#                    STANDARD CMAKE BUILD                                    
# ============================================================               
else()                                                                       

  add_library(Sim SHARED
    gen_data.cc
    send_TCP.cc
    gen_eventData.cc
  )
  set_target_properties(Sim PROPERTIES OUTPUT_NAME "Sim")
  target_link_libraries(Sim PRIVATE stmdaq_all_deps)

  add_executable(run_sim.exe
    run_sim.cc
  )

  target_link_libraries(run_sim.exe PRIVATE
    stmdaq_all_deps
    stmdaq_python_deps
    Sim
    Config    
    Utils
    Buffers
    Processing
  )

  add_executable(run_eventSim.exe
    run_eventSim.cc
  )

  target_link_libraries(run_eventSim.exe PRIVATE
    stmdaq_all_deps
    stmdaq_python_deps
    Sim
    Config    
    Utils
    Buffers
    Processing
  )


  add_executable(recieveTCP.exe
    recieveTCP.cc
  )

endif()
