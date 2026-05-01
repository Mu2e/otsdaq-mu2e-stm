#include "otsdaq-mu2e-stm/TablePlugins/SubsystemSTMDAQTable.h"
#include "otsdaq/Macros/TablePluginMacros.h"
#include "Mu2e-STMDAQ/config/config.hh"

#include <cstdlib>
#include <fstream>
#include <sstream>
#include <filesystem>
#include <fcntl.h>
#include <sys/file.h>

using namespace ots;

//==============================================================================
SubsystemSTMDAQTable::SubsystemSTMDAQTable(void) : TableBase("SubsystemSTMDAQTable") {}

//==============================================================================
SubsystemSTMDAQTable::~SubsystemSTMDAQTable(void) {}

void SubsystemSTMDAQTable::init(ConfigurationManager* configManager) {
  isFirstAppInContext_ = configManager->isOwnerFirstAppInContext();
  if(!isFirstAppInContext_)
	  return;

  // Update the xml files
  updateXMLfiles(configManager);

  // Generate csv
  generateOfflineTableMap(configManager);

  const std::string dbserviceOnlinePath =
      getenv("DBSERVICE_ONLINE_PATH") ? getenv("DBSERVICE_ONLINE_PATH") : "";
  if(dbserviceOnlinePath.empty())
	  return;

  // Write ?
  for(const auto& offlineTable : mapOfflineTables_)
  {
    const std::string fileName =
	dbserviceOnlinePath + "/" + offlineTable.first + ".txt";
    std::ofstream out(fileName);
    if(!out)
    {
	    __SS__ << "Failed to open file: " << fileName << __E__;
	    __SS_THROW__;
    }
    out << offlineTable.second;
  }
}

//==============================================================================
void SubsystemSTMDAQTable::updateXMLfiles(const ConfigurationManager* configManager) {

  auto table = configManager->getTable<TableBase>("SubsystemSTMDAQTable");
  const auto& tableView = table->getView();

  //Copy from source to scratch
  const std::string copyFrom = getenv("XML_PATH") ? getenv("XML_PATH") : "";
  const std::string xmlMaster = getenv("STM_XML") ? getenv("STM_XML") : "";
  std::filesystem::path p(xmlMaster);
  std::filesystem::path xmlPath = p.parent_path();
  std::filesystem::path realXmlPath = std::filesystem::canonical(xmlPath);
  std::filesystem::path realMasterPath = std::filesystem::canonical(xmlMaster);

  std::string lock_file = realXmlPath /".lock";

  int fd = open(lock_file.c_str(), O_CREAT | O_RDWR, 0666);

  if (fd == -1) {
    std::cerr << "Fatal: Could not open/create lock file.\n";
    return;
  }

  if (::flock(fd, LOCK_EX | LOCK_NB) == -1) {
    if (errno == EWOULDBLOCK) {
      // The file exists AND someone else currently holds the lock!
      std::cout << "Another DAQ process is currently copying. Skipping...\n";
      close(fd);
      return;
    } else {
      // Some other system error happened
      std::cerr << "Error acquiring lock.\n";
      close(fd);
      return;
    }
  }

  std::filesystem::copy(copyFrom, realXmlPath, std::filesystem::copy_options::recursive | std::filesystem::copy_options::overwrite_existing);

  // Get the config instance of the stmdaq
  auto& cfg = Config::getInstance(realMasterPath);

  // Iterate over rows
  for(unsigned int row = 1; row < tableView.getNumberOfRows(); ++row) {

    std::string key;
    tableView.getValue(key, row, 2);

    std::string value;
    tableView.getValue(value, row, 3);

    std::string config_str = "stm." + key;
    cfg.setValue(config_str, value);
  }

  flock(fd, LOCK_UN);
  close(fd);

}

//==============================================================================
void SubsystemSTMDAQTable::generateOfflineTableMap(
    const ConfigurationManager* configManager)
{
  mapOfflineTables_.clear();

  auto table = configManager->getTable<TableBase>("SubsystemSTMDAQTable");
  const auto& tableView = table->getView();

  std::stringstream offlineTable;
  offlineTable << "PARAMETER, VALUE" << __E__;

  for(unsigned int row = 1; row < tableView.getNumberOfRows(); ++row) {

    std::string key;
    tableView.getValue(key, row, 0);

    std::string value;
    tableView.getValue(value, row, 3);

    offlineTable << key << ", "
		 << value
		 << "\n";
  }

  mapOfflineTables_["STMDAQConfig"] = offlineTable.str();
}

//==============================================================================
std::string SubsystemSTMDAQTable::getStructureAsJSON(
    const ConfigurationManager* configManager)
{
	if(mapOfflineTables_.empty())
		generateOfflineTableMap(configManager);

	std::stringstream out;
	out << "{";

	for(auto it = mapOfflineTables_.begin(); it != mapOfflineTables_.end(); ++it)
	{
		out << "\"" << it->first << "\": \""
		    << StringMacros::escapeJSONStringEntities(it->second) << "\"";
		if(std::next(it) != mapOfflineTables_.end())
			out << ",";
	}

	out << "}";
	return out.str();
}

DEFINE_OTS_TABLE(SubsystemSTMDAQTable)
