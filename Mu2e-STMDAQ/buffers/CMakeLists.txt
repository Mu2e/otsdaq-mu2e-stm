# buffers/CMakeLists.txt

# ---- Sources (shared) ----                                                 
set(BUFFERS_SOURCES
  buffer_queue.cc
  #buffer_pool.cc
)

# ============================================================               
#                         CET BUILD                                          
# ============================================================               
if(USE_CET)                                                                  
                                                                             
  cet_make_library(                                                          
	  LIBRARY_NAME Buffers
    SOURCE                                                                   
      ${BUFFERS_SOURCES}
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
  add_library(Buffers SHARED
      ${BUFFERS_SOURCES}
  )

  set_target_properties(Buffers PROPERTIES OUTPUT_NAME "Buffers")

  target_link_libraries(Buffers PRIVATE
    stmdaq_all_deps
  )
                                                                             
endif()

