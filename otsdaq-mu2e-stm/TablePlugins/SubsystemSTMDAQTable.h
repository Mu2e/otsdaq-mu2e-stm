#ifndef _ots_SubsystemSTMDAQ_h_
#define _ots_SubsystemSTMDAQ_h_

#include <cstdint>
#include <map>
#include <string>

#include "otsdaq/ConfigurationInterface/ConfigurationManager.h"
#include "otsdaq/TableCore/TableBase.h"

namespace ots
{
// clang-format off
class SubsystemSTMDAQTable : public TableBase
{

  public:

	SubsystemSTMDAQTable(void);
	virtual ~SubsystemSTMDAQTable(void);

	// Methods
	void init(ConfigurationManager* configManager);
	std::string getStructureAsJSON(const ConfigurationManager* configManager) override;
	void updateXMLfiles(const ConfigurationManager* configManager);
	void generateOfflineTableMap(const ConfigurationManager* configManager);

  private:

	std::map<std::string, std::string> mapOfflineTables_;

};

}  // namespace ots
#endif
