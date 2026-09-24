cet_make_library(
  LIBRARY_NAME OpsProviderLibraryX
  SOURCE
  operation_provider.cc
  LIBRARIES PUBLIC
    ${STMDAQ_EXTRA_LIBS}
    ${PUGIXML_LIBRARY_DIR}
    UtilsLibraryX
    ConfigLibraryX
)  

# Include paths
stmdaq_target_includes(OpsProviderLibraryX)

install_headers()
