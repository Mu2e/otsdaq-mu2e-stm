#!/usr/bin/python

# -*- coding: utf-8 -*-
import sys
import time
import uhal
import array as arr
import math
import numpy as np

DATAGEN_SINE_WAVE = 0
DATAGEN_SAW_WAVE  = 1



def GenerateWaveform16 (numbersamples, period, amplitude,datatype):
  
  amp = 0
  pi = 3.1415926535
  tmp2 = 0x0
  
  wav_buffer = arr.array('h', [0]*numbersamples)
  
  if datatype == DATAGEN_SINE_WAVE:
    for i in range (numbersamples/2):
      ampl = amplitude/2 -1
      x = (pi/period*2)*(i*2+0)
      y = math.sin(x)
      wav_buffer[2*i+0] = (int(y*ampl) )
      x = (pi/period*2)*(i*2+1)
      y = math.sin(x)
      wav_buffer[2*i+1] = (int(y*ampl) )
   
  elif datatype == DATAGEN_SAW_WAVE:
    for i in range(numbersamples/2):
      tmp2 = (0xffff) & ((2*i)%period*(amplitude -1)/period*2)
      wav_buffer[2*i+0] = (int(tmp2) )
      tmp2 = (0xffff) & ((2*i)%period*(amplitude -1)/period*2)
      wav_buffer[2*i+1] = int(tmp2) 
      
  
  return wav_buffer

def int2bin(integer,digits):
  if integer >= 0:
    return bin(integer)[2:].zfill(digits)
  else:
    return bin (2**digits + integer) [2:]
