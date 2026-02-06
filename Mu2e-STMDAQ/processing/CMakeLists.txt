set(LIB_CC
  template_module.cc
  udp.cc
  check_data.cc
  form_events.cc
  baseline.cc
  zero_suppress.cc
  mwd.cc prescale.cc
  event_builder.cc
  write_manager.cc
  operation_manager.cc
  thread_manager.cc 
)

cet_make_library(
  LIBRARY_NAME ProcessingLibraryX
  SOURCE
    ${LIB_CC}
  LIBRARIES PUBLIC
  ${STMDAQ_EXTRA_LIBS}  # e.g., rt
  ${PUGIXML_LIBRARY_DIR}  
  UtilsLibraryX
  ConfigLibraryX
  BuffersLibraryX
  DebugLibraryX
  DQMLibraryX  
)

# Include paths
stmdaq_target_includes(ProcessingLibraryX)

install_headers()
install_source()
