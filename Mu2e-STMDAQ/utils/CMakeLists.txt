set(LIB_CC
  cpu_utils.cc
  Logger.cc
  async_logger.cc
  EnvVars.cc
  signal_handler.cc
  shm_manager.cc
)

cet_make_library(
  LIBRARY_NAME UtilsLibraryX
  SOURCE
    ${LIB_CC}
  LIBRARIES PUBLIC
    ${STMDAQ_EXTRA_LIBS}  # e.g., rt
    ${PUGIXML_LIBRARY_DIR}  
  )

# Include paths
stmdaq_target_includes(UtilsLibraryX)

install_headers()
install_source()
