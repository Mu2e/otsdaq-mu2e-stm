
#get_cmake_property(_variableNames VARIABLES)
#foreach(_variableName ${_variableNames})
#	message(STATUS "${_variableName}=${${_variableName}}")
#endforeach()

cet_make_library(LIBRARY_NAME STMDAQSupervisor
  SOURCE
  STMDAQSupervisor.cc
  LIBRARIES PRIVATE
  otsdaq::CoreSupervisors
  otsdaq_mu2e_stm::Utils
  otsdaq_mu2e_stm::Config
  otsdaq_mu2e_stm::Hardware
  otsdaq_mu2e_stm::Buffers
  otsdaq_mu2e_stm::Debug
  otsdaq_mu2e_stm::Processing
  otsdaq_mu2e_stm::Sim
  otsdaq_mu2e_stm::FE
  stmdaq_all_deps
  stmdaq_python_deps
)

install_headers()
install_source()
