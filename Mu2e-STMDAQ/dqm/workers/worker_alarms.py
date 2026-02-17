import psutil
import queue
import os
import time
import numpy as np
import plotly.graph_objects as go
from dash import dcc
from datetime import datetime, timedelta
from utils.shared_memory import SharedMemoryReader
from utils.config import get_xml_node_value


# Define the expected shared memory layout
# <Q Q Q => (uint64_t, uint64_t, size_t, size_t, size_t)
HEADER_FORMAT = f"<Q Q Q Q Q"

# Shared memory block reader for alarms
reader = SharedMemoryReader("/dqm_alarm_data", HEADER_FORMAT)

def run_worker(task_queue, result_queue, core_id):
    # Pin worker to CPU core
    p = psutil.Process(os.getpid())
    try:
        p.cpu_affinity([core_id])
        print(f"DQM alarm worker pinned to core {core_id}")
    except Exception as e:
        print(f"DQM alarm worker could no set affinity: {e}")

    while True:
        try:
            task = task_queue.get(timeout=0.5)
        except queue.Empty:
            continue

        if task is None:
            print(f"DQM alarm worker received shutdown signal. Exiting...")

        time_dict = task

        try:
            alert_arr, latest_time = read_alarms(time_dict)
            result_queue.put((alert_arr, latest_time))
        except Exception as e:
            print(f"Error in alarm worker: {e}")
            result_queue.put((None, None))


def read_alarms(time_dict):

    # Get the time of the last alarm processed
    prev_alarm_time = time_dict["latest_alarm_time"]
    
    # Read the memory block and extract data
    data, status = reader.read_updated()
    if not data or len(data) != 2:
        return [], prev_alarm_time

    ((timestamp_ns,
     max_alarms,
     num_alarms,
     write_idx),
     alarm_info) = data

    alerts = []

    # Read in time order 
    sorted_alarms = np.sort(alarm_info, order='time')

    for i in range(num_alarms): 

        # Only read new alarms
        time_ns = sorted_alarms[i]['time'] 
        if time_ns <= prev_alarm_time:
            continue

        # If this is the last alarm, update latest time
        if i == num_alarms-1:
            time_dict["latest_alarm_time"] = time_ns

        # Get formatted time
        dt = datetime.fromtimestamp(time_ns/1e9)
        timestamp_str = dt.strftime('%Y-%m-%d %H:%M:%S.%f')[:-4]

        # Get the alarms text
        alarm_str = sorted_alarms[i]['name'].decode('utf-8').strip('\x00')

        # Get the level colour
        level = sorted_alarms[i]['level']
        if level == 0: 
            level_colour = "red"
        elif level == 2:
            level_colour = "orange"

        # Create the notif dict
        new_alert = {"message" : f"{alarm_str}",
                    "color" : f"{level_colour}",
                    "action" : "show",
                    "autoClose" : 5000
                    }

        # Add to notif array
        alerts.append(new_alert)

    return alerts, time_dict

