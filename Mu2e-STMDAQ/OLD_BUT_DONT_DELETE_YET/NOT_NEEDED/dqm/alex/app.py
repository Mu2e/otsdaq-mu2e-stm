from flask import Flask,render_template,url_for,request,redirect, make_response,jsonify
import random
import json
from time import time
from random import random
import socket

# generate random Gaussian values
import numpy as np
from numpy.random import seed
from numpy.random import randn
from numpy import random
import math
import scipy.stats as stats

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


# Create a datagram socket
udpsock = socket.socket(family=socket.AF_INET, type=socket.SOCK_DGRAM)
udpsock.setsockopt(socket.SOL_SOCKET, socket.SO_REUSEADDR, 1)
# Bind to address and ip
udpsock.bind((localIP, localPort))
udpsock.setblocking(0)

# Start Flask app
app = Flask(__name__)

# Test function to produce a guassian
def gauss():
    mu = random.normal(loc=0, scale=0.1, size=1)
    variance = random.normal(loc=1, scale=0.1, size=1)
    sigma = math.sqrt(variance[0])
    x = np.linspace(mu[0] - 3*sigma, mu[0] + 3*sigma, 100)
    y = stats.norm.pdf(x, mu[0], sigma)
    output = [x,y]
    output = np.transpose(output).tolist()
    return output

# Flask main
@app.route('/', methods=["GET", "POST"])
def main():

    # Firmware register table headers
    reg_headers = ['Register', 'Ready?']
    # Firmware register table entries
    reg_data = [
        {'Register':'ROC lock on DTC fiber', 'Ready?':'Yes'},
        {'Register':'ROC internal PLL DTC lock', 'Ready?':'Yes'},
        {'Register':'DTC-ROC high-speed serial link phase aligment', 'Ready?':'No'},
        {'Register':'ADC high-speed serial link initialised', 'Ready?':'Yes'},
        {'Register':'ADC initial high-speed link initial lane alignment check', 'Ready?':'Yes'},
        {'Register':'ADC clock and SYSREF frequency checks', 'Ready?':'Yes'},
        {'Register':'ROC recovered clock phase check', 'Ready?':'No'},]

    # STM info table headers
    stm_headers = ['Parameter', 'Value']
    # STM info table entries
    stm_data = [
        {'Parameter':'Data directory', 'Value':"/data1/STM_VST_DATA/March24"},
        {'Parameter':'Data size written to disk (Gb)', 'Value':0.0},
        {'Parameter':'Total data write rate (Gbit/s)', 'Value':0.00},
        {'Parameter':'ADC Temperature (C)', 'Value':'0'},
        {'Parameter':'Run Number', 'Value':'1'},
        {'Parameter':'Subrun Number', 'Value':'1'},
        {'Parameter':'Event Number', 'Value':'1'},]

    # Channel info table headers
    ch_headers = ['Parameter', 'Value']
    # Channel info table entries
    ch_data = [
        {'Parameter':'Pulse Rate (kHz)', 'Value':0.0},
        {'Parameter':'ADC Baseline (ADC counts)', 'Value':'4 ± 13'},
        {'Parameter':'Zero-supression rate (%)', 'Value':0},
        {'Parameter':'Prescale value', 'Value':0},
        {'Parameter':'Data write rate (Gbit/s)', 'Value':0.00},
        {'Parameter':'Dropped UDP packets', 'Value':0},]

    # Render template and pass table information to html
    return render_template('stm_dqm.html',
                           reg_headers=reg_headers,reg_data=reg_data,
                           stm_headers=stm_headers,stm_data=stm_data,
                           ch_headers=ch_headers,ch_data=ch_data)

# Function to get data from UDP socket
def get_data_from_socket():
    # Receive byte array from UDP socket
    byte_data,addr = udpsock.recvfrom(bufferSize)    
    # Convert byte array to int16_t array
    y = np.frombuffer(byte_data, dtype=np.int16)
    # Get temperature
    adc_temp_arr = y[0:adc_temp_len]
    set_adc_temp(adc_temp_arr)

    # Get pulse data only
    y = y[adc_temp_len:]

    # Get channel 0 data
    y0 = y[0:int(len(y)/2)]    

    # Get channel 0 data
    y1 = y[int(len(y)/2):]    
    
    # Provide x-values
    x = list(range(len(y0)))
    # Convert to time (ns)
    x = np.array(x)*adc_time    
        
    # Create 2D arrays of x,y
    data0 = [x,y0] # channel 0
    data1 = [x,y1] # channel 0

    # Transpose to html ordeing and ensure list 
    data0 = np.transpose(data0).tolist()
    data1 = np.transpose(data1).tolist()

    # Return data array
    return data0,data1
    
# Function to update data from UDP socket    
@app.route('/data', methods=["GET", "POST"])
def data():

    # Get ch0 data
    data0,data1 = get_data_from_socket()
    # Get ch1 data
    #data2 = gauss()
    # Combine into list
    data = [data0,data1]
    
    # Make json response
    response = make_response(json.dumps(data))
    response.content_type = 'application/json'

    # Return json response
    return response

# Function to update table data
@app.route('/table', methods=["GET", "POST"])
def table():

    # Generate dummy data
    temp = adc_temp
    # Fixed value data
    run = 1
    # Change global varible
    set_subrun()

    # Define dictionary of all data to return
    dictionary_to_return = {
        'temp': temp,
        'run': run,
        'subrun': subrun
    }

    # Return json response
    return jsonify(dictionary_to_return)

# Run app
if __name__ == "__main__":
    app.run(debug=True)
