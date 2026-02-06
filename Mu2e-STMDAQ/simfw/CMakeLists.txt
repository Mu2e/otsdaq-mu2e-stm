set(LIB_CC
  gen_data.cc
)

cet_make_library(
  LIBRARY_NAME SimDataLibraryX
  SOURCE
    ${LIB_CC}
  LIBRARIES PUBLIC
  ${STMDAQ_EXTRA_LIBS}  # e.g., rt
  ${PUGIXML_LIBRARY_DIR}  
  UtilsLibraryX
  ConfigLibraryX
  OpsProviderLibraryX
  BuffersLibraryX
  ProcessingLibraryX
)

cet_make_exec(
  NAME run_sim
  SOURCE
    run_sim.cc
  LIBRARIES
    SimDataLibraryX
)

# Include paths
stmdaq_target_includes(SimDataLibraryX)

install_headers()
install_source()
