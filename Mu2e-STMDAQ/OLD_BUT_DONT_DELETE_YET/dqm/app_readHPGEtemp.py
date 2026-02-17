from flask import Flask,render_template,url_for,request,redirect, make_response,jsonify
import random
import json
from time import time
from random import random
import socket
import logging
import sys
import subprocess
from subprocess import check_output

# Import subroutines
from global_vars import *
from dqm_data import *
from tables import *

# Get python hardware scripts
# caution: path[0] is reserved for script path (or '' in REPL)
hw_dir = stmdaq_dir+'/hardwareScripts/python/'
sys.path.insert(1,hw_dir)
from part_no_read_test_300MSPS_noDAC import *
from dtc_link_reset_stats import *
from check_adc_temp import *
from dtc_sim_test import *

# generate random Gaussian values
import numpy as np
from numpy.random import seed
from numpy.random import randn
from numpy import random
import math
import scipy.stats as stats
import serial
import serial.tools.list_ports

# Get serial ports for HPGe temperature
# ports = serial.tools.list_ports.comports()
# serialInst = serial.Serial()
# serialInst.baudrate = 9600
# serialInst.port = '/dev/ttyACM0'
# serialInst.open()

# Write terminal output to file
fname = stmdaq_dir+'/dqm/output.txt'
f = open(fname, 'w')
original = sys.stdout
sys.stdout = Tee(sys.stdout, f)

# Boolean to indicate if a process is already running
process_running = False
# Update global subrun variable
def toggle_process_running():
    global process_running
    process_running = not process_running 

# Boolean to indicate if adc is initialised
adc_init_finished = False                                                 
# Update global subrun variable
def signal_adc_init():
    global adc_init_finished
    adc_init_finished = True 

# ADC temperature counter
adc_temp_count = 0                               
# Update adc temperature counter
def update_adc_temp_count(value):
    global adc_temp_count
    adc_temp_count = value

# Update daq running variables
def is_daq_running():
    global daq_running
    var = str(check_output(["screen -ls; true"],shell=True))
    if ("."+stmdaq_screen+"\\t(") in var:
        daq_running = True
    else:
        daq_running = False
    return daq_running

# Set data arrays
data0 = []
data1 = []
def store_pulse_data(d0,d1):
    global data0
    global data1
    data0 = d0
    data1 = d1

# Get space left on disk
def get_disk_usage(directory):
    command = "df -h "+directory+" | awk 'END { print $5 }'"
    var = str(check_output([command],shell=True))
    usage = var[2:4]
    return usage

# Create dqm class
dqm = dqm_data()

# Function to get data from UDP socket
def get_data():
    try:
        # Receive byte array from UDP socket
        byte_data,addr = udpsock.recvfrom(bufferSize)    
        # Convert byte array to int16_t array
        data = np.frombuffer(byte_data, dtype=np.int16)
        
        # Get pulse data
        y = data[dqm.tot_info_len:]
        
        # Get channel 0 data
        y0 = y[0:int(len(y)/2)]    
        
        # Get channel 1 data
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

        # Store in memory
        store_pulse_data(data0,data1)

    except:
        return [-1]
    # Return data array
    return data

# Create a datagram socket
udpsock = socket.socket(family=socket.AF_INET, type=socket.SOCK_DGRAM)
udpsock.setsockopt(socket.SOL_SOCKET, socket.SO_REUSEADDR, 1)
# Bind to address and ip
udpsock.bind((localIP, localPort))
# Set non-blocking socket
udpsock.setblocking(0)

# Build start dictionary of empty data
dqm.build_start_dict()

# Start Flask app
app = Flask(__name__)
# Disable json reording dictionaries
app.config['JSON_SORT_KEYS'] = False
# Disable standard python flask loffing
log = logging.getLogger('werkzeug')
log.disabled = True

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

    # Create registers table
    reg_headers,reg_data,reg_ids = create_reg_table(dqm)

    # Create stm info table
    stm_headers,stm_data,stm_ids = create_stm_table(dqm)

    # Create channel info table
    ch0_headers,ch0_data,ch0_ids = create_chan_table(dqm,0)
    ch1_headers,ch1_data,ch1_ids = create_chan_table(dqm,1)
    
    print("DQM running...")

    # Render template and pass table information to html
    return render_template(
        'stm_dqm.html',
        reg_headers=reg_headers,reg_data=reg_data,reg_ids=reg_ids,
        stm_headers=stm_headers,stm_data=stm_data,stm_ids=stm_ids,
        ch0_headers=ch0_headers,ch0_data=ch0_data,ch0_ids=ch0_ids,
        ch1_headers=ch1_headers,ch1_data=ch1_data,ch1_ids=ch1_ids)

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
    d0 = [x,y0] # channel 0
    d1 = [x,y1] # channel 0

    # Transpose to html ordeing and ensure list 
    d0 = np.transpose(d0).tolist()
    d1 = np.transpose(d1).tolist()

    # Return data array
    return d0,d1
    
# Function to update data from UDP socket    
@app.route('/data', methods=["GET", "POST"])
def data():

    # Get ch0 data
    #data0,data1 = get_data_from_socket()

    # Combine into list
    #data = [gauss(),gauss()]
    data = [data0,data1]
    
    # Make json response
    response = make_response(json.dumps(data))
    response.content_type = 'application/json'

    # Return json response
    return response

# Function to update table data
@app.route('/table', methods=["GET", "POST"])
def table():
    
     # Get data from socket
    data = get_data()
    # Build dictionary of data
    if (data[0] != -1): dqm.build_info_dict(data)
    # #  Don't check the ADC unless it has been initialised
    # print(adc_init_finished)
    # if (adc_init_finished == True):
    #     if (adc_temp_count == 0):
    #         # Get ADC temp        
    #         temp = get_adc_temp(hw_dir)    
    #         print(temp)
    #         if (temp == -1):
    #             dqm.info_dict["adc_temp"][0]["value"] = "Check failed"
    #         else:
    #             dqm.info_dict["adc_temp"][0]["value"] = ("%.2f" % float(temp))
    #     update_adc_temp_count(adc_temp_count+1)
    #     print(adc_temp_count)
    #     if (adc_temp_count == 10): update_adc_temp_count(0)
    
    # # Get HPGe Temperature
    # if serialInst.in_waiting:
    #     packet = serialInst.readline()
    #     hpge_temp = float(packet.decode('utf8'))
    #     dqm.info_dict["hpge_temp"][0]["value"] = hpge_temp

    # Check DTC link
    success = check_dtc_link(hw_dir)
    if (success == 1):
        dqm.info_dict["dtc_link_up"][0]["value"] = "Yes"
    elif (success == 0):
        dqm.info_dict["dtc_link_up"][0]["value"] = "No"
    else:
        dqm.info_dict["dtc_link_up"][0]["value"] = "Check failed"

    # Check if the DAQ is running
    success = is_daq_running()
    if (success):
        dqm.info_dict["daq_running"][0]["value"] = "Yes"
    else:
        dqm.info_dict["daq_running"][0]["value"] = "No"

    # Check DTC counters
    counters = dtc_check_counters(hw_dir)    
    if (counters[0] == -1):
        dqm.info_dict["evm_hb_seq_err_count"][0]["value"] = "Check failed"
    else:
        dqm.info_dict["evm_hb_seq_err_count"][0]["value"] = counters[0]
    if (counters[1] == -1):
        dqm.info_dict["hb_evm_int_err_count"][0]["value"] = "Check failed"
    else:
        dqm.info_dict["hb_evm_int_err_count"][0]["value"] = counters[1]
    if (counters[2] == -1):
        dqm.info_dict["hb_crc_err_count"][0]["value"] = "Check failed"
    else:
        dqm.info_dict["hb_crc_err_count"][0]["value"] = counters[2]
    if (counters[3] == -1):
        dqm.info_dict["event_tag_skipped_count"][0]["value"] = "Check failed"
    else:
        dqm.info_dict["event_tag_skipped_count"][0]["value"] = counters[3]
    if (counters[4] == -1):
        dqm.info_dict["dtc_clk_marker_sync_off"][0]["value"] = "Check failed"
    else:
        dqm.info_dict["dtc_clk_marker_sync_off"][0]["value"] = counters[4]

    # Get disk usage
    directory=str(dqm.info_dict["data_dir"][0]["value"])
    #if (directory != " - "):
        #usage = get_disk_usage(directory)
        #dqm.info_dict["disk_usage"][0]["value"] = usage

    # Get last line of output file as string
    with open(fname, 'rb') as f:
        try:  # catch OSError in case of a one line file 
            f.seek(-2, os.SEEK_END)
            while f.read(1) != b'\n':
                f.seek(-2, os.SEEK_CUR)
        except OSError:
            f.seek(0)
        last_line = str(f.readline().decode())
    dqm.info_dict["last_line"] = last_line

    # Return json response
    return jsonify(dqm.info_dict)

#background process happening without any refreshing
@app.route('/init_adc')
def init_adc():

    # Check if a process is already running
    if (process_running == True):
        print("A process is already runnning. Please wait for it to finish.")
    else:
        # Signal a process is running
        toggle_process_running()
        # Change init adc value in data dictionary
        dqm.info_dict["adc_init"][0]["value"] = "Working..."
        # Check to see if the ADC has already been initialised
        init = check_adc_status(hw_dir)
        # If the ADC has already been initialised
        if (init == 1):        
            print("ADC already initialised!")            
            # Signal that the ADC has been initialised            
            dqm.info_dict["adc_init"][0]["value"] = "Yes"
            # Signal that the ADC has been initialised
            signal_adc_init()
        # Else the ADC has not been initialised
        else:
            # Initialise adc
            print("Starting ADC initialisation...")            
            success = initADC(hw_dir)
            if (success == 1):
                dqm.info_dict["adc_init"][0]["value"] = "Yes"
                # Signal that the ADC has been initialised
                signal_adc_init()
            else:
                dqm.info_dict["adc_init"][0]["value"] = "Failed"
        # Signal the process has stopped
        toggle_process_running()

        
    # Non-return
    return ("nothing")

#background process happening without any refreshing
@app.route('/dtc_check')
def dtc_check():

    # Check if a process is already running
    if (process_running == True):
        print("A process is already runnning. Please wait for it to finish.")
    else:
        # Signal a process is running
        toggle_process_running()
        # Change init adc value in data dictionary
        dqm.info_dict["dtc_comma_char"][0]["value"] = "Working..."        
        # Initialise adc
        errors =  dtc_comma_check(hw_dir)
        if (errors == -1):
            print("DTC communication error")
            dqm.info_dict["dtc_comma_char"][0]["value"] = "Check failed"
        else:
            if (errors == 0):
                print("DTC check passed")
            else:
                print("DTC check failed")
            dqm.info_dict["dtc_comma_char"][0]["value"] = errors
        # Signal the process has stopped
        toggle_process_running()
        
    # Non-return
    return ("nothing")

#background process happening without any refreshing
@app.route('/start_readout')
def start_readout():

    # Start 10G readout
    start_10G_readout(hw_dir)
    print("Started 10G readout")
        
    # Non-return
    return ("nothing")

#background process happening without any refreshing
@app.route('/start_daq')
def start_daq():

    # # Check first to see if the DAQ is running
    # if (daq_running):    
    #     print("STM DAQ is already running!")
    # # If not already running...
    # else:
    #     print("Starting STM DAQ...")        
    #     # Create a screen to start the DAQ
    # command1 = 'screen -dmS ' + stmdaq_screen + ' bash -C "cd $STMDAQ_ROOT; source setup.sh; ./build/bin/testDQM.exe"'
    # print(command1)
    # subprocess.run(command1, shell = True, executable="/bin/bash")
    print("STM DAQ started.")        
    
    # Non-return
    return ("nothing")


#background process happening without any refreshing
@app.route('/reset_readout')
def reset():

    # Start 10G readout
    reset_readout(hw_dir)
    print("Reset readout")
        
    # Non-return
    return ("nothing")

# Run app
if __name__ == "__main__":
    app.run(debug=True)
