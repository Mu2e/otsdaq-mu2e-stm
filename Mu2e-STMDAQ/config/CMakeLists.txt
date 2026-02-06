set(LIB_CC
  stm_data.cc 
)

cet_make_library(
  LIBRARY_NAME ConfigLibraryX
  SOURCE
  ${LIB_CC}
  LIBRARIES PUBLIC
  ${STMDAQ_EXTRA_LIBS}  # e.g., rt
  ${PUGIXML_LIBRARY_DIR}  
  UtilsLibraryX
)

# Include paths
stmdaq_target_includes(ConfigLibraryX)

install_headers()
install_source()
