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

def make_uint64_t(p):
    value = np.uint16(p[3]);
    for i in range(3):
        value <<= 16;
        value |= np.uint16(p[2-i]);
    return value

# Global subrun variable
subrun= 0
# Update global subrun variable
def set_subrun():
    global subrun
    subrun += 1

# Global adc temperature variable
adc_temp = 0
adc_temp_len = 4
# Update global adc_temperature variable
def set_adc_temp(values):
    global adc_temp
    temp = make_uint64_t(values)
    adc_temp = float((temp&0xFFF) / 4.0)


