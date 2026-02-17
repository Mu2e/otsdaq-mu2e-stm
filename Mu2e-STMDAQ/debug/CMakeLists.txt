# debug/CMakeLists.txt

# ---- Sources (shared) ----                                                 
set(DEBUG_SOURCES
  test_funcs.cc
)

# ============================================================               
#                         CET BUILD                                          
# ============================================================               
if(USE_CET)                                                                  
                                                                             
  cet_make_library(                                                          
	  LIBRARY_NAME Debug
    SOURCE                                                                   
      ${DEBUG_SOURCES}
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

  add_library(Debug SHARED
    test_funcs.cc
  )

  set_target_properties(Debug PROPERTIES OUTPUT_NAME "Debug")

  target_link_libraries(Debug PRIVATE
    stmdaq_all_deps
)
endif()
