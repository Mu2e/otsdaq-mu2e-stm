# frontend/CMakeLists.txt

# ---- Sources (shared) ----                                                 
set(FRONTEND_SOURCES
  main.cc
)

# ============================================================               
#                         CET BUILD                                          
# ============================================================               
if(USE_CET)                                                                  
                                                                             
  cet_make_exec(  
    NAME main.exe
    SOURCE                                                                   
      ${FRONTEND_SOURCES}
    LIBRARIES PRIVATE
      Utils          
      Buffers
      Config
      Hardware
      Debug
      Processing
      stmdaq_all_deps      
      stmdaq_python_deps
      fftw_deps
    )                                                                          

  # Put the binary in the top-level build/bin like the rest
  set_target_properties(main.exe PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin"
  )

                                                                             
  install_headers()                                                          
  install_source()  

# ============================================================               
#                    STANDARD CMAKE BUILD                                    
# ============================================================               
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
  )

endif()
