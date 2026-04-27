cet_build_plugin(STMDQMJsonProducer art::module
  REG_SOURCE
    STMDQMJsonProducer_module.cc
  LIBRARIES REG
  art_root_io::TFileService_service
  artdaq-core-mu2e::Data
  Offline::Mu2eUtilities
  Offline::ProditionsService
  Offline::DataProducts
  Offline::RecoDataProducts  
)

install_headers()
install_source()
