# frontend/CMakeLists.txt

# ---- Sources (shared) ----
set(FE_SOURCES
  stm_frontend.cc
)

# ============================================================               
#                         CET BUILD                                          
# ============================================================               
if(USE_CET)

  cet_make_library(
    LIBRARY_NAME FE
    SOURCE
      ${FE_SOURCES}
    LIBRARIES PUBLIC
      Utils          
      Buffers
      Config
      Hardware
      Debug
      Processing
    LIBRARIES PRIVATE
      stmdaq_all_deps
      stmdaq_python_deps
      fftw_deps
    )    

    install_headers()
    install_source()  

# ============================================================               
#                    STANDARD CMAKE BUILD                                    
# ============================================================               
else()                                                                       
  add_library(FE SHARED
    ${FE_SOURCES}
  )
  set_target_properties(FE PROPERTIES OUTPUT_NAME "FE")
  target_link_libraries(FE PRIVATE stmdaq_all_deps)
  

endif()
