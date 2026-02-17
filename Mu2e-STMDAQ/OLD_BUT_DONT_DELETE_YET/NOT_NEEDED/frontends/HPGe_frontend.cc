/********************************************************************\

  Name:         HPGe_frontend.cxx
  Created by:   Alex Keshavarzi

  Contents:     Experiment specific readout code (user part) of
                Midas frontend. 

\********************************************************************/

#undef NDEBUG // midas required assert() to be always enabled

#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <assert.h> // assert()

#include "midas.h"
//#include "experim.h"
#include "mfe.h"

// Hex reader
#include "STMDAQ-TestBeam/utils/Hex.hh"

// uhal
#include "uhal/uhal.hpp"
#include "uhal/ConnectionManager.hpp"

// Hardware scripts
#include "STMDAQ-TestBeam/hardwareScripts/IPBusManager.hh"
#include "STMDAQ-TestBeam/hardwareScripts/KCU105.hh"

// UDP
#include "STMDAQ-TestBeam/utils/UDPsocket.hh"

// Extract data                                                             
#include "STMDAQ-TestBeam/frontends/extractData.hh"

// Data variables                                                                               
#include "STMDAQ-TestBeam/utils/dataVars.hh"

// XML interface
#include "STMDAQ-TestBeam/utils/xml.hh"
#include "STMDAQ-TestBeam/utils/EnvVars.hh"

// Binary file code
#include "STMDAQ-TestBeam/utils/BinaryFile.hh"

using namespace std;
using namespace uhal;

// Define classes
class KCU105 kcu105; // Hardware class
class UDPsocket udps; // UDP socket class
extractData getData; // Extract data class   

/*-- Globals -------------------------------------------------------*/

/* The frontend name (client name) as seen by other MIDAS clients   */
const char *frontend_name = "HPGe";
/* The frontend file name, don't change it */
const char *frontend_file_name = __FILE__;

/* frontend_loop is called periodically if this variable is TRUE    */
BOOL frontend_call_loop = FALSE;

/* a frontend status page is displayed with this frequency in ms */
//INT display_period = 3000;
INT display_period = 0;

/* maximum event size produced by this frontend */
INT max_event_size = 10000;

/* maximum event size for fragmented events (EQ_FRAGMENTED) */
INT max_event_size_frag = 5 * 1024 * 1024;

/* buffer size to hold events */
INT event_buffer_size = 100 * 10000;

/*-- Function declarations -----------------------------------------*/

INT frontend_init(void);
INT frontend_exit(void);
INT begin_of_run(INT run_number, char *error);
INT end_of_run(INT run_number, char *error);
INT pause_run(INT run_number, char *error);
INT resume_run(INT run_number, char *error);
INT frontend_loop(void);

INT read_data(char *pevent, INT off);

int16_t *formTriggerHeader(int16_t *headerData, uint16_t dataType);
void checkPacketHeader(uint16_t pHdrEnd);
void checkTriggerHeader(uint16_t *tHdrStart);

INT poll_event(INT source, INT count, BOOL test);
INT interrupt_configure(INT cmd, INT source, POINTER_T adr);

/*-- Equipment list ------------------------------------------------*/

BOOL equipment_common_overwrite = TRUE;

EQUIPMENT equipment[] = {

   {"HPGE",               /* equipment name */
      {1, 0,                 /* event ID, trigger mask */
         "SYSTEM",           /* event buffer */
         EQ_POLLED,          /* equipment type */
         0,                  /* event source */
         "MIDAS",            /* format */
         TRUE,               /* enabled */
         RO_RUNNING |        /* read only when running */
         RO_ODB,             /* and update ODB */
         100,                /* poll for 100ms */
         0,                  /* stop run after this event limit */
         0,                  /* number of sub events */
         0,                  /* don't log history */
         "", "", "",},
      read_data,    /* readout routine */
   },

   {""}
};

// Declare IPBusManager
IPBusManager* hw = new IPBusManager();

// Define the XML reader instance
Xml *xml_file;

// Emulating data boolean
bool simulation = true;
bool emulateADC = true;

// UDP socket number
int sock;

/********************************************************************\
              Callback routines for system transitions

  These routines are called whenever a system transition like start/
  stop of a run occurs. The routines are called on the following
  occations:

  frontend_init:  When the frontend program is started. This routine
                  should initialize the hardware.

  frontend_exit:  When the frontend program is shut down. Can be used
                  to releas any locked resources like memory, commu-
                  nications ports etc.

  begin_of_run:   When a new run is started. Clear scalers, open
                  rungates, etc.

  end_of_run:     Called on a request to stop a run. Can send
                  end-of-run event and close run gates.

  pause_run:      When a run is paused. Should disable trigger events.

  resume_run:     When a run is resumed. Should enable trigger events.
\********************************************************************/

/*-- Frontend Init -------------------------------------------------*/

INT frontend_init()
{
  /* put any hardware initialization here */
  
  // Open the XML file and instantiate the xml_file object
  std::string xml_path = EnvVars::expand("${STM_XML}");
  std::cout << "XML_PATH = " << xml_path << std::endl;
  xml_file = new Xml(xml_path);

  // Check experimental configuration varaibles
  if (checkExpConfig(exp) == 0){
    cout << "Exiting due to bad value in " << xml_path << endl;
    exit(0);
  }
  cout << "Parameters in " << xml_path << " are okay!" << endl;
  
  // Create the instance to write the binary files
  BinaryFile* bf = new BinaryFile();
  getData.setBF(bf);

  int status, size;
  char str[256];
  printf("frontend_init!\n");
  
  set_equipment_status(equipment[0].name, "Initialising...", "yellow");

  printf("Acquiring data source...\n");
  
  // Create and bind UDP socket
  sock = udps.bindSocket(udps.createSocket());
  
  if (!simulation){
    
    printf("Connecting device through IPBusManager...");
    hw = new IPBusManager(hw->getConnectionFile(),hw->getDeviceID());

    // Initialise ADC
    if (kcu105.init(hw) == SUCCESS){

      // Set firmware registers from xml config file
      kcu105.setRegisters(hw);

      // Start and reset the external clock counter   
      kcu105.startResetClock(hw);

      // Reset trigger FIFOs
      kcu105.resetTriggerFIFOs(hw);

      // Set the 10G readout mode
      kcu105.start10Greadout(hw);

      // Reset and initliaise DDR4 read
      kcu105.reset_DDR4(hw);

      // If emulating data from the ADC, switch on ADC emulation
      if (emulateADC) {
	printf("Emulating ADC data in firmware...\n");
	kcu105.emulateADC(hw);
      }

      // Enable triggered data-taking
      kcu105.enable_trigData(hw);                                             
                 
      // Start continous 10g packet sending
      kcu105.start10Gpackets(hw);

      // Succesful initialisation
      printf("Hardware initialisation succesful!\n");

      //Initialize ODB structures for this frontned
      status = cm_get_experiment_database(&hDB, NULL); //Get ODB data base handle                 

      set_equipment_status(equipment[0].name, "OK", "green");

      return SUCCESS;

    }
  }
  else if (simulation){

    // Print to screen
    printf("Simulating data in software...\n");
    
    // Kill instance of simulation running in a different screen
    printf("Killing any previous instance of simulatin...\n");
    string killSimCmd = "screen -S HPGeSim -X quit";
    const char* killSim = killSimCmd.c_str();
    system(killSim);

    //Initialize ODB structures for this frontend
    status = cm_get_experiment_database(&hDB, NULL); //Get ODB data base handle                   
    set_equipment_status(equipment[0].name, "OK", "green");

    return SUCCESS;

  }

  /* print message and return FE_ERR_HW if frontend should not be started */
  printf("Error: data source not found! Exiting...\n");
  exit(0);
  return FE_ERR_HW;

}

/*-- Frontend Exit -------------------------------------------------*/

INT frontend_exit()
{
  
  // Stop ADC emulation
  if (emulateADC) kcu105.stopEmulateADC(hw);

  // Stop simulation
  if (simulation){
    // Kill instance of simulation running in a different screen
    string killSimCmd = "screen -S HPGeSim -X quit";
    const char* killSim = killSimCmd.c_str();
    system(killSim);
  }

  // Disable triggered data taking
  kcu105.disable_trigData(hw);

  // Stop 10G packet sending
  kcu105.stop10Gpackets(hw);

  // Stop 10G readout
  kcu105.stop10Greadout(hw);
  
  // Delete the instance of the xml file
  delete xml_file;
  
  // Delete the instance of the binary file
  delete getData._bf;
  
  return SUCCESS;

}

/*-- Begin of Run --------------------------------------------------*/

INT begin_of_run(INT run_number, char *error)
{
  /* put here clear scalers etc. */

  int midas_run_number = run_number;

  // Save xml run to odb directory with run number
  string runXML = "$STMDAQ_ROOT/odb/stm_run"+to_string(run_number)+".xml";
  string saveXMLcmd = "cp "+xml_path+" "+runXML;
  const char* saveXML = saveXMLcmd.c_str();
  system(saveXML);

  // Get max subrun size from XML
  int max_binary_file_size = xml_file->int_value("stm.max_binary_file_size",100); // in MB
  std::cout << "Max max_binary_file_size (int) = " << max_binary_file_size << \
    std::endl;
  int Mbytes = 1048576;
  unsigned int max_binary_size = (unsigned int) max_binary_file_size * (unsigned int) Mbytes;
  std::string binary_file = xml_file->value("stm.raw_binary_filename");
  std::cout << "Max Binary File Size  = " << max_binary_size << " bytes " << \
    std::endl;
  std::cout << "Binary File           = " << binary_file << std::endl;
  
  if (max_binary_size >= INT32_MAX) {
    std::cout << "Max binary file size has to be less than 2 Gb, else int32 i\
ssues, EXITING" << std::endl;
    exit(-1);
  }

  getData._bf->set_subrun_filesize(max_binary_size);
  getData._bf->open_raw_output_file(binary_file, midas_run_number, 0);

  // If simulating data in software...
  if (simulation){
    // Run simulation in screen
    printf("Starting simulation...\n");
    string startSimCmd = "screen -dmS HPGeSim bash -c 'cd $STMDAQ_ROOT; source setup.sh; ./build/bin/HPGeSim.exe;'";
    const char* startSim = startSimCmd.c_str();
    system(startSim);
  }

   return SUCCESS;

}

/*-- End of Run ----------------------------------------------------*/

INT end_of_run(INT run_number, char *error)
{
  
  getData._bf->close_output_file();

   return SUCCESS;
}

/*-- Pause Run -----------------------------------------------------*/

INT pause_run(INT run_number, char *error)
{
   return SUCCESS;
}

/*-- Resuem Run ----------------------------------------------------*/

INT resume_run(INT run_number, char *error)
{
   return SUCCESS;
}

/*-- Frontend Loop -------------------------------------------------*/

INT frontend_loop()
{
   /* if frontend_call_loop is true, this routine gets called when
      the frontend is idle or once between every event */
   return SUCCESS;
}

/*------------------------------------------------------------------*/

/********************************************************************\

  Readout routines for different events

\********************************************************************/

/*-- Trigger event routines ----------------------------------------*/

INT poll_event(INT source, INT count, BOOL test)
/* Polling routine for events. Returns TRUE if event
   is available. If test equals TRUE, don't return. The test
   flag is used to time the polling */
{
   int i;
   DWORD flag;

   for (i = 0; i < count; i++) {
      /* poll hardware and set flag to TRUE if new event is available */
      flag = TRUE;

      if (flag)
         if (!test)
            return TRUE;
   }

   return 0;
}

/*-- Interrupt configuration ---------------------------------------*/

INT interrupt_configure(INT cmd, INT source, POINTER_T adr)
{
   switch (cmd) {
   case CMD_INTERRUPT_ENABLE:
      break;
   case CMD_INTERRUPT_DISABLE:
      break;
   case CMD_INTERRUPT_ATTACH:
      break;
   case CMD_INTERRUPT_DETACH:
      break;
   }
   return SUCCESS;
}

/*-- Event readout -------------------------------------------------*/

INT read_data(char *pevent, INT off)
{
  
  // Data array
  int16_t *pdata;

  /* init bank structure */
  bk_init(pevent);
  
  /* create structured ADC0 bank */
  bk_create(pevent, "HPGe", TID_UINT16, (void **)&pdata);

  // FOR DEBUG - check DDR4 read/write address is incrementing
  // if (!simulation){
  //     uint32_t ddr4Addr = kcu105.getDDR4addr(hw);
  //     cout << "DDR4 address = " << HEX(ddr4Addr,8) << endl;
  // }
  
  // Get 10G ethernet packet
  struct packet p;
  udps.getPacket(p,sock);

  // memcopy packet + trigger header to MIDAS bank
  // Headers are uint16_t
  // Packet header = 3 x int16, trigger header = 19 x int16 = 38 bytes
  int16_t hdrSize = fw_pHdr_Size + fw_tHdr_Size;
  memcpy(pdata,p.data,hdrSize);
  // Increment data to just store packet + trigger header (19 words)
  pdata+=hdrSize/2; 

  getData.extractPacket(p.data,p.size);

  // Close bank
  bk_close(pevent, pdata);
  
  return bk_size(pevent);
  
}
