
#get_cmake_property(_variableNames VARIABLES)
#foreach(_variableName ${_variableNames})
#	message(STATUS "${_variableName}=${${_variableName}}")
#endforeach()

cet_make_library(LIBRARY_NAME STMDAQSupervisor
  SOURCE
  STMDAQSupervisor.cc
  LIBRARIES PRIVATE
  otsdaq::CoreSupervisors
  otsdaq_mu2e_stm::UtilsLibraryX
  otsdaq_mu2e_stm::ConfigLibraryX
  otsdaq_mu2e_stm::OpsProviderLibraryX
  otsdaq_mu2e_stm::BuffersLibraryX
  otsdaq_mu2e_stm::DebugLibraryX
  otsdaq_mu2e_stm::DQMLibraryX
  otsdaq_mu2e_stm::ProcessingLibraryX
  otsdaq_mu2e_stm::SimDataLibraryX
)

install_headers()
install_source()
