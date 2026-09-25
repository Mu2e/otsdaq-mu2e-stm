include(otsdaq::table)

cet_build_plugin(SubsystemSTMDAQTable otsdaq::table
  LIBRARIES REG
  otsdaq::SlowControlsTableBase
  otsdaq::XDAQContextTable
  LIBRARIES PRIVATE
  otsdaq-mu2e-stm::Config
  stmdaq_all_deps
)


install_headers()
install_source()
