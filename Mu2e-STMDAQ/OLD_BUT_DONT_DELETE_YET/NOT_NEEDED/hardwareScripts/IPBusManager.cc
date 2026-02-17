// Hardware scripts
#include "STMDAQ-TestBeam/hardwareScripts/IPBusManager.hh"

//Standard constructor - shouldn't be used
IPBusManager::IPBusManager() {
}

//initialise connection manager and HW interface
IPBusManager::IPBusManager(std::string conn_file, std::string dev_id) 
  : connection_file(conn_file)
  , device_id(dev_id)
  , manager()
  , hw()
{

  //add file:// prefix to connection file name as required
  std::stringstream conn_id;
  conn_id << "file://" << conn_file;

  try {
    //Create connection manager
    manager.reset( new ConnectionManager ( conn_id.str() ) );
    //Create interface to device
    hw.reset( new HwInterface ( manager->getDevice( dev_id ) ) );
  }
  catch ( const std::exception & e ) { std::cout << "uhal error occurred" << std::endl; }

}

ConnectionManager* IPBusManager::getConnectionManager() const {
  return manager.get();
}

HwInterface* IPBusManager::getHwInterface() const {
  return hw.get();
}

// Function to get a vector std::string of all nodes in a device  
std::vector<std::string> IPBusManager::getAllNodes() {

  std::vector<std::string> nodes;
  try {

    nodes = getHwInterface()->getNodes();
    sort( nodes.begin(), nodes.end() );

  }
  catch ( const std::exception & e ) { std::cout << "uhal error occurred" << std::endl; }

  return nodes;
}

// Function to read single registry value of a given node  
uint64_t IPBusManager::read(std::string node) {

  uint64_t regVal = 0;

  try {

    ValWord< uint32_t > nodeVal = getHwInterface()->getNode(node).read();
    getHwInterface()->dispatch();
    regVal = nodeVal.value();

  }
  catch ( const std::exception & e ) { std::cout << "uhal error occurred" << std::endl; }

  return regVal;

}

// Function to write to single registry value of a given node  
void IPBusManager::write(std::string node, uint64_t value) {

  try {

    uint64_t nodeValInit = read(node);
    
    getHwInterface()->getNode(node).write(value);
    getHwInterface()->dispatch();

    //check the write worked
    uint64_t nodeVal = read(node);

  }
  catch ( const std::exception & e ) { std::cout << "uhal error occurred" << std::endl; }

  return;

}

// Function to read whole registry block
std::vector<uint32_t> IPBusManager::readBlock(std::string node) {

  std::vector<uint32_t> block;
  
  try {
    
    ValVector< uint32_t > blockVal = getHwInterface()->getNode(node).readBlock(getHwInterface()->getNode(node).getSize());
    getHwInterface()->dispatch();
    block = blockVal.value();

  }
  catch ( const std::exception & e ) { std::cout << "uhal error occurred" << std::endl; }

  return block;

}

// Set UHAL logging level to maximum
void IPBusManager::maxUhalLogging(){
  
  try{
    
    setLogLevelTo(uhal::Debug()); // Use Debug() for max logging
  }
  catch ( const std::exception & e ) { std::cout << "uhal error occurred" << std::endl; }
  
  return;
  
}


// Set UHAL logging level to notice
void IPBusManager::noticeUhalLogging(){
  
  try{
    
    setLogLevelTo(uhal::Notice()); // Notice level
  }
  catch ( const std::exception & e ) { std::cout << "uhal error occurred" << std::endl; }
  
  return;
  
}


// Set UHAL logging level to minimum
void IPBusManager::minUhalLogging(){
  
  try{
    
    setLogLevelTo(uhal::Error()); // Use Error() for min logging
  }
  catch ( const std::exception & e ) { std::cout << "uhal error occurred" << std::endl; }
  
  return;
  
}
