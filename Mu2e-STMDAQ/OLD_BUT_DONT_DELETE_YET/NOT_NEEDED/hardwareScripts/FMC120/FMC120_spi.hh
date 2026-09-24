////////////////////////////////////////////////////////////////////////
/// This module implements the FMC120 spi AND i^2c read/write
////////////////////////////////////////////////////////////////////////

#ifndef FMC120_SPI_hh
#define FMC120_SPI_hh

// IPBus Manager
#include "STMDAQ-TestBeam/hardwareScripts/IPBusManager.hh"

static const uint16_t LMK_SELECT = 0x1;
static const uint16_t DAC_SELECT = 0x2;
static const uint16_t ADC0_SELECT = 0x4;
static const uint16_t ADC1_SELECT = 0x8;
static const uint16_t ADC_SELECT_BOTH = 0xC;  

// FMC120_clocktree_init() configure the clock tree                           
// for internal clock operations                                              
static const uint CLOCKTREE_CLKSRC_INTERNAL = 0;
// FMC120_clocktree_init() configure the clock tree                           
// for external clock operations                                              
static const uint CLOCKTREE_CLKSRC_EXTERNAL = 1;
// FMC120_clocktree_init() configure the clock tree                           
// for external reference operations                                          
static const uint CLOCKTREE_CLKSRC_EXTERNAL_REF = 2;

class FMC120_spi {

public:

  // Constructor
  FMC120_spi();


  // i^2c write
  void i2c_write(IPBusManager* hw, uint32_t slaveaddr, uint32_t data);

  // i^2c read
  uint32_t i2c_read(IPBusManager* hw, uint32_t slaveaddr);

  // spi read
  uint32_t spi_read(IPBusManager* hw,
		    uint32_t spi_select,
		    uint32_t slaveaddress);

  // spi write
  uint32_t spi_write(IPBusManager* hw,
		     uint32_t spi_select,
		     uint32_t slaveaddress,
		     uint32_t data);

  

private : 

};

// Instance of FMC120_spi functions                                         
static FMC120_spi spi_120;

#endif
