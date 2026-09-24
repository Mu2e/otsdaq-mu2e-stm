//////////////////////////////////////////////
/// This module monitors the hardware (main)
//////////////////////////////////////////////

#include <fstream>
#include <iostream>

// Monitor HW
#include "STMDAQ-TestBeam/hardwareScripts/monitorHW.hh"

// General ADC scripts
#include "STMDAQ-TestBeam/hardwareScripts/initADC.hh"
#include "STMDAQ-TestBeam/hardwareScripts/repeat.hh"
#include "STMDAQ-TestBeam/hardwareScripts/utils.hh"

// FMC120
#include "STMDAQ-TestBeam/hardwareScripts/FMC120/FMC120.hh"
#include "STMDAQ-TestBeam/hardwareScripts/FMC120/FMC120_freqcnt.hh"
#include "STMDAQ-TestBeam/hardwareScripts/FMC120/FMC120_spi.hh"

// FMC144
#include "STMDAQ-TestBeam/hardwareScripts/FMC144/FMC144.hh"
#include "STMDAQ-TestBeam/hardwareScripts/FMC144/FMC144_freqcnt.hh"
#include "STMDAQ-TestBeam/hardwareScripts/FMC144/FMC144_monitor.hh"
#include "STMDAQ-TestBeam/hardwareScripts/FMC144/capture.hh"

// Standard constructor - shouldn't be used
monitorHW::monitorHW() {}

// Get and return all monitoring values
std::tuple<uint64_t> monitorHW::get_values(IPBusManager* hw)
{
	// Get the ADC temperature
	uint64_t temp = get_adc_temp(hw, "FMC120");

	return {temp};
}

// Get the ADC temperature
uint64_t monitorHW::get_adc_temp(IPBusManager* hw, std::string adc)
{
	// Get and return ADC temperature
	hw->write("i2c_master.i2c.byte", 0x1);
	spi_120.i2c_write(hw, 0x2F00, 0x2200);
	spi_120.i2c_write(hw, 0x2F00, 0xA000);
	uint64_t temp = 0;
	if(adc == "FMC120")
	{
		temp = spi_120.i2c_read(hw, 0x2F02);
	}

	return temp;
}
