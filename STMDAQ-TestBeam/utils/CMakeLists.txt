set(LIB_CC 
  ziggurat.cc 
  UDPsocket.cc 
  queue.cc 
  queue_write.cc 
  EnvVars.cc 
  dataVars.cc 
  Random.cc 
  Logger.cc
  )
cet_make_library(LIBRARY_NAME UtilsLibrary
                 SOURCE
                 ${LIB_CC}
                 LIBRARIES PUBLIC
                 rt
)

install_headers()
install_source()


