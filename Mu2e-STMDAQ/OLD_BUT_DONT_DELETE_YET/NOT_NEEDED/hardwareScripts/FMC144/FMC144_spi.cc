////////////////////////////////////////////////////////////////////////
/// This module implements the FMC144 spi read/write
////////////////////////////////////////////////////////////////////////

#include <unistd.h>

// FMC144_spi
#include "STMDAQ-TestBeam/hardwareScripts/FMC144/FMC144_spi.hh"

//Standard constructor - shouldn't be used
FMC144_spi::FMC144_spi() {}

// spi read
uint64_t FMC144_spi::spi_read(IPBusManager* hw,
				 std::string spictrlname, 
				 uint64_t slaveaddress){

  int spi_busy = 1;
  std::string node;

  node = "axi_fmc144_8lane.spi_ctrl."+spictrlname+".spi_address";
  hw->write(node,slaveaddress);
  node = "axi_fmc144_8lane.spi_ctrl."+spictrlname+".spi_command_status";
  hw->write(node,0x02);

  while ((spi_busy & 0x0003) != 0x0002){
    
    node = "axi_fmc144_8lane.spi_ctrl."+spictrlname+".spi_command_status";
    uint64_t value = hw->read(node);
    spi_busy = int(value);
    usleep(20000); // 0.02 secs
    
  }
  
  node = "axi_fmc144_8lane.spi_ctrl."+spictrlname+".spi_read_data";
  uint64_t data = hw->read(node);

  return data;

}

// spi write
uint64_t FMC144_spi::spi_write(IPBusManager* hw,
				  std::string spictrlname, 
				  uint64_t slaveaddress, 
				  uint64_t data){

  int spi_busy = 1;
  std::string node;

  node = "axi_fmc144_8lane.spi_ctrl."+spictrlname+".spi_address";
  hw->write(node,slaveaddress);
  node = "axi_fmc144_8lane.spi_ctrl."+spictrlname+".spi_write_data";
  hw->write(node,data);
  node = "axi_fmc144_8lane.spi_ctrl."+spictrlname+".spi_command_status";
  hw->write(node,0x01);

  while (spi_busy & 0x01){

    node = "axi_fmc144_8lane.spi_ctrl."+spictrlname+".spi_command_status";
    uint64_t value = hw->read(node);
    spi_busy = int(value);
    usleep(20000); // 0.02 secs

  }

  return 1;

}

// Function to check, ask for user input to continue and if not, hard stop
int FMC144_spi::spi_check(IPBusManager* hw,
			     std::string spictrlname, 
			     uint64_t slaveaddress, 
			     uint64_t bitOp, 
			     uint64_t condition, 
			     std::string warning){

  // Get registry value to be checked
  uint64_t dword = spi_read(hw,spictrlname,slaveaddress);
  bool check; // Boolean for check condition
  // Does the condition include a bitwise operator?
  if (bitOp == NULL){
    check = (dword == condition) ? true : false;
  }
  else{
    check = ((dword & bitOp) == condition) ? true : false;
  }
  int count = 0; // Initialise loop counter
  // While registry value is not what expected...
  while (!check){
    // Output warning about status...
    std::cout << warning+" Waiting..." << std::endl;
    // Sleep for 1 second
    usleep(1e6);
    // Check registry value again
    dword = spi_read(hw,spictrlname,slaveaddress);
    if (bitOp == NULL){
      check = (dword == condition) ? true : false;
    }
    else{
      check = ((dword & bitOp) == condition) ? true : false;
    }
    count += 1; // Increase loop counter
    // If loop counter reaches 10 attempts...
    if (count == 10){
      // Ask for user input to continue or stop
      std::string answer;
      std::cout << "Unsuccesful. Continue? (Y/N)" << std::endl;
      std::cin >> answer;
      // Ensure input from user is Y/N
      while (answer != "Y" and answer != "N" and answer != "y" and answer != "n"){
	std::cout << "Input not recognised... Continue? (Y/N)" << std::endl;
	std::cin >> answer;
      }
      // Is user does not want to continue, hard stop!
      if (answer == "N" or answer == "n") {
	printf("Stopping...\n");
	exit(0);
      }
      // Re-initialise loop counter
      count = 0;
    }
  }

  return 1;
  
}
