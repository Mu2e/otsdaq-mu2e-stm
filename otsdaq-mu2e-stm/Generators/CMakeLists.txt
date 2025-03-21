
cet_make_library(LIBRARY_NAME otsdaq-mu2e-stm_Generators_Mu2eReceiverBase
SOURCE 
Mu2eEventReceiverBase.cc
LIBRARIES PUBLIC
artdaq::DAQdata
canvas::canvas
artdaq_core_mu2e::artdaq-core-mu2e_Overlays
mu2e_pcie_utils::DTCInterface
artdaq_plugin_types::CommandableFragmentGenerator
  )

include(artdaq::commandableGenerator)
  
cet_build_plugin(STMUDPReceiver artdaq::commandableGenerator LIBRARIES REG otsdaq_mu2e_stm::otsdaq-mu2e-stm_Generators_Mu2eReceiverBase otsdaq_mu2e_stm::UtilsLibrary)

#get_cmake_property(_variableNames VARIABLES)
#foreach (_variableName ${_variableNames})
#    message(STATUS "${_variableName}=${${_variableName}}")
#endforeach()

install_headers()
install_source()
