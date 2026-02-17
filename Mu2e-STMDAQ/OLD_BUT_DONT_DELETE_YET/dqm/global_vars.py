import os
import numpy as np

# Get STM DAQ directory                                                 
stmdaq_dir=os.environ['STMDAQ_ROOT']

# The name of the STM DAQ screen
stmdaq_screen = "stmdaq"

# Number of channels
CHNUM = 2

# UDP socket IP
localIP     = "127.0.0.2"
# UDP socket port
localPort   = 10000
# UDP socket buffer size (UDP max value)
bufferSize  = 65536

# ADC sampling frequency
samp_freq = 300*1e6
# ADC sampling time (ns)
adc_time = 1/samp_freq*1e9

fADC = 370
tadc = 1.0/fADC
 
before_peak_max_time = 1
before_peak_max = int(before_peak_max_time/tadc)
after_peak_max_time = 2
after_peak_max = int(after_peak_max_time/tadc)
total_peak_max = before_peak_max + after_peak_max

# Print to terminal and to file
class Tee(object):
    def __init__(self, *files):
        self.files = files
    def write(self, obj):
        for f in self.files:
            f.write(obj)
            f.flush() # If you want the output to be visible immediately
    def flush(self) :
        for f in self.files:
            f.flush()

def make_uint32_t(p):
    value = np.uint16(p[1]);    
    value <<= 16;
    value |= np.uint16(p[0])
    # Convert from nupmy int16_t to int 
    value = int(value.item(0))
    return value

def make_uint64_t(p):
    value = np.uint16(p[3]);
    for i in range(3):
        value <<= 16;
        value |= np.uint16(p[2-i]);
    # Convert from nupmy int16_t to int 
    value = int(value.item(0))
    return value


