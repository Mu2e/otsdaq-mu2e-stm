set(LIB_CC
 test_funcs.cc  
)

cet_make_library(
  LIBRARY_NAME DebugLibraryX
  SOURCE
    ${LIB_CC}
  LIBRARIES PUBLIC
    ${STMDAQ_EXTRA_LIBS}  # e.g., rt
    ${PUGIXML_LIBRARY_DIR}  
    UtilsLibraryX
    ConfigLibraryX
  )

# Include paths
stmdaq_target_includes(DebugLibraryX)

install_headers()
install_source()
