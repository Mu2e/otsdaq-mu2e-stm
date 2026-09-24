////////////////////////////////////////////////////////////////////////  
/// This module implements the FMC144 spi read/write
////////////////////////////////////////////////////////////////////////  

#ifndef FMC144_SPI_hh
#define FMC144_SPI_hh

// IPBus Manager
#include "STMDAQ-TestBeam/hardwareScripts/IPBusManager.hh"

class FMC144_spi {

public:

  // Constructor
  FMC144_spi();

  // spi read
  uint64_t spi_read(IPBusManager* hw,
		    std::string spictrlname, 
		    uint64_t slaveaddress);

  // spi write
  uint64_t spi_write(IPBusManager* hw,
		     std::string spictrlname, 
		     uint64_t slaveaddress, 
		     uint64_t data);

  // Check regsitry function
  int spi_check(IPBusManager* hw,
		std::string spictrlname, 
		uint64_t slaveaddress, 
		uint64_t bitOp, 
		uint64_t condition, 
		std::string warning);

private : 

};

// Instance of FMC144_spi functions
static FMC144_spi spi_144;

#endif
