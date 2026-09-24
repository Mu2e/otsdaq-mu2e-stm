# exes/CMakeLists.txt

# ============================================================               
#                         CET BUILD                                          
# ============================================================               
if(USE_CET)
  
  cet_make_exec(  
    NAME
     main.exe
    SOURCE
     main.cc
    LIBRARIES PUBLIC
     Utils          
     Buffers
     Config
     Hardware
     Debug
     Processing
     fftw_deps
     FE
    LIBRARIES PRIVATE
     stmdaq_all_deps
     stmdaq_python_deps
   )
   
  # Put the binary in the top-level build/bin like the rest
  set_target_properties(main.exe PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin"
  )
  
  cet_make_exec(
    NAME
     run_sim.exe
    SOURCE 
     run_sim.cc
    LIBRARIES PUBLIC
     Utils          
     Config
     Buffers
     Processing
     Sim
    LIBRARIES PRIVATE
     stmdaq_all_deps      
     stmdaq_python_deps
  )
  
  cet_make_exec(
    NAME
     run_eventSim.exe
    SOURCE 
     run_eventSim.cc
    LIBRARIES PUBLIC
     Utils          
     Config
     Buffers
     Processing
     Sim
    LIBRARIES PRIVATE
     stmdaq_all_deps      
     stmdaq_python_deps    
  )
  
  cet_make_exec(
    NAME
     recieveTCP.exe
    SOURCE 
     recieveTCP.cc
  )

  install_headers()
  install_source()  

# ============================================================           #                    STANDARD CMAKE BUILD                                # ============================================================               
else()

  add_executable(main.exe
    main.cc
  )
  
  # Put the binary in the top-level build/bin like the rest
  set_target_properties(main.exe PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin"
  )

  target_link_libraries(main.exe PRIVATE
    stmdaq_all_deps
    stmdaq_python_deps
    fftw_deps
    # ---- STM_LIBS ----
    Utils
    Buffers
    Config
    Hardware
    Debug
    Processing
    FE
  )
  
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
