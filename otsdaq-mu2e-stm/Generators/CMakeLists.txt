
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
#  otsdaq-mu2e-stm::Generators_Mu2eReceiverBase
#  otsdaq-mu2e-stm::ConfigLibraryX
#  otsdaq-mu2e-stm::OpsProviderLibraryX
#  otsdaq-mu2e-stm::BuffersLibraryX
#  otsdaq-mu2e-stm::DebugLibraryX
#  otsdaq-mu2e-stm::DQMLibraryX
#  otsdaq-mu2e-stm::ProcessingLibraryX
#)

cet_build_plugin(STMTCPReceiver artdaq::commandableGenerator
  LIBRARIES REG
  otsdaq-mu2e-stm::Generators_Mu2eReceiverBase
  otsdaq-mu2e-stm::Utils
  otsdaq-mu2e-stm::Config
  stmdaq_all_deps
  stmdaq_uhal_deps
)

#cet_build_plugin(STMTCPReceiverSim artdaq::commandableGenerator LIBRARIES REG otsdaq-mu2e-stm::Generators_Mu2eReceiverBase otsdaq-mu2e-stm::Utils)

#get_cmake_property(_variableNames VARIABLES)
#foreach (_variableName ${_variableNames})
#    message(STATUS "${_variableName}=${${_variableName}}")
#endforeach()

install_headers()
install_source()
