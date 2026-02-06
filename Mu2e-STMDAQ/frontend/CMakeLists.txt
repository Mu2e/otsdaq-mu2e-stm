
cet_make_exec(
  NAME main
  SOURCE
  main.cc
  LIBRARIES
  ${STMDAQ_EXTRA_LIBS}  # e.g., rt
  ${PUGIXML_LIBRARY_DIR}  
  UtilsLibraryX
  ConfigLibraryX
  OpsProviderLibraryX
  BuffersLibraryX
  DebugLibraryX
  DQMLibraryX  
  SimDataLibraryX
  ProcessingLibraryX
)

install_headers()
install_source()
