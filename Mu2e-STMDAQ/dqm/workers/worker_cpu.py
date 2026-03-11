import psutil
import queue
import os
import time
import dash
import plotly.graph_objects as go
from dash import dcc
from datetime import datetime, timedelta
from utils.shared_memory import SharedMemoryReader
from utils.config import get_xml_node_value


# Define the expected shared memory layout
# <Q Q Q => (uint64_t, uint64_t, uint64_t)
HEADER_FORMAT = f"<Q Q Q d Q"

# Shared memory block reader for ADC baseline
reader = SharedMemoryReader("/dqm_daq_data", HEADER_FORMAT)

req_speed = 6

def run_worker(task_queue, result_queue, core_id):
    # Pin worker to CPU core
    p = psutil.Process(os.getpid())
    try:
        p.cpu_affinity([core_id])
        print(f"DQM CPU worker pinned to core {core_id}")
    except Exception as e:
        print(f"DQM CPU worker could no set affinity: {e}")

    while True:
        try:
            task = task_queue.get(timeout=0.5)
        except queue.Empty:
            continue

        if task is None:
            print(f"DQM CPU worker received shutdown signal. Exiting...")

        history = task

        try:
            status, speedos, temp_plot, packets_plot, slow_ops_alarm = draw_speeds(history)
            result_queue.put((status, speedos, temp_plot, packets_plot, slow_ops_alarm))
        except Exception as e:
            print(f"Error in CPU worker: {e}")
            result_queue.put(([None]*5))


def make_speedo(name, speed):
    speedo_plot = go.Figure(go.Indicator(
        mode = "gauge+number+delta",
        value = speed,
        domain = {'x': [0, 1], 'y': [0, 1]},
        title = {'text': name},
        delta = {'reference': req_speed},
        number = {'suffix': " Gbit/s"},
        gauge = {
                'axis': {'range': [0, max(speed*1.05,15)]},
                'steps' : [{'range': [0, req_speed], 'color': "indianred"}],
                }
        ))

    return speedo_plot

def make_temp_plot():
    empty_temp = go.Figure(layout=dict(
        title=dict(
            text="ADC temperature history",
            font=dict(size=20, family="Arial", color="black"),
            x=0.5,  # Center title (optional)
            xanchor="center"
        ),
        xaxis=dict(
            title=dict(
                text="Time",
                font=dict(size=18)
            ),
            tickformat="%H:%M:%S.%f",
        ),
        yaxis=dict(
            title=dict(
                text="ADC temperature [C]",
                font=dict(size=18)
            )
        ),
        margin={"l": 40, "r": 10, "t": 100, "b": 40},
        legend=dict(
            orientation="h",
            yanchor="bottom",
            y=1.02,
            xanchor="center",
            x=0.4,
            font=dict(size=16)
        )
    ))

    return empty_temp

def make_packet_plot():
    empty_packet = go.Figure(layout=dict(
        title=dict(
            text="Dropped packets",
            font=dict(size=20, family="Arial", color="black"),
            x=0.5,  # Center title (optional)
            xanchor="center"
        ),
        xaxis=dict(
            title=dict(
                text="Time",
                font=dict(size=18)
            ),
            tickformat="%H:%M:%S.%f",
        ),
        yaxis=dict(
            title=dict(
                text="Packets dropped",
                font=dict(size=18)
            )
        ),
        margin={"l": 40, "r": 10, "t": 100, "b": 40},
        legend=dict(
            orientation="h",
            yanchor="bottom",
            y=1.02,
            xanchor="center",
            x=0.4,
            font=dict(size=16)
        )
    ))
    return empty_packet

def draw_speeds(history):

    empty_temp = make_temp_plot()
    empty_drop = make_packet_plot()

    empty_speedo = make_speedo("Operation",0)
    empty_arr = [dcc.Graph(figure=empty_speedo)]

    # Read the memory block and extract data
    data, status = reader.read_updated()
    if not data:
        return status, empty_arr, empty_temp, empty_drop, []
    elif len(data) != 2 and data == "no update":
        return status, *([dash.no_update] * 3), []

    ((timestamp_ns,
     num_dropped_packets,
     adc_temp,
     num_ops),
     ops_info) = data
    
    # Get date and time from timestamp
    dt = datetime.fromtimestamp(timestamp_ns / 1e9)

    # Add new values
    history['time'].append(dt)
    history['temp'].append(adc_temp)
    history['dropped_packets'].append(num_dropped_packets)

    # (Optional) Keep only last N points to limit graph load
    max_points = 100
    for key in history:
        if isinstance(history[key], list):
            history[key] = history[key][-max_points:]

    # Fill temperature history
    temp_plot = go.Figure(empty_temp)
    temp_plot.add_trace(go.Scatter(x=history['time'],
                               y=history['temp'],
                               mode='lines',
    ))

    drop_plot = go.Figure(empty_drop)
    drop_plot.add_trace(go.Scatter(x=history['time'],
                                y=history['dropped_packets'],
                                mode='lines'
    ))

    # --- Fill array of operations speed ---
    plots_arr = []
    slow_ops_alarm = []
    alarm_msg = "These operation(s) are too slow: \n"
    for op in ops_info:
        name = op['name'].decode('utf-8').strip('\x00')
        speed = op['speed']

        speedo = make_speedo(name, speed)
        plots_arr.append(dcc.Graph(
            figure=speedo,
            responsive=True,
            style={'height': '100%', 'width': '100%'}
        ))

        if speed < req_speed:
            alarm_msg = alarm_msg + name + ": " + str(speed) + " Gbit/s \n"

    if len(alarm_msg) > 36:
        slow_ops_alarm = [{'action':"show",
                    'title':"CPU performance warning",
                    'message': alarm_msg,
                    }]


    return status, plots_arr, temp_plot, drop_plot, slow_ops_alarm

