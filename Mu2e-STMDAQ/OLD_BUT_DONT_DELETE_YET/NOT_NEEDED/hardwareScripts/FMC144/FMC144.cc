////////////////////////////////////////////////////////////////////////
/// This module configures the FMC144 chips using FMC144_x sub-modules.
////////////////////////////////////////////////////////////////////////

// FMC144.hh 
#include "STMDAQ-TestBeam/hardwareScripts/FMC144/FMC144.hh"

// FMC144_spi read/write
#include "STMDAQ-TestBeam/hardwareScripts/FMC144/FMC144_spi.hh"

//Standard constructor - shouldn't be used
FMC144::FMC144() {}

// Initialise clocktree  
void FMC144::clocktree_init(IPBusManager* hw, int clockmode){
  
  uint64_t dword;

  if (clockmode == getClockMode("DDS")){
    
    uint64_t long ftw = 104145741382943;
    double freq = 370.0;
    double fs = 1000.0;
    
    ftw = long((1UL << 48) * (freq/fs));
    std::cout << ftw << std::endl;
    //Enable SDO, Soft reset (not self clearing)
    spi_144.spi_write(hw,"AD9912_CTRL",0x000,0xBD);
    //Clear Soft reset
    spi_144.spi_write(hw,"AD9912_CTRL",0x000,0x99);
    //Read part ID LSB
    uint64_t commandword = 0x002;
    dword = spi_144.spi_read(hw,"AD9912_CTRL",commandword);
    std::cout << HEX(dword,8) << std::endl;
    uint64_t partId = dword;
    //Read part ID MSB
    dword = spi_144.spi_read(hw,"AD9912_CTRL",0x003);
    std::cout << HEX(dword,8) << std::endl;
    partId = dword + (dword<<8);
    // std::cout << partId << std::endl;
    //Part ID should be 0x1902 
    if (partId == 0x1902 or partId == 0x1982){   
      printf("DDS part ID: %5.4lX (OK)\n",partId);
    }
    else{
      printf("DDS part ID: %5.4lX (ERROR)\n",partId);
      //Enable the HSTL output, disable all power down features
      spi_144.spi_write(hw,"AD9912_CTRL",0x010,0x00);
      //Write FTW
      spi_144.spi_write(hw,"AD9912_CTRL",0x1A6,(0xFF & (ftw >>0)));
      spi_144.spi_write(hw,"AD9912_CTRL",0x1A7,(0xFF & (ftw >>8)));
      spi_144.spi_write(hw,"AD9912_CTRL",0x1A8,(0xFF & (ftw >>16)));
      spi_144.spi_write(hw,"AD9912_CTRL",0x1A9,(0xFF & (ftw >>24)));
      spi_144.spi_write(hw,"AD9912_CTRL",0x1AA,(0xFF & (ftw >>32)));
      spi_144.spi_write(hw,"AD9912_CTRL",0x1AB,(0xFF & (ftw >>40)));
      //Update must be written last
      spi_144.spi_write(hw,"AD9912_CTRL",0x005,0x01);
    }
  }
    
  spi_144.spi_write(hw,"LMK04828_CTRL",0x000,0x90);
  //  sleep(0.200);
  usleep(200000.0); // 0.2 secs
  //
  spi_144.spi_write(hw,"LMK04828_CTRL",0x000,0x10);
  //  sleep(0.200);
  usleep(200000.0); // 0.2 secs
  //
  spi_144.spi_write(hw,"LMK04828_CTRL",0x148,0x00);
  //  sleep(0.200);
  usleep(200000.0); // 0.2 secs
  //
  spi_144.spi_write(hw,"LMK04828_CTRL",0x15F,0x3B); //STATUS_LD1_MUX LD1 = SPI_MISO
  //  sleep(0.200);
  usleep(200000.0); // 0.2 secs
  //
  spi_144.spi_write(hw,"LMK04828_CTRL",0x002,0x00); //POWERDOWN enabled
  //sleep(0.200)
  usleep(200000.0); // 0.2 secs
  
  //DCLKout0/SDCLKout1 configuration
  spi_144.spi_write(hw,"LMK04828_CTRL",0x100,0x08); //DIV_CLKOUT1_0 DIV_BY_7 IDL/ODL ==>In/Out Drive level
  spi_144.spi_write(hw,"LMK04828_CTRL",0x101,0x22);
  spi_144.spi_write(hw,"LMK04828_CTRL",0x103,0x05);
  spi_144.spi_write(hw,"LMK04828_CTRL",0x104,0x62);
  spi_144.spi_write(hw,"LMK04828_CTRL",0x105,0x00);
  spi_144.spi_write(hw,"LMK04828_CTRL",0x106,0xB0);
  spi_144.spi_write(hw,"LMK04828_CTRL",0x107,0x11); // FMT_CLK1_0 LVDS,LVDS !DCLK_INV , !DCLK_INV
  
   //DCLKout2/SDCLKout3 configuration
  spi_144.spi_write(hw,"LMK04828_CTRL",0x108,0x28);
  spi_144.spi_write(hw,"LMK04828_CTRL",0x109,0x22);
  spi_144.spi_write(hw,"LMK04828_CTRL",0x10B,0x05);
  spi_144.spi_write(hw,"LMK04828_CTRL",0x10C,0x62); //DIG_DLY_SCLK3
  spi_144.spi_write(hw,"LMK04828_CTRL",0x10D,0x13);
  spi_144.spi_write(hw,"LMK04828_CTRL",0x10E,0xB0);
  spi_144.spi_write(hw,"LMK04828_CTRL",0x10F,0x55); //FMT_CLK3_2 LVPECL,LVPECL, !DCLK_INV, !SCLK_INV
  
   //DCLKout4/SDCLKout5 configuration (ADC0)
  spi_144.spi_write(hw,"LMK04828_CTRL",0x110,0x08);
  spi_144.spi_write(hw,"LMK04828_CTRL",0x111,0x22);
  spi_144.spi_write(hw,"LMK04828_CTRL",0x113,0x05);
  spi_144.spi_write(hw,"LMK04828_CTRL",0x114,0x62);
  spi_144.spi_write(hw,"LMK04828_CTRL",0x115,0x00);
  spi_144.spi_write(hw,"LMK04828_CTRL",0x116,0xB0);
  spi_144.spi_write(hw,"LMK04828_CTRL",0x117,0x15); //DIV_CLKOUT7_6 // 16 to enable sysref
  
   //DCLKout6/SDCLKout7 configuration (ADC1)
  spi_144.spi_write(hw,"LMK04828_CTRL",0x118,0x08);
  spi_144.spi_write(hw,"LMK04828_CTRL",0x119,0x22);
  spi_144.spi_write(hw,"LMK04828_CTRL",0x11B,0x05);
  spi_144.spi_write(hw,"LMK04828_CTRL",0x11C,0x62);
  spi_144.spi_write(hw,"LMK04828_CTRL",0x11D,0x00);
  spi_144.spi_write(hw,"LMK04828_CTRL",0x11E,0xB0); //PD_CLK7_6 EN_DCLK, EN_SCLK
  spi_144.spi_write(hw,"LMK04828_CTRL",0x11F,0x15);
  
   //DCLKout8/SDCLKout9 configuration 
  spi_144.spi_write(hw,"LMK04828_CTRL",0x120,0x08); //DIV_CLKOUT9_8 (divided by 7) 
  spi_144.spi_write(hw,"LMK04828_CTRL",0x121,0x22); //DIG_DLY_DCLK8
  spi_144.spi_write(hw,"LMK04828_CTRL",0x123,0x05); //ANA_DLY_DCLK8
  spi_144.spi_write(hw,"LMK04828_CTRL",0x124,0x62);
  spi_144.spi_write(hw,"LMK04828_CTRL",0x125,0x00);
  spi_144.spi_write(hw,"LMK04828_CTRL",0x126,0xB1);
  spi_144.spi_write(hw,"LMK04828_CTRL",0x127,0x11);
  
   //DCLKout10/SDCLKout11 configuration 
  spi_144.spi_write(hw,"LMK04828_CTRL",0x128,0x08); //DIV_CLKOUT9_8 (divided by 7)
  
  spi_144.spi_write(hw,"LMK04828_CTRL",0x129,0x22);
  spi_144.spi_write(hw,"LMK04828_CTRL",0x12B,0x05);
  spi_144.spi_write(hw,"LMK04828_CTRL",0x12C,0x62);
  spi_144.spi_write(hw,"LMK04828_CTRL",0x12D,0x00);
  spi_144.spi_write(hw,"LMK04828_CTRL",0x12E,0xB1);
  spi_144.spi_write(hw,"LMK04828_CTRL",0x12F,0x11);
   //DCLKout12/SDCLKout13 configuration 
  
  spi_144.spi_write(hw,"LMK04828_CTRL",0x130,0x08); //DIV_CLKOUT13_12
  spi_144.spi_write(hw,"LMK04828_CTRL",0x131,0x22);
  spi_144.spi_write(hw,"LMK04828_CTRL",0x133,0x05);
  spi_144.spi_write(hw,"LMK04828_CTRL",0x134,0x62);
  spi_144.spi_write(hw,"LMK04828_CTRL",0x135,0x00);
  spi_144.spi_write(hw,"LMK04828_CTRL",0x136,0xB1);
  spi_144.spi_write(hw,"LMK04828_CTRL",0x137,0x11);
  
  spi_144.spi_write(hw,"LMK04828_CTRL",0x138,0x00); //VCO_OSC_OUT VCO0 (Low speed), PLL1_FB=OSCin, OSSTD::COUT=input
  spi_144.spi_write(hw,"LMK04828_CTRL",0x139,0x03); //SYSREF_MUX : SYSREF continuous
  spi_144.spi_write(hw,"LMK04828_CTRL",0x13A,0x01);
  spi_144.spi_write(hw,"LMK04828_CTRL",0x13B,0x00);
  
  spi_144.spi_write(hw,"LMK04828_CTRL",0x13C,0x07);
  spi_144.spi_write(hw,"LMK04828_CTRL",0x13D,0x0E);
  spi_144.spi_write(hw,"LMK04828_CTRL",0x13E,0x03);
  spi_144.spi_write(hw,"LMK04828_CTRL",0x13F,0x00); //FB_CTRL PLL2_FB=prescaler, PLL1_FB=OSCIN
  spi_144.spi_write(hw,"LMK04828_CTRL",0x140,0x01);
  spi_144.spi_write(hw,"LMK04828_CTRL",0x141,0x00);
  spi_144.spi_write(hw,"LMK04828_CTRL",0x142,0x00);
  spi_144.spi_write(hw,"LMK04828_CTRL",0x143,0x50);
  spi_144.spi_write(hw,"LMK04828_CTRL",0x144,0xFF);
  spi_144.spi_write(hw,"LMK04828_CTRL",0x145,0x7F);
  spi_144.spi_write(hw,"LMK04828_CTRL",0x146,0x00);
  
  if (clockmode == getClockMode("EXTERNAL") 
      or clockmode == getClockMode("EXTERNAL_REF") 
      or clockmode == getClockMode("DDS")){
    spi_144.spi_write(hw,"LMK04828_CTRL",0x147,0x1A);
  }
  else{ //internal clock
    spi_144.spi_write(hw,"LMK04828_CTRL",0x147,0x2A); //CLKIN_MUX CLKIN: !INVERT,Auto, CLKIN1=PLL1, CLKIN0=PLL1
  }
  
  spi_144.spi_write(hw,"LMK04828_CTRL",0x148,0x00);
  spi_144.spi_write(hw,"LMK04828_CTRL",0x149,0x40);
  spi_144.spi_write(hw,"LMK04828_CTRL",0x14A,0x00);
  spi_144.spi_write(hw,"LMK04828_CTRL",0x14B,0x05);
  spi_144.spi_write(hw,"LMK04828_CTRL",0x14C,0xFF);
  spi_144.spi_write(hw,"LMK04828_CTRL",0x14D,0x00);
  spi_144.spi_write(hw,"LMK04828_CTRL",0x14E,0x00);
  spi_144.spi_write(hw,"LMK04828_CTRL",0x14F,0x7F);
  spi_144.spi_write(hw,"LMK04828_CTRL",0x150,0x00);
  spi_144.spi_write(hw,"LMK04828_CTRL",0x151,0x02);
  spi_144.spi_write(hw,"LMK04828_CTRL",0x152,0x00);
  spi_144.spi_write(hw,"LMK04828_CTRL",0x153,0x00);
  spi_144.spi_write(hw,"LMK04828_CTRL",0x154,0x78);
   // target 80KHz with 100 MHz reference (CLKIN1 = REF IN and ignored with external clock
  spi_144.spi_write(hw,"LMK04828_CTRL",0x155,0x04); //CLKIN1_DIV (MS)
  spi_144.spi_write(hw,"LMK04828_CTRL",0x156,0xE2); //CLKIN1_DIV (LS)
   // Fpd is 80KHz with 100MHz Onboard (internal) reference
  spi_144.spi_write(hw,"LMK04828_CTRL",0x157,0x04);
  spi_144.spi_write(hw,"LMK04828_CTRL",0x158,0xE2);
  
  spi_144.spi_write(hw,"LMK04828_CTRL",0x159,0x18);
  spi_144.spi_write(hw,"LMK04828_CTRL",0x15A,0x01);
  spi_144.spi_write(hw,"LMK04828_CTRL",0x15C,0x20);
  spi_144.spi_write(hw,"LMK04828_CTRL",0x15D,0x00);
  spi_144.spi_write(hw,"LMK04828_CTRL",0x15E,0x00);
  spi_144.spi_write(hw,"LMK04828_CTRL",0x15B,0xDF);
  
  spi_144.spi_write(hw,"LMK04828_CTRL",0x15F,0x3B);
  spi_144.spi_write(hw,"LMK04828_CTRL",0x160,0x00);
  spi_144.spi_write(hw,"LMK04828_CTRL",0x161,0x60);
  spi_144.spi_write(hw,"LMK04828_CTRL",0x162,0x50);
  spi_144.spi_write(hw,"LMK04828_CTRL",0x163,0x00);
  spi_144.spi_write(hw,"LMK04828_CTRL",0x164,0x00);
  spi_144.spi_write(hw,"LMK04828_CTRL",0x165,0x0C);
  
  spi_144.spi_write(hw,"LMK04828_CTRL",0x17C,0x15);
  spi_144.spi_write(hw,"LMK04828_CTRL",0x17D,0x33);
  spi_144.spi_write(hw,"LMK04828_CTRL",0x166,0x00);
  spi_144.spi_write(hw,"LMK04828_CTRL",0x167,0x00);
  spi_144.spi_write(hw,"LMK04828_CTRL",0x168,0xFA);
  spi_144.spi_write(hw,"LMK04828_CTRL",0x169,0x49);
  spi_144.spi_write(hw,"LMK04828_CTRL",0x16A,0x00);
  spi_144.spi_write(hw,"LMK04828_CTRL",0x16B,0x10);
  spi_144.spi_write(hw,"LMK04828_CTRL",0x16C,0x00);
  spi_144.spi_write(hw,"LMK04828_CTRL",0x16D,0x00);
  
  spi_144.spi_write(hw,"LMK04828_CTRL",0x16E,0x13);
  spi_144.spi_write(hw,"LMK04828_CTRL",0x173,0x00);
  
  if (clockmode == getClockMode("EXTERNAL") or clockmode == getClockMode("DDS")){
    spi_144.spi_write(hw,"LMK04828_CTRL",0x138,0x40);
    spi_144.spi_write(hw,"LMK04828_CTRL",0x100,0x01);
    spi_144.spi_write(hw,"LMK04828_CTRL",0x108,0x01);
    spi_144.spi_write(hw,"LMK04828_CTRL",0x110,0x01);
    spi_144.spi_write(hw,"LMK04828_CTRL",0x118,0x01);
    spi_144.spi_write(hw,"LMK04828_CTRL",0x130,0x01);
    spi_144.spi_write(hw,"LMK04828_CTRL",0x103,0x02);
    spi_144.spi_write(hw,"LMK04828_CTRL",0x10B,0x02);
    spi_144.spi_write(hw,"LMK04828_CTRL",0x113,0x02);
    spi_144.spi_write(hw,"LMK04828_CTRL",0x11B,0x02);
    spi_144.spi_write(hw,"LMK04828_CTRL",0x133,0x02);
    spi_144.spi_write(hw,"LMK04828_CTRL",0x13B,0x20);
    spi_144.spi_write(hw,"LMK04828_CTRL",0x147,0x00);
    
    spi_144.spi_write(hw,"LMK04828_CTRL",0x120,0x01);
    spi_144.spi_write(hw,"LMK04828_CTRL",0x128,0x01);
    spi_144.spi_write(hw,"LMK04828_CTRL",0x123,0x02);
    spi_144.spi_write(hw,"LMK04828_CTRL",0x12B,0x02);
  
  }
   
  //  sleep(0.100); // From Erdem's code
  usleep(1e6); // 1 sec
  
  spi_144.spi_write(hw,"LMK04828_CTRL",0x182,0x01);
  spi_144.spi_write(hw,"LMK04828_CTRL",0x182,0x00);
  //  sleep(0.30);
  usleep(3e5); // 0.3 secs
  
  dword = spi_144.spi_read(hw,"LMK04828_CTRL",0x182);
  //if (rc == 1){
  std::cout << HEX(dword,8) << std::endl;

  // Check if PLL1 is locked...
  if (clockmode == getClockMode("INTERNAL") or clockmode == getClockMode("EXTERNAL_REF")){
    int check = spi_144.spi_check(hw,"LMK04828_CTRL",0x182,0x02,0x02,"PLL1 not locked");
    if (check == 1) {
      std::cout << "PLL1 locked" << std::endl;
    }
    else{
      std::cout << "ERROR: PLL1 lock unsuccesful" << std::endl;
      printf("Stopping...\n");
      exit(0);
    }
  }

  spi_144.spi_write(hw,"LMK04828_CTRL",0x183,0x01);
  spi_144.spi_write(hw,"LMK04828_CTRL",0x183,0x00);
  //  sleep(0.5);
  usleep(5e5); // 0.5 secs
  
  dword = spi_144.spi_read(hw,"LMK04828_CTRL",0x183);
  //if (rc == 1){
  std::cout << HEX(dword,8) << std::endl;;

  if (clockmode == getClockMode("INTERNAL") or clockmode == getClockMode("EXTERNAL_REF")){
    int check = spi_144.spi_check(hw,"LMK04828_CTRL",0x183,0x02,0x02,"PLL2 not locked");
    if (check == 1) {
      std::cout << "PLL2 locked" << std::endl;
    }
    else{
      std::cout << "ERROR: PLL2 lock unsuccesful" << std::endl;
      printf("Stopping...\n");
      exit(0);
    }
  }

  spi_144.spi_write(hw,"LMK04828_CTRL",0x143,0xD1);
  spi_144.spi_write(hw,"LMK04828_CTRL",0x139,0x00);
  spi_144.spi_write(hw,"LMK04828_CTRL",0x144,0x00);
  spi_144.spi_write(hw,"LMK04828_CTRL",0x147,0x03);
  spi_144.spi_write(hw,"LMK04828_CTRL",0x143,0xF1);
  spi_144.spi_write(hw,"LMK04828_CTRL",0x143,0xD1);
  
  //  sleep(0.080);
  usleep(8e4); // 0.08 secs
  
  spi_144.spi_write(hw,"LMK04828_CTRL",0x144,0xFF);
  spi_144.spi_write(hw,"LMK04828_CTRL",0x143,0x50);
  spi_144.spi_write(hw,"LMK04828_CTRL",0x139,0x03);
  
}

// Initialise ADC0 and ADC1 
int FMC144::adc_init(IPBusManager* hw, int adc_mode){

  uint64_t dword;

  int number_of_lanes = 4;
  uint64_t reg60start = 0x7E;
  uint64_t reg60end = 0x7F;
    
  if (adc_mode == getADCmode("2CH_4LANE") or adc_mode == getADCmode("4CH_4LANE")){
    reg60start = 0x7C;
    reg60end = 0x7D;
  }
  
  //ADC 0 Initialization
  // RESET ADC0
  spi_144.spi_write(hw,"ADC16DX370_CTRL0",0x0,0xBD);
  // Disable JESD
  spi_144.spi_write(hw,"ADC16DX370_CTRL0",0x60,0x00);
  // Configure JESD
  spi_144.spi_write(hw,"ADC16DX370_CTRL0",0x60,reg60start);
  
  if(testADC0()){
    //Enable test mode ADC0
    spi_144.spi_write(hw,"ADC16DX370_CTRL0",0x61,0x06);
    dword = spi_144.spi_read(hw,"ADC16DX370_CTRL0",0x61);
    printf("ADC in test mode ..: %lX\n",dword);
  }

  //Check chip ID (ADC0)
  dword = spi_144.spi_read(hw,"ADC16DX370_CTRL0",0x04);
  std::cout << HEX(dword,8) << std::endl;
  int check = spi_144.spi_check(hw,"ADC16DX370_CTRL0",0x04,NULL,getADCpartID(),"Error - wrong ADC0 part no.");
  if (check == 1) {
    std::cout << "Check ADC0 part no. --> Succesful." << std::endl;
  }
  else{
    std::cout << "ERROR: Error - wrong ADC0 part no." << std::endl;
    printf("Stopping...\n");
    exit(0);
  }
  
  //OM1
  spi_144.spi_write(hw,"ADC16DX370_CTRL0",0x12,0x85);
  // Enable JESD
  spi_144.spi_write(hw,"ADC16DX370_CTRL0",0x60,reg60end);

  if (adc_mode == getADCmode("4CH_4LANE") or adc_mode == getADCmode("4CH_8LANE")){
    //ADC 1 initialization
    //Reset ADC1
    spi_144.spi_write(hw,"ADC16DX370_CTRL1",0x00,0xBD);
    //Disable JESD
    spi_144.spi_write(hw,"ADC16DX370_CTRL1",0x60,0x00);
    //Configure JESD
    spi_144.spi_write(hw,"ADC16DX370_CTRL1",0x60,reg60start);

    if (testADC1()){
      //Enable test mode ADC1
      spi_144.spi_write(hw,"ADC16DX370_CTRL1",0x61,0x06);
      dword = spi_144.spi_read(hw,"ADC16DX370_CTRL1",0x61);
      printf("ADC in test mode ..: %lX\n",dword);
    }

    //Check chip ID (ADC1)
    dword = spi_144.spi_read(hw,"ADC16DX370_CTRL1",0x04);
    std::cout << HEX(dword,8) << std::endl;
    int check = spi_144.spi_check(hw,"ADC16DX370_CTRL1",0x04,NULL,getADCpartID(),"Error - wrong ADC1 part no.");
    if (check == 1) {
      std::cout << "Check ADC1 part no. --> Succesful." << std::endl;
    }
    else{
      std::cout << "ERROR: Error - wrong ADC1 part no." << std::endl;
      printf("Stopping...\n");
      exit(0);
    }
    
    //OM1
    spi_144.spi_write(hw,"ADC16DX370_CTRL1",0x12,0x85);
    // Enable JESD
    spi_144.spi_write(hw,"ADC16DX370_CTRL1",0x60,reg60end);
  
  }

  //  sleep(0.500);
  usleep(5e5); // 0.5 secs
  std::cout << "Releasing JESD" << std::endl;

  return 1;
  
}

// Initialise DAC0     
int FMC144::dac_init(IPBusManager* hw, int odelay, int number_of_lanes){

  uint64_t dword;
  std::string node;


  uint64_t reg_3B;
  uint64_t reg_31;
  uint64_t reg_1A;
  uint64_t reg_05;
  if (enableDACPLL()){
    reg_3B = 0x8000;
    reg_31 = 0x6C08;
    reg_1A = 0x0000;
    reg_05 = 0x0000;
  }
  else{
    reg_3B = 0x0000;
    reg_31 = 0x1000;
    reg_1A = 0x0020;
    reg_05 = 0x0001;
  }

  uint64_t reg_32 = 0x01C0;   
  uint64_t reg_33 = 0xE51C;
 
  //JESD settings
  uint64_t reg_25 = 0x2000; // jesd clock div = 2
  uint64_t reg_3E = 0x0128; // controls RATE
  uint64_t reg_3C = 0x0228; // MPY
  uint64_t reg_4B = 0x1F00; // RBD
  uint64_t reg_4C = 0x1F07; // K=32, L=8
  uint64_t reg_4D = 0x0300; // M=4, S=1
  uint64_t reg_4E = 0x0F4F; // high density mode, scramble off
  
  //Global settings
  uint64_t reg_00 = 0x0018;
  uint64_t reg_04 = 0x0000;
  
  uint64_t reg_4F = 0x1CC1;
  uint64_t reg_5F = 0x0123;
  uint64_t reg_60 = 0x4567;
  uint64_t reg_46 = 0x0044;
  uint64_t reg_47 = 0x190A;
  uint64_t reg_24 = 0x0040; 
  
  uint64_t reg_5C = 0x1111;
  uint64_t reg4A_off = 0xFF1E;
  uint64_t reg4A_on = 0xff01;
  
  uint64_t reg54 = 0xFF; // sync_reguest_ena_link1 (which error causes a sync request)
  uint64_t reg52 = 0xFF; // error_ena_link0 (what counts as an error?)
  uint64_t reg51 = 0xFF; // sync_request_ena_link0
  uint64_t reg55 = 0xFF; // 0x55, 0x58, 0x58
  
  if (number_of_lanes == getADCmode("2CH_4LANE") or number_of_lanes == getADCmode("4CH_4LANE")){
    reg_3B = 0x0000;
    reg_31 = 0x6C08;
    reg_32 = 0x01C0;
    reg_3C = 0x0250;
    reg_3C = 0x0228;
    reg_3E = 0x0108;
    reg_00 = 0x0018;
    reg_04 = 0xF0F0;
    reg_05 = 0xFF0D;
    reg_25 = 0x0000;
    reg_4B = 0x0801;
    reg_4C = 0x1F03;
    reg_4D = 0x0300;
    reg_4E = 0x0F0F;
    reg4A_on = 0x0F01;
    reg4A_off = 0x0F1E;
    reg51 = 0xFF;
    reg54 = 0xFF;
    reg52 = 0xFF;
    reg55 = 0x1F;
  }
  
  spi_144.spi_write(hw,"DAC38J84_CTRL0",0x46,reg_46);
  spi_144.spi_write(hw,"DAC38J84_CTRL0",0x47,reg_47);
 
  //Disable TX
  node = "axi_fmc144_8lane.FMC144_ctrl.dac_ctrl";
  hw->write(node,0x00);
  
  //Reset DAC Block
  spi_144.spi_write(hw,"DAC38J84_CTRL0",0x4A,reg4A_off);
  //Reset Setup for 4-wire SPI
  spi_144.spi_write(hw,"DAC38J84_CTRL0",0x02,0x83);
  //Reset Setup for 4-wire SPI
  spi_144.spi_write(hw,"DAC38J84_CTRL0",0x02,0x2082);
 
  //DAC SPI check
  dword = spi_144.spi_read(hw,"DAC38J84_CTRL0",0x7F);
  dword = dword & 0xFF;
  int check = spi_144.spi_check(hw,"DAC38J84_CTRL0",0x7F,0xFF,getDACpartID(),"Error - wrong DAC part ID");
  if (check == 1) {
    std::cout << "Check DAC part ID --> Succesful." << std::endl;
  }
  else{
    std::cout << "ERROR: Error - wrong DAC part ID" << std::endl;
    printf("Stopping...\n");
    exit(0);
  }

  spi_144.spi_write(hw,"DAC38J84_CTRL0",0x03,0xA300);
  //Invert DAC Outputs
  spi_144.spi_write(hw,"DAC38J84_CTRL0",0x01,0xF3);
  // ALARM Control - Assuming all are on by default
  spi_144.spi_write(hw,"DAC38J84_CTRL0",0x04,reg_04);
  spi_144.spi_write(hw,"DAC38J84_CTRL0",0x05,reg_05);
  spi_144.spi_write(hw,"DAC38J84_CTRL0",0x06,0xFFFF);
  spi_144.spi_write(hw,"DAC38J84_CTRL0",0x1A,reg_1A);
  spi_144.spi_write(hw,"DAC38J84_CTRL0",0x33,reg_33);
  spi_144.spi_write(hw,"DAC38J84_CTRL0",0x3D,0x0088);
  spi_144.spi_write(hw,"DAC38J84_CTRL0",0x49,0x0);
  spi_144.spi_write(hw,"DAC38J84_CTRL0",0x61,0x0F);
  spi_144.spi_write(hw,"DAC38J84_CTRL0",0x51,reg51);
  spi_144.spi_write(hw,"DAC38J84_CTRL0",0x54,reg54);
  spi_144.spi_write(hw,"DAC38J84_CTRL0",0x57,reg54);
  spi_144.spi_write(hw,"DAC38J84_CTRL0",0x5A,reg54);
 
  spi_144.spi_write(hw,"DAC38J84_CTRL0",0x52,reg52);
  spi_144.spi_write(hw,"DAC38J84_CTRL0",0x55,reg55);
  spi_144.spi_write(hw,"DAC38J84_CTRL0",0x58,reg55);
  spi_144.spi_write(hw,"DAC38J84_CTRL0",0x5B,reg55);
 
  spi_144.spi_write(hw,"DAC38J84_CTRL0",0x4B,reg_4B);
  spi_144.spi_write(hw,"DAC38J84_CTRL0",0x4C,reg_4C);
  spi_144.spi_write(hw,"DAC38J84_CTRL0",0x4D,reg_4D);
  spi_144.spi_write(hw,"DAC38J84_CTRL0",0x4E,reg_4E);
 
  spi_144.spi_write(hw,"DAC38J84_CTRL0",0x4F,reg_4F);
  spi_144.spi_write(hw,"DAC38J84_CTRL0",0x24,reg_24);
  spi_144.spi_write(hw,"DAC38J84_CTRL0",0x5C,reg_5C);
  spi_144.spi_write(hw,"DAC38J84_CTRL0",0x1F,0x4440);
  spi_144.spi_write(hw,"DAC38J84_CTRL0",0x1E,0x4444);
  spi_144.spi_write(hw,"DAC38J84_CTRL0",0x20,0x4044);
 
  spi_144.spi_write(hw,"DAC38J84_CTRL0",0x3C,reg_3C);
  spi_144.spi_write(hw,"DAC38J84_CTRL0",0x00,reg_00);
  spi_144.spi_write(hw,"DAC38J84_CTRL0",0x25,reg_25);
  spi_144.spi_write(hw,"DAC38J84_CTRL0",0x31,reg_31);
  spi_144.spi_write(hw,"DAC38J84_CTRL0",0x3B,reg_3B);
  spi_144.spi_write(hw,"DAC38J84_CTRL0",0x32,reg_32);
  spi_144.spi_write(hw,"DAC38J84_CTRL0",0x3E,reg_3E);
  spi_144.spi_write(hw,"DAC38J84_CTRL0",0x5F,reg_5F);
  spi_144.spi_write(hw,"DAC38J84_CTRL0",0x60,reg_60);
 
  // sleep(0.500);
  usleep(5e5); // 0.5 secs
  node = "axi_fmc144_8lane.FMC144_ctrl.dac_ctrl";
  hw->write(node,0x01);
  
  //  sleep(0.500);
  usleep(5e5); // 0.5 secs
  spi_144.spi_write(hw,"DAC38J84_CTRL0",0x4A,0xFF1F);
  spi_144.spi_write(hw,"DAC38J84_CTRL0",0x4A,0xFF01);
  //  sleep(0.800);
  usleep(8e5); // 0.8 secs
 
  spi_144.spi_write(hw,"DAC38J84_CTRL0",0x6C,0x0);
  spi_144.spi_write(hw,"DAC38J84_CTRL0",0x6D,0x0);
      
  for (int i = 0; i < 8; i++){
    spi_144.spi_write(hw,"DAC38J84_CTRL0",(0x64+i),0x0);
  }
  
  //  sleep(0.8);
  usleep(8e5); // 0.8 secs
  std::cout << "Status" << std::endl;
  std::cout << "........................" << std::endl;
  
  dword = spi_144.spi_read(hw,"DAC38J84_CTRL0",0x6C);
  dword = dword & ~0x2;

  uint64_t bitOp = ~0x02;
  if (enableDACPLL() == false){
    dword = dword & ~0x3;
    bitOp = ~0x03;
  }
  
  check = spi_144.spi_check(hw,"DAC38J84_CTRL0",0x6C,bitOp,0,"PLL LOCK ERROR");
  if (check == 1) {
    std::cout << "Check PLL LOCK --> Succesful." << std::endl;
  }
  else{
    std::cout << "ERROR: PLL LOCK ERROR" << std::endl;
    printf("Stopping...\n");
    exit(0);
  }
  
  dword = spi_144.spi_read(hw,"DAC38J84_CTRL0",0x6D);
  check = spi_144.spi_check(hw,"DAC38J84_CTRL0",0x6D,NULL,0,"LANE LOSS SIGNAL");
  if (check == 1) {
    std::cout << "Check LANE LOSS SIGNAL --> Succesful." << std::endl;
  }
  else{
    std::cout << "ERROR: LANE LOSS SIGNAL" << std::endl;
    printf("Stopping...\n");
    exit(0);
  }

  for (int i = 0; i < 8; i++){
    dword = spi_144.spi_read(hw,"DAC38J84_CTRL0",0x64+i);
    if (dword == 0x2 or dword == 0x3 or dword == 0x0){
      printf("LANE%d STATUS: OK\n",i);
    }
    else{
      printf("LANE%d STATUS: Error. Register is %lx\n",i,dword);
      printf("Stopping...\n");
      exit(0);
    }
    if (i == 3 and number_of_lanes != getADCmode("4CH_8LANE")){
      break;
    }
  }
     
  std::cout << "......................................" << std::endl;
 
  for (int i = 0; i < 4; i++){
    dword = spi_144.spi_read(hw,"DAC38J84_CTRL0",0x41+i);
    if (dword != 0x0){
      printf("LINK%d %ld errors found!!!\n",i,dword);
      printf("Stopping...\n");
      exit(0);
    }
  }
  
  return 1;
  
}

// Initialize both ADC chips on the FMC144. 
int FMC144::init(IPBusManager* hw, int clockmode, int adc_mode){

  std::cout << "Entered FMC144::init" << std::endl;
  
  uint64_t dword;
  std::string node;

  node = "axi_fmc144_8lane.FMC144_ctrl.pin_ctrl";
  if (clockmode == getClockMode("DDS")){
    hw->write(node,0x04);
  }
  else if (clockmode == getClockMode("EXTERNAL") or clockmode == getClockMode("EXTERNAL_REF")){
    hw->write(node,0x08);
  }
  else{
    hw->write(node,0x10);
    dword = hw->read(node);
  }

  // Configure the clock tree  
  std::cout << "Configuring the clock tree..." << std::endl;
  clocktree_init(hw,clockmode);

  // Configure ADC0 and ADC1                                                                                 
  std::cout << "Configuring ADCs..." << std::endl;
  if (adc_init(hw,adc_mode) == 0){
    std::cout << "Could not initalise FMC144.ADC" << std::endl;
    printf("Stopping...\n");
    exit(0);
  }

  // Configure Transceiver
  // Assert Transceiver Reset
  node = "axi_fmc144_8lane.FMC144_ctrl.transceiver";
  hw->write(node,0x01);
  //  sleep(0.01);
  usleep(1e4); // 0.01 secs
  // Release Transceiver Reset 
  hw->write(node,0x00);
  std::cout << "Waiting for QPLL to lock..." << std::endl;
  //  sleep(15.0);
  usleep(15e6); // 15 secs
  // Enable manual bit alignment 
  hw->write(node,0x10);       

  // Wait for alignment to complete  
  std::cout << "Waiting for alignment to complete..." << std::endl;
  //  sleep(15.0);
  usleep(15e6); // 15 secs

  // Configure DAC0        
  std::cout << "Configuring DAC..." << std::endl;
  if (dac_init(hw,0,adc_mode) == 0) {
    std::cout << "Could not initalise FMC144.DAC0" << std::endl;
    printf("Stopping...\n");
    exit(0);
  }

  // Wait for JESD ADC to become stable   
  node = "axi_fmc144_8lane.FMC144_ctrl.adc0_stat";
  uint64_t current0 = hw->read(node);

  std::cout << "Expecting JESD to be ready soon. Please wait..." << std::endl;
  for (int xx = 0; xx < 100; xx++){
    //    sleep(0.30);
    usleep(3e5); // 0.3 secs
    node = "axi_fmc144_8lane.FMC144_ctrl.adc0_stat";
    uint64_t next0 = hw->read(node);
    if (current0 == next0){
      std::cout << "JESD READY!" << std::endl;
      break;
    }
    else{
      std::cout << "." << std::endl;
      current0 = next0;
    }
  }

  std::cout << "Done!" << std::endl;

  // NOTE!!!!! The following conditional Yes/No prints are defined in the reverse
  // order in python, e.g.:emacs -nw main_test.py
  // print("CLKIN Detected..: %s" %("No","Yes") [int(dword,0) >> 0 & 0x1 ])

  // Check ADC0 status  
  dword = spi_144.spi_read(hw,"ADC16DX370_CTRL0",0x6C);
  //  std::cout << dword >> 0 & 0x1 << std::endl;
  printf("---------ADC0 Status -------------\n");
  printf("CLKIN Detected..: %s\n",(dword >> 0 & 0x1 ? "Yes" : "No" )); 
  printf("CAL_DONE........: %s\n",(dword >> 1 & 0x1 ? "Yes" : "No" ));
  printf("PLL_locked......: %s\n",(dword >> 2 & 0x1 ? "Yes" : "No" ));
  printf("ALIGN...........: %s\n",(dword >> 4 & 0x1 ? "Yes" : "No" ));
  printf("LINK............: %s\n",(dword >> 6 & 0x1 ? "Yes" : "No" ));
  printf("....................................\n");

  if (adc_mode == getADCmode("4CH_4LANE") or adc_mode == getADCmode("4CH_8LANE")){
  // Check ADC1 status  
    dword = spi_144.spi_read(hw,"ADC16DX370_CTRL1",0x6C);
    printf("---------ADC1 Status -------------\n");
    printf("CLKIN Detected..: %s\n",(dword >> 0 & 0x1 ? "Yes" : "No" )); 
    printf("CAL_DONE........: %s\n",(dword >> 1 & 0x1 ? "Yes" : "No" ));
    printf("PLL_locked......: %s\n",(dword >> 2 & 0x1 ? "Yes" : "No" ));
    printf("ALIGN...........: %s\n",(dword >> 4 & 0x1 ? "Yes" : "No" ));
    printf("LINK............: %s\n",(dword >> 6 & 0x1 ? "Yes" : "No" ));
    printf("....................................\n");
  }
  std::cout << "Exiting FMC144::init" << std::endl;

  return 1;

}
