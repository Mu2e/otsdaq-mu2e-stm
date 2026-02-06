set(LIB_CC
  dqm_manager.cc 
)

cet_make_library(
  LIBRARY_NAME DQMLibraryX
  SOURCE
    ${LIB_CC}
  LIBRARIES PUBLIC
    ${STMDAQ_EXTRA_LIBS}  # e.g., rt
    ${PUGIXML_LIBRARY_DIR}  
    UtilsLibraryX
    ConfigLibraryX
  )

# Include paths
stmdaq_target_includes(DQMLibraryX)

install_headers()
install_source()
