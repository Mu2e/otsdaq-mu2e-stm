set(LIB_CC 
  checkData.cc
  formEvents.cc
  prescale.cc
  )
cet_make_library(LIBRARY_NAME ProcessLibrary
                 SOURCE
                 ${LIB_CC}
                 LIBRARIES PUBLIC
		 otsdaq_mu2e_stm::UtilsLibrary
                 rt
)

install_headers()
install_source()


