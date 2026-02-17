
cet_make_library(LIBRARY_NAME Generators_Mu2eReceiverBase
SOURCE 
Mu2eEventReceiverBase.cc
LIBRARIES PUBLIC
artdaq::DAQdata
canvas::canvas
artdaq-core-mu2e::Overlays
mu2e-pcie-utils::DTCInterface
artdaq_plugin_types::CommandableFragmentGenerator
  )

include(artdaq::commandableGenerator)
  
#cet_build_plugin(STMUDPReceiver artdaq::commandableGenerator
#  LIBRARIES REG
#  otsdaq_mu2e_stm::Generators_Mu2eReceiverBase
#  otsdaq_mu2e_stm::ConfigLibraryX
#  otsdaq_mu2e_stm::OpsProviderLibraryX
#  otsdaq_mu2e_stm::BuffersLibraryX
#  otsdaq_mu2e_stm::DebugLibraryX
#  otsdaq_mu2e_stm::DQMLibraryX
#  otsdaq_mu2e_stm::ProcessingLibraryX
#)

cet_build_plugin(STMTCPReceiver artdaq::commandableGenerator
  LIBRARIES REG
  otsdaq_mu2e_stm::Generators_Mu2eReceiverBase
  otsdaq_mu2e_stm::Utils
)

#cet_build_plugin(STMTCPReceiverSim artdaq::commandableGenerator LIBRARIES REG otsdaq_mu2e_stm::Generators_Mu2eReceiverBase otsdaq_mu2e_stm::UtilsLibrary)

#get_cmake_property(_variableNames VARIABLES)
#foreach (_variableName ${_variableNames})
#    message(STATUS "${_variableName}=${${_variableName}}")
#endforeach()

install_headers()
install_source()
