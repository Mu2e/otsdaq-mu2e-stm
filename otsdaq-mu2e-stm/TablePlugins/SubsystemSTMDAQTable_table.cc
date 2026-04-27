#include "otsdaq-mu2e-stm/TablePlugins/SubsystemSTMDAQTable.h"
#include "otsdaq/Macros/TablePluginMacros.h"
#include "Mu2e-STMDAQ/config/config.hh"

#include <cstdlib>
#include <fstream>
#include <sstream>

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

  // Get the config instance of the stmdaq
  const std::string xmlPath = getenv("STM_XML") ? getenv("STM_XML") : "";
  auto& cfg = Config::getInstance(xmlPath);

  // Iterate over rows
  for(unsigned int row = 1; row < tableView.getNumberOfRows(); ++row) {

    std::string key;
    tableView.getValue(key, row, 2);

    std::string value;
    tableView.getValue(value, row, 3);

    std::string config_str = "stm." + key;
    cfg.setValue(config_str, value);
  }

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
