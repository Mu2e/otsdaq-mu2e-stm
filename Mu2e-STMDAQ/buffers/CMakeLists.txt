set(LIB_CC
  buffer_queue.cc
  buffer_pool.cc   
)

cet_make_library(
  LIBRARY_NAME BuffersLibraryX
  SOURCE
    ${LIB_CC}
  LIBRARIES PUBLIC
    ${STMDAQ_EXTRA_LIBS}  # e.g., rt
    ${PUGIXML_LIBRARY_DIR}  
    UtilsLibraryX
    ConfigLibraryX
    OpsProviderLibraryX
  )

# Include paths
stmdaq_target_includes(BuffersLibraryX)

install_headers()
install_source()
