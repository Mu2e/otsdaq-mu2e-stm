
#get_cmake_property(_variableNames VARIABLES)
#foreach(_variableName ${_variableNames})
#	message(STATUS "${_variableName}=${${_variableName}}")
#endforeach()

cet_make_library(LIBRARY_NAME STMDAQSupervisor
  SOURCE
  STMDAQSupervisor.cc
  LIBRARIES PRIVATE
  otsdaq::CoreSupervisors
  otsdaq-mu2e-stm::Utils
  otsdaq-mu2e-stm::Config
  otsdaq-mu2e-stm::Hardware
  otsdaq-mu2e-stm::Buffers
  otsdaq-mu2e-stm::Debug
  otsdaq-mu2e-stm::Processing
  otsdaq-mu2e-stm::Sim
  otsdaq-mu2e-stm::FE
  stmdaq_all_deps
  stmdaq_uhal_deps
)

install_headers()
install_source()
