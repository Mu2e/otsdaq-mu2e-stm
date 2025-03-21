# Sources and objects
set(LIB_CC
    fe_threads.cc
    genData.cc
    test_funcs.cc
)

set(MAIN_CC
    testUDP.cc
)

set(SOURCE_CC
    ${LIB_CC}
    ${MAIN_CC}
)
  
cet_make_library(LIBRARY_NAME FELibrary
                 SOURCE
                 ${LIB_CC}
                 LIBRARIES PUBLIC
		 otsdaq_mu2e_stm::UtilsLibrary
		 otsdaq_mu2e_stm::ProcessLibrary
                 rt
	       )

cet_make_exec(NAME testUDP
  SOURCE
  ${MAIN_CC}
  LIBRARIES PUBLIC
  otsdaq_mu2e_stm::UtilsLibrary
  otsdaq_mu2e_stm::ProcessLibrary
  otsdaq_mu2e_stm::FELibrary
)	       

install_headers()
install_source()

