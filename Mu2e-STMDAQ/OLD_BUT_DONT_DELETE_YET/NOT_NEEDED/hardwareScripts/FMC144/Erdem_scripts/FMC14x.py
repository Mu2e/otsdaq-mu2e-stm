#!/usr/bin/python

# -*- coding: utf-8 -*-
#import sys
import time
#import uhal
import spi_functions as s

CLOCKTREE_CLKSRC_INTERNAL = 0
CLOCKTREE_CLKSRC_EXTERNAL = 1
CLOCKTREE_CLKSRC_EXTERNAL_REF = 2
CLOCKTREE_CLKSRC_DDS = 3

ADC_MODE_2CH_8LANE = 0
ADC_MODE_4CH_8LANE = 1
ADC_MODE_2CH_4LANE = 2
ADC_MODE_4CH_4LANE = 3

FMC14x_ADC_PART_ID = 0x02
FMC14x_DAC_PART_ID = 0x0A

DACPLL_ENABLE = 0

def FMC14x_clocktree_init(hw,clockmode):
  
  if clockmode == CLOCKTREE_CLKSRC_DDS:
    ftw = 104145741382943
    freq = 370.0
    fs = 1000.0
    
#    ftw = long ((1 << 48) * (freq/fs))
    ftw = int ((1 << 48) * (freq/fs))  # python3 long->int
    print (ftw)
    #Enable SDO, Soft reset (not self clearing)
    s.spi_write(hw,"AD9912_CTRL",0x000,0xBD)
    #time.sleep(0.200)
    ##Clear Soft reset
    s.spi_write(hw,"AD9912_CTRL",0x000,0x99)
    #time.sleep(0.200)
    #Read part ID LSB
    commandword = 0x002
    #dword = s.spi_read(hw,"AD9912_CTRL",commandword)
    dword = s.spi_read(hw,"AD9912_CTRL",0x002)
    print (dword)
    partId = dword
    #Read part ID MSB
    dword = s.spi_read(hw,"AD9912_CTRL",0x003)
    print (dword)
    partId = int(dword,0) + (int(dword,0)<<8)
    #print (partId)
    #Part ID should be 0x1902 
    if (partId == 0x1902 or partId == 0x1982):
      print ("DDS part ID: %5.4X (OK)" %partId)
    else:
      print ("DDS part ID: %5.4X (ERROR)" %partId)
    #Enable the HSTL output, disable all power down features
    s.spi_write(hw,"AD9912_CTRL",0x010,0x00)
    #Write FTW
    s.spi_write(hw,"AD9912_CTRL",0x1A6,(0xFF & (ftw >>0)))
    s.spi_write(hw,"AD9912_CTRL",0x1A7,(0xFF & (ftw >>8)))
    s.spi_write(hw,"AD9912_CTRL",0x1A8,(0xFF & (ftw >>16)))
    s.spi_write(hw,"AD9912_CTRL",0x1A9,(0xFF & (ftw >>24)))
    s.spi_write(hw,"AD9912_CTRL",0x1AA,(0xFF & (ftw >>32)))
    s.spi_write(hw,"AD9912_CTRL",0x1AB,(0xFF & (ftw >>40)))
    #Update must be written last
    s.spi_write(hw,"AD9912_CTRL",0x005,0x01)
  
  s.spi_write(hw,"LMK04828_CTRL",0x000,0x90)
  time.sleep(0.200)
  s.spi_write(hw,"LMK04828_CTRL",0x000,0x10)
  time.sleep(0.200)
  s.spi_write(hw,"LMK04828_CTRL",0x148,0x00)
  time.sleep(0.200)
  s.spi_write(hw,"LMK04828_CTRL",0x15F,0x3B) #STATUS_LD1_MUX LD1 = SPI_MISO
  time.sleep(0.200)
  s.spi_write(hw,"LMK04828_CTRL",0x002,0x00) #POWERDOWN enabled
  #time.sleep(0.200)
  
  #DCLKout0/SDCLKout1 configuration
  #s.spi_write(hw,"LMK04828_CTRL",0x100,0x07) #DIV_CLKOUT1_0 DIV_BY_7 IDL/ODL ==>In/Out Drive level
  s.spi_write(hw,"LMK04828_CTRL",0x100,0x08) #DIV_CLKOUT1_0 DIV_BY_7 IDL/ODL ==>In/Out Drive level
  s.spi_write(hw,"LMK04828_CTRL",0x101,0x22)
  s.spi_write(hw,"LMK04828_CTRL",0x103,0x05)
  s.spi_write(hw,"LMK04828_CTRL",0x104,0x62)
  s.spi_write(hw,"LMK04828_CTRL",0x105,0x00)
  s.spi_write(hw,"LMK04828_CTRL",0x106,0xB0)
  s.spi_write(hw,"LMK04828_CTRL",0x107,0x11) # FMT_CLK1_0 LVDS,LVDS !DCLK_INV , !DCLK_INV
  
  #DCLKout2/SDCLKout3 configuration
  s.spi_write(hw,"LMK04828_CTRL",0x108,0x28)
  #s.spi_write(hw,"LMK04828_CTRL",0x108,0x27)
  s.spi_write(hw,"LMK04828_CTRL",0x109,0x22)
  s.spi_write(hw,"LMK04828_CTRL",0x10B,0x05)
  s.spi_write(hw,"LMK04828_CTRL",0x10C,0x62) #DIG_DLY_SCLK3
  s.spi_write(hw,"LMK04828_CTRL",0x10D,0x13)
  s.spi_write(hw,"LMK04828_CTRL",0x10E,0xB0)
  s.spi_write(hw,"LMK04828_CTRL",0x10F,0x55) #FMT_CLK3_2 LVPECL,LVPECL, !DCLK_INV, !SCLK_INV
  
  #DCLKout4/SDCLKout5 configuration (ADC0)
  #s.spi_write(hw,"LMK04828_CTRL",0x110,0x07)
  s.spi_write(hw,"LMK04828_CTRL",0x110,0x08)
  s.spi_write(hw,"LMK04828_CTRL",0x111,0x22)
  s.spi_write(hw,"LMK04828_CTRL",0x113,0x05)
  s.spi_write(hw,"LMK04828_CTRL",0x114,0x62)
  s.spi_write(hw,"LMK04828_CTRL",0x115,0x00)
  s.spi_write(hw,"LMK04828_CTRL",0x116,0xB0)
  s.spi_write(hw,"LMK04828_CTRL",0x117,0x15) #DIV_CLKOUT7_6 // 16 to enable sysref
  
  #DCLKout6/SDCLKout7 configuration (ADC1)
  #s.spi_write(hw,"LMK04828_CTRL",0x118,0x07)
  s.spi_write(hw,"LMK04828_CTRL",0x118,0x08)
  s.spi_write(hw,"LMK04828_CTRL",0x119,0x22)
  s.spi_write(hw,"LMK04828_CTRL",0x11B,0x05)
  s.spi_write(hw,"LMK04828_CTRL",0x11C,0x62)
  s.spi_write(hw,"LMK04828_CTRL",0x11D,0x00)
  s.spi_write(hw,"LMK04828_CTRL",0x11E,0xB0) #PD_CLK7_6 EN_DCLK, EN_SCLK
  s.spi_write(hw,"LMK04828_CTRL",0x11F,0x15)
  
  #DCLKout8/SDCLKout9 configuration
  #s.spi_write(hw,"LMK04828_CTRL",0x120,0x07) #DIV_CLKOUT9_8 (divided by 7)
  s.spi_write(hw,"LMK04828_CTRL",0x120,0x08) #DIV_CLKOUT9_8 (divided by 7)
  
  s.spi_write(hw,"LMK04828_CTRL",0x121,0x22) #DIG_DLY_DCLK8
  s.spi_write(hw,"LMK04828_CTRL",0x123,0x05) #ANA_DLY_DCLK8
  s.spi_write(hw,"LMK04828_CTRL",0x124,0x62)
  s.spi_write(hw,"LMK04828_CTRL",0x125,0x00)
  s.spi_write(hw,"LMK04828_CTRL",0x126,0xB1)
  s.spi_write(hw,"LMK04828_CTRL",0x127,0x11)
  
  #DCLKout10/SDCLKout11 configuration 
  #s.spi_write(hw,"LMK04828_CTRL",0x128,0x07) #DIV_CLKOUT9_8 (divided by 7)
  s.spi_write(hw,"LMK04828_CTRL",0x128,0x08) #DIV_CLKOUT9_8 (divided by 7)
  s.spi_write(hw,"LMK04828_CTRL",0x129,0x22)
  s.spi_write(hw,"LMK04828_CTRL",0x12B,0x05)
  s.spi_write(hw,"LMK04828_CTRL",0x12C,0x62)
  s.spi_write(hw,"LMK04828_CTRL",0x12D,0x00)
  s.spi_write(hw,"LMK04828_CTRL",0x12E,0xB1)
  s.spi_write(hw,"LMK04828_CTRL",0x12F,0x11)
  
  #DCLKout12/SDCLKout13 configuration 
  
  #s.spi_write(hw,"LMK04828_CTRL",0x130,0x07) #DIV_CLKOUT13_12
  s.spi_write(hw,"LMK04828_CTRL",0x130,0x08) #DIV_CLKOUT13_12
  s.spi_write(hw,"LMK04828_CTRL",0x131,0x22)
  s.spi_write(hw,"LMK04828_CTRL",0x133,0x05)
  s.spi_write(hw,"LMK04828_CTRL",0x134,0x62)
  s.spi_write(hw,"LMK04828_CTRL",0x135,0x00)
  s.spi_write(hw,"LMK04828_CTRL",0x136,0xB1)
  s.spi_write(hw,"LMK04828_CTRL",0x137,0x11)
  
  s.spi_write(hw,"LMK04828_CTRL",0x138,0x00) #VCO_OSC_OUT VCO0 (Low speed), PLL1_FB=OSCin, OSCOUT=input
  s.spi_write(hw,"LMK04828_CTRL",0x139,0x03) #SYSREF_MUX : SYSREF continuous
  #s.spi_write(hw,"LMK04828_CTRL",0x13A,0x00)
  #s.spi_write(hw,"LMK04828_CTRL",0x13B,0xE0)
  s.spi_write(hw,"LMK04828_CTRL",0x13A,0x01)
  s.spi_write(hw,"LMK04828_CTRL",0x13B,0x00)
  
  s.spi_write(hw,"LMK04828_CTRL",0x13C,0x07)
  s.spi_write(hw,"LMK04828_CTRL",0x13D,0x0E)
  s.spi_write(hw,"LMK04828_CTRL",0x13E,0x03)
  s.spi_write(hw,"LMK04828_CTRL",0x13F,0x00) #FB_CTRL PLL2_FB=prescaler, PLL1_FB=OSCIN
  s.spi_write(hw,"LMK04828_CTRL",0x140,0x01)
  s.spi_write(hw,"LMK04828_CTRL",0x141,0x00)
  s.spi_write(hw,"LMK04828_CTRL",0x142,0x00)
  s.spi_write(hw,"LMK04828_CTRL",0x143,0x50)
  s.spi_write(hw,"LMK04828_CTRL",0x144,0xFF)
  s.spi_write(hw,"LMK04828_CTRL",0x145,0x7F)
  s.spi_write(hw,"LMK04828_CTRL",0x146,0x00)
  
  if (clockmode == CLOCKTREE_CLKSRC_EXTERNAL) or (clockmode == CLOCKTREE_CLKSRC_EXTERNAL_REF) or (clockmode == CLOCKTREE_CLKSRC_DDS):
    s.spi_write(hw,"LMK04828_CTRL",0x147,0x1A)
  else: #internal clock
    s.spi_write(hw,"LMK04828_CTRL",0x147,0x2A) #CLKIN_MUX CLKIN: !INVERT,Auto, CLKIN1=PLL1, CLKIN0=PLL1
  
  s.spi_write(hw,"LMK04828_CTRL",0x148,0x00)
  s.spi_write(hw,"LMK04828_CTRL",0x149,0x40)
  s.spi_write(hw,"LMK04828_CTRL",0x14A,0x00)
  s.spi_write(hw,"LMK04828_CTRL",0x14B,0x05)
  s.spi_write(hw,"LMK04828_CTRL",0x14C,0xFF)
  s.spi_write(hw,"LMK04828_CTRL",0x14D,0x00)
  s.spi_write(hw,"LMK04828_CTRL",0x14E,0x00)
  s.spi_write(hw,"LMK04828_CTRL",0x14F,0x7F)
  s.spi_write(hw,"LMK04828_CTRL",0x150,0x00)
  s.spi_write(hw,"LMK04828_CTRL",0x151,0x02)
  s.spi_write(hw,"LMK04828_CTRL",0x152,0x00)
  s.spi_write(hw,"LMK04828_CTRL",0x153,0x00)
  s.spi_write(hw,"LMK04828_CTRL",0x154,0x78)
  # target 80KHz with 100 MHz reference (CLKIN1 = REF IN and ignored with external clock
  s.spi_write(hw,"LMK04828_CTRL",0x155,0x04) #CLKIN1_DIV (MS)
  s.spi_write(hw,"LMK04828_CTRL",0x156,0xE2) #CLKIN1_DIV (LS)
  # Fpd is 80KHz with 100MHz Onboard (internal) reference
  s.spi_write(hw,"LMK04828_CTRL",0x157,0x04)
  s.spi_write(hw,"LMK04828_CTRL",0x158,0xE2)
  
  s.spi_write(hw,"LMK04828_CTRL",0x159,0x18)
  s.spi_write(hw,"LMK04828_CTRL",0x15A,0x01)
  s.spi_write(hw,"LMK04828_CTRL",0x15C,0x20)
  s.spi_write(hw,"LMK04828_CTRL",0x15D,0x00)
  s.spi_write(hw,"LMK04828_CTRL",0x15E,0x00)
  s.spi_write(hw,"LMK04828_CTRL",0x15B,0xDF)
  
  s.spi_write(hw,"LMK04828_CTRL",0x15F,0x3B)
  s.spi_write(hw,"LMK04828_CTRL",0x160,0x00)
  s.spi_write(hw,"LMK04828_CTRL",0x161,0x60)
  s.spi_write(hw,"LMK04828_CTRL",0x162,0x50)
  s.spi_write(hw,"LMK04828_CTRL",0x163,0x00)
  s.spi_write(hw,"LMK04828_CTRL",0x164,0x00)
  s.spi_write(hw,"LMK04828_CTRL",0x165,0x0C)
  
  s.spi_write(hw,"LMK04828_CTRL",0x17C,0x15)
  s.spi_write(hw,"LMK04828_CTRL",0x17D,0x33)
  s.spi_write(hw,"LMK04828_CTRL",0x166,0x00)
  s.spi_write(hw,"LMK04828_CTRL",0x167,0x00)
#  s.spi_write(hw,"LMK04828_CTRL",0x168,0xFC)
  s.spi_write(hw,"LMK04828_CTRL",0x168,0xFA)
  s.spi_write(hw,"LMK04828_CTRL",0x169,0x49)
  s.spi_write(hw,"LMK04828_CTRL",0x16A,0x00)
  s.spi_write(hw,"LMK04828_CTRL",0x16B,0x10)
  s.spi_write(hw,"LMK04828_CTRL",0x16C,0x00)
  s.spi_write(hw,"LMK04828_CTRL",0x16D,0x00)
  
  s.spi_write(hw,"LMK04828_CTRL",0x16E,0x13)
  s.spi_write(hw,"LMK04828_CTRL",0x173,0x00)
  
  if (clockmode == CLOCKTREE_CLKSRC_EXTERNAL) or (clockmode == CLOCKTREE_CLKSRC_DDS):
    s.spi_write(hw,"LMK04828_CTRL",0x138,0x40)
    s.spi_write(hw,"LMK04828_CTRL",0x100,0x01)
    s.spi_write(hw,"LMK04828_CTRL",0x108,0x01)
    s.spi_write(hw,"LMK04828_CTRL",0x110,0x01)
    s.spi_write(hw,"LMK04828_CTRL",0x118,0x01)
    s.spi_write(hw,"LMK04828_CTRL",0x130,0x01)
    s.spi_write(hw,"LMK04828_CTRL",0x103,0x02)
    s.spi_write(hw,"LMK04828_CTRL",0x10B,0x02)
    s.spi_write(hw,"LMK04828_CTRL",0x113,0x02)
    s.spi_write(hw,"LMK04828_CTRL",0x11B,0x02)
    s.spi_write(hw,"LMK04828_CTRL",0x133,0x02)
    s.spi_write(hw,"LMK04828_CTRL",0x13B,0x20)
    s.spi_write(hw,"LMK04828_CTRL",0x147,0x00)
    
    s.spi_write(hw,"LMK04828_CTRL",0x120,0x01)
    s.spi_write(hw,"LMK04828_CTRL",0x128,0x01)
    s.spi_write(hw,"LMK04828_CTRL",0x123,0x02)
    s.spi_write(hw,"LMK04828_CTRL",0x12B,0x02)
  
  time.sleep(0.100)
  
  s.spi_write(hw,"LMK04828_CTRL",0x182,0x01)
  s.spi_write(hw,"LMK04828_CTRL",0x182,0x00)
  time.sleep(0.30)
  
  dword = s.spi_read(hw,"LMK04828_CTRL",0x182)
  #if rc == 1:
  print (dword)
  
  if clockmode == CLOCKTREE_CLKSRC_INTERNAL or clockmode == CLOCKTREE_CLKSRC_EXTERNAL_REF:
    if (int(dword,0) & 0x02) != 0x02:
      print ("PLL1 not locked")
    else:
      print ("PLL1 locked")
  
  s.spi_write(hw,"LMK04828_CTRL",0x183,0x01)
  s.spi_write(hw,"LMK04828_CTRL",0x183,0x00)
  time.sleep(0.5)
  
  dword = s.spi_read(hw,"LMK04828_CTRL",0x183)
  #if rc == 1:
  print (dword)
  
  if clockmode == CLOCKTREE_CLKSRC_INTERNAL or clockmode == CLOCKTREE_CLKSRC_EXTERNAL_REF:
    if int(dword,0) & 0x02 != 0x02:
      print ("PLL2 not locked")
    else:
      print ("PLL2 locked")
      
  s.spi_write(hw,"LMK04828_CTRL",0x143,0xD1)
  s.spi_write(hw,"LMK04828_CTRL",0x139,0x00)
  s.spi_write(hw,"LMK04828_CTRL",0x144,0x00)
  s.spi_write(hw,"LMK04828_CTRL",0x147,0x03)
  s.spi_write(hw,"LMK04828_CTRL",0x143,0xF1)
  s.spi_write(hw,"LMK04828_CTRL",0x143,0xD1)
  
  time.sleep(0.080)
  
  s.spi_write(hw,"LMK04828_CTRL",0x144,0xFF)
  s.spi_write(hw,"LMK04828_CTRL",0x143,0x50)
  s.spi_write(hw,"LMK04828_CTRL",0x139,0x03)
  
def FMC14x_adc_init(hw,adc_mode):
  number_of_lanes = 4
  reg60start = 0x7E
  reg60end = 0x7F
  
  if adc_mode == ADC_MODE_2CH_4LANE or adc_mode == ADC_MODE_4CH_4LANE:
    reg60start = 0x7C
    reg60end = 0x7D
    
  #ADC 0 Initialization
  # RESET ADC0
  s.spi_write(hw,"ADC16DX370_CTRL0",0x0,0xBD)
  # Disable JESD
  s.spi_write(hw,"ADC16DX370_CTRL0",0x60,0x00)
  # Configure JESD
  s.spi_write(hw,"ADC16DX370_CTRL0",0x60,reg60start)
  
  if 0:
    #Enable test mode ADC0
    s.spi_write(hw,"ADC16DX370_CTRL0",0x61,0x06)
    dword = s.spi_read(hw,"ADC16DX370_CTRL0",0x61)
    print ("ADC in test mode ..: %X" %int(dword,0))
    
  #Check chip ID (ADC0)
  dword = s.spi_read(hw,"ADC16DX370_CTRL0",0x04)
  print (dword)
  if int(dword,0) != FMC14x_ADC_PART_ID:
    print ("Error - wrong ADC0 part no.")
    return 0
  
  #OM1
  s.spi_write(hw,"ADC16DX370_CTRL0",0x12,0x85)
  # Enable JESD
  s.spi_write(hw,"ADC16DX370_CTRL0",0x60,reg60end)
  
  if adc_mode == ADC_MODE_4CH_4LANE or adc_mode == ADC_MODE_4CH_8LANE:
    #ADC 1 initialization
    #Reset ADC1
    s.spi_write(hw,"ADC16DX370_CTRL1",0x00,0xBD)
    #Disable JESD
    s.spi_write(hw,"ADC16DX370_CTRL1",0x60,0x00)
    #Configure JESD
    s.spi_write(hw,"ADC16DX370_CTRL1",0x60,reg60start)
  
    if 0:
    #Enable test mode ADC1
       s.spi_write(hw,"ADC16DX370_CTRL1",0x61,0x06)
       dword = s.spi_read(hw,"ADC16DX370_CTRL1",0x61)
       print ("ADC in test mode ..: %X" %int(dword,0))
  
    #Check chip ID (ADC1)
    dword = s.spi_read(hw,"ADC16DX370_CTRL1",0x04)
    print (dword)
    if int(dword,0) != FMC14x_ADC_PART_ID:
       print ("Error - wrong ADC1 part no.")
       return 0
  
    #OM1
    s.spi_write(hw,"ADC16DX370_CTRL1",0x12,0x85)
    # Enable JESD
    s.spi_write(hw,"ADC16DX370_CTRL1",0x60,reg60end)
    
  time.sleep(0.500)
  print ("Releasing JESD")
  
  return 1

def FMC14x_dac_init(hw,odelay,number_of_lanes):

   if DACPLL_ENABLE:
     reg_3B = 0x8000
     reg_31 = 0x6C08
     reg_1A = 0x0000
     reg_05 = 0x0000
   else:
     reg_3B = 0x0000
     reg_31 = 0x1000
     reg_1A = 0x0020
     reg_05 = 0x0001
   
   reg_32 = 0x01C0
   reg_33 = 0xE51C
   
   #JESD settings
   reg_25 = 0x2000 # jesd clock div = 2
   reg_3E = 0x0128 # controls RATE
   reg_3C = 0x0228 # MPY
   reg_4B = 0x1F00 # RBD
   reg_4C = 0x1F07 # K=32, L=8
   reg_4D = 0x0300 # M=4, S=1
   reg_4E = 0x0F4F # high density mode, scramble off
   
   #Global settings
   reg_00 = 0x0018
   reg_04 = 0x0000
   
   reg_4F = 0x1CC1
   reg_5F = 0x0123
   reg_60 = 0x4567
   reg_46 = 0x0044
   reg_47 = 0x190A
   reg_24 = 0x0040 
   
   reg_5C = 0x1111
   reg4A_off = 0xFF1E
   reg4A_on = 0xff01
   
   reg54 = 0xFF # sync_reguest_ena_link1 (which error causes a sync request)
   reg52 = 0xFF # error_ena_link0 (what counts as an error?)
   reg51 = 0xFF # sync_request_ena_link0
   reg55 = 0xFF # 0x55, 0x58, 0x58
   
   if number_of_lanes == ADC_MODE_2CH_4LANE or number_of_lanes == ADC_MODE_4CH_4LANE:
     reg_3B = 0x0000
     reg_31 = 0x6C08
     reg_32 = 0x01C0
     reg_3C = 0x0250
     reg_3C = 0x0228
     reg_3E = 0x0108
     reg_00 = 0x0018
     reg_04 = 0xF0F0
     reg_05 = 0xFF0D
     reg_25 = 0x0000
     reg_4B = 0x0801
     reg_4C = 0x1F03
     reg_4D = 0x0300
     reg_4E = 0x0F0F
     reg4A_on = 0x0F01
     reg4A_off = 0x0F1E
     reg51 = 0xFF
     reg54 = 0xFF
     reg52 = 0xFF
     reg55 = 0x1F
     
   s.spi_write(hw,"DAC38J84_CTRL0",0x46,reg_46)
   s.spi_write(hw,"DAC38J84_CTRL0",0x47,reg_47)
   
   #Disable TX
   hw.getNode("axi_fmc144_8lane.FMC144_ctrl.dac_ctrl").write(0x00)
   hw.dispatch()
   
   #Reset DAC Block
   s.spi_write(hw,"DAC38J84_CTRL0",0x4A,reg4A_off)
   #Reset Setup for 4-wire SPI
   s.spi_write(hw,"DAC38J84_CTRL0",0x02,0x83)
   #Reset Setup for 4-wire SPI
   s.spi_write(hw,"DAC38J84_CTRL0",0x02,0x2082)
   
   #DAC SPI check
   dword = s.spi_read(hw,"DAC38J84_CTRL0",0x7F)
   dword = int(dword,0) & 0xFF
   if dword != FMC14x_DAC_PART_ID:
      print ("wrong DAC part ID")
      return 0
   
   s.spi_write(hw,"DAC38J84_CTRL0",0x03,0xA300)
   #Invert DAC Outputs
   s.spi_write(hw,"DAC38J84_CTRL0",0x01,0xF3)
   # ALARM Control - Assuming all are on by default
   s.spi_write(hw,"DAC38J84_CTRL0",0x04,reg_04)
   s.spi_write(hw,"DAC38J84_CTRL0",0x05,reg_05)
   s.spi_write(hw,"DAC38J84_CTRL0",0x06,0xFFFF)
   s.spi_write(hw,"DAC38J84_CTRL0",0x1A,reg_1A)
   s.spi_write(hw,"DAC38J84_CTRL0",0x33,reg_33)
   s.spi_write(hw,"DAC38J84_CTRL0",0x3D,0x0088)
   s.spi_write(hw,"DAC38J84_CTRL0",0x49,0x0)
   s.spi_write(hw,"DAC38J84_CTRL0",0x61,0x0F)
   s.spi_write(hw,"DAC38J84_CTRL0",0x51,reg51)
   s.spi_write(hw,"DAC38J84_CTRL0",0x54,reg54)
   s.spi_write(hw,"DAC38J84_CTRL0",0x57,reg54)
   s.spi_write(hw,"DAC38J84_CTRL0",0x5A,reg54)
   
   s.spi_write(hw,"DAC38J84_CTRL0",0x52,reg52)
   s.spi_write(hw,"DAC38J84_CTRL0",0x55,reg55)
   s.spi_write(hw,"DAC38J84_CTRL0",0x58,reg55)
   s.spi_write(hw,"DAC38J84_CTRL0",0x5B,reg55)
   
   s.spi_write(hw,"DAC38J84_CTRL0",0x4B,reg_4B)
   s.spi_write(hw,"DAC38J84_CTRL0",0x4C,reg_4C)
   s.spi_write(hw,"DAC38J84_CTRL0",0x4D,reg_4D)
   s.spi_write(hw,"DAC38J84_CTRL0",0x4E,reg_4E)
   
   s.spi_write(hw,"DAC38J84_CTRL0",0x4F,reg_4F)
   s.spi_write(hw,"DAC38J84_CTRL0",0x24,reg_24)
   s.spi_write(hw,"DAC38J84_CTRL0",0x5C,reg_5C)
   s.spi_write(hw,"DAC38J84_CTRL0",0x1F,0x4440)
   s.spi_write(hw,"DAC38J84_CTRL0",0x1E,0x4444)
   s.spi_write(hw,"DAC38J84_CTRL0",0x20,0x4044)
   
   s.spi_write(hw,"DAC38J84_CTRL0",0x3C,reg_3C)
   s.spi_write(hw,"DAC38J84_CTRL0",0x00,reg_00)
   s.spi_write(hw,"DAC38J84_CTRL0",0x25,reg_25)
   s.spi_write(hw,"DAC38J84_CTRL0",0x31,reg_31)
   s.spi_write(hw,"DAC38J84_CTRL0",0x3B,reg_3B)
   s.spi_write(hw,"DAC38J84_CTRL0",0x32,reg_32)
   s.spi_write(hw,"DAC38J84_CTRL0",0x3E,reg_3E)
   s.spi_write(hw,"DAC38J84_CTRL0",0x5F,reg_5F)
   s.spi_write(hw,"DAC38J84_CTRL0",0x60,reg_60)
   
   time.sleep(0.500)
   hw.getNode("axi_fmc144_8lane.FMC144_ctrl.dac_ctrl").write(0x01)
   hw.dispatch()
   
   time.sleep(0.500)
   s.spi_write(hw,"DAC38J84_CTRL0",0x4A,0xFF1F)
   s.spi_write(hw,"DAC38J84_CTRL0",0x4A,0xFF01)
   time.sleep(0.800)
   
   s.spi_write(hw,"DAC38J84_CTRL0",0x6C,0x0)
   s.spi_write(hw,"DAC38J84_CTRL0",0x6D,0x0)
   
   for i in range(8):
     s.spi_write(hw,"DAC38J84_CTRL0",(0x64+i),0x0)
   
   time.sleep(0.8)
   print ("Status")
   print ("........................")
   
   dword = s.spi_read(hw,"DAC38J84_CTRL0",0x6C)
   dword = int(dword,0) & ~0x2
   
   if DACPLL_ENABLE == 0:
     dword = dword & ~0x3
   
   if dword !=0:
     print ("PLL LOCK ERROR")
     
   dword = s.spi_read(hw,"DAC38J84_CTRL0",0x6D)
   if dword != 0:
     print ("LANE LOSS SIGNAL")
     
   for i in range(8):
     dword = s.spi_read(hw,"DAC38J84_CTRL0",0x64+i)
     if int(dword,0) == 0x2 or int(dword,0) == 0x3 or int(dword,0) == 0x0:
       print ("LANE%d STATUS: OK" %i)
     else:
       print ("LANE%d STATUS: Error. Register is %x" %(i,int(dword,0)))
     if i == 3 and number_of_lanes != ADC_MODE_4CH_8LANE:
       break
       
   print("......................................")
   
   for i in range(4):
     dword = s.spi_read(hw,"DAC38J84_CTRL0",0x41+i)
     if int(dword,0) != 0x0:
       print ("LINK%d %d errors found!!!" %(i,int(dword,0)))
   
   return 1
     
	 
   
def FMC14x_init (hw,clockmode,adc_mode):
  
  if clockmode == CLOCKTREE_CLKSRC_DDS:
    hw.getNode("axi_fmc144_8lane.FMC144_ctrl.pin_ctrl").write(0x04)
    hw.dispatch()
  elif clockmode == CLOCKTREE_CLKSRC_EXTERNAL or clockmode == CLOCKTREE_CLKSRC_EXTERNAL_REF:
    hw.getNode("axi_fmc144_8lane.FMC144_ctrl.pin_ctrl").write(0x08)
    hw.dispatch()
  else:
    hw.getNode("axi_fmc144_8lane.FMC144_ctrl.pin_ctrl").write(0x10)
    hw.dispatch()
    dword = hw.getNode("axi_fmc144_8lane.FMC144_ctrl.pin_ctrl").read()
    hw.dispatch()
  
  print ("Configuring the clock tree ...")
  FMC14x_clocktree_init(hw,clockmode)
  print ("Configuring ADCs ...")
  if FMC14x_adc_init(hw,adc_mode) == 0:
    print ("Could not initialise FMC14x.ADC")
    
  #Configure Transceiver
  #Assert Transceiver Reset
  hw.getNode("axi_fmc144_8lane.FMC144_ctrl.transceiver").write(0x01)
  hw.dispatch()
  time.sleep(0.010)
  #Release Transceiver Reset
  hw.getNode("axi_fmc144_8lane.FMC144_ctrl.transceiver").write(0x00)
  hw.dispatch()
  print("Wait for QPLL to lock")
  time.sleep(15)
  #Enable manual bit alignment
  hw.getNode("axi_fmc144_8lane.FMC144_ctrl.transceiver").write(0x10)
  hw.dispatch()
  print("Wait for alignment to complete")
  time.sleep(15)
  print ("Configuring DAC ...")
  if FMC14x_dac_init(hw,0,adc_mode) == 0:
    print ("Could not initialise FMC14x.DAC0")
    
  current0 = hw.getNode("axi_fmc144_8lane.FMC144_ctrl.adc0_stat").read()
  hw.dispatch()
  
  print("Expecting JESD to be ready soon, please wait!")
  for xx in range(100):
    time.sleep(0.3)
    next0 = hw.getNode("axi_fmc144_8lane.FMC144_ctrl.adc0_stat").read()
    hw.dispatch()
    if current0 == next0:
      print("JESD READY!")
      break
    else:
      print(".")
      current0 = next0
  
  print("Done!")
  # Check ADC0 status
  dword = s.spi_read(hw,"ADC16DX370_CTRL0",0x6C)
  print ("---------ADC0 Status -------------")
  print("CLKIN Detected..: %s" %("No","Yes") [int(dword,0) >> 0 & 0x1 ])
  print("CAL_DONE........: %s" %("No","Yes") [int(dword,0) >> 1 & 0x1 ])
  print("PLL_locked......: %s" %("No","Yes") [int(dword,0) >> 2 & 0x1 ])
  print("ALIGN...........: %s" %("No","Yes") [int(dword,0) >> 4 & 0x1 ])
  print("LINK............: %s" %("No","Yes") [int(dword,0) >> 6 & 0x1 ])
  print("....................................")
  
  if adc_mode == ADC_MODE_4CH_4LANE or adc_mode == ADC_MODE_4CH_8LANE:
    #Check ADC1 status
    dword = s.spi_read(hw,"ADC16DX370_CTRL1",0x6C)
    print ("---------ADC1 Status -------------")
    print("CLKIN Detected..: %s" %("No","Yes") [int(dword,0) >> 0 & 0x1 ])
    print("CAL_DONE........: %s" %("No","Yes") [int(dword,0) >> 1 & 0x1 ])
    print("PLL_locked......: %s" %("No","Yes") [int(dword,0) >> 2 & 0x1 ])
    print("ALIGN...........: %s" %("No","Yes") [int(dword,0) >> 4 & 0x1 ])
    print("LINK............: %s" %("No","Yes") [int(dword,0) >> 6 & 0x1 ])
    print("....................................")
	    
  
  
  return 1
