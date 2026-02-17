import numpy as np
import math
import psutil
import queue
import os
import time
import plotly.graph_objects as go
from datetime import datetime, timedelta
from utils.shared_memory import SharedMemoryReader
from utils.config import get_xml_node_value

# Get num peaks
peak_min = int(get_xml_node_value("dqm/pulses/min_height"))
peak_max = 0
bin_num = int(get_xml_node_value("dqm/pulses/peak_nbins"))
window_s = float(get_xml_node_value("baseline/hist/window_period"))
bin_width = (peak_max - peak_min)/bin_num
bin_edges = np.linspace(peak_min,peak_max,bin_num)
bin_centres = np.linspace(peak_min+bin_width/2, peak_max-bin_width/2, bin_num)

# Define the expected shared memory header layout
# <Q Q Q => (uint64_t, uint64_t, size_t)
HEADER_FORMAT = f"<Q Q Q"

# Shared memory block reader for ADC baseline
reader = SharedMemoryReader("/dqm_peak_data", HEADER_FORMAT)

def run_worker(task_queue, result_queue, core_id):
    # Pin worker to CPU core
    p = psutil.Process(os.getpid())
    try:
        p.cpu_affinity([core_id])
        print(f"DQM peaks worker pinned to core {core_id}")
    except Exception as e:
        print(f"DQM peak worker could no set affinity: {e}")

    while True:
        try:
            task = task_queue.get(timeout=0.5)
        except queue.Empty:
            continue

        if task is None:
            print(f"DQM peaks worker received shutdown signal. Exiting...")

        try:
            status, hist_all, hist_window = draw_peaks()
            result_queue.put((status, hist_all, hist_window))
        except Exception as e:
            print(f"Error in peaks worker: {e}")
            result_queue.put((None, None, None))


def initialise_plot():

    empty_peak = go.Figure(layout=dict(
                title=dict(
                    text=f"Histogram of peak heights",
                    font=dict(size=20, family="Arial", color="black"),
                    x=0.5,  # Center title (optional)
                    xanchor="center",
                    y = 0.9,
                    yanchor = "top"
                ),
                xaxis=dict(
                    title=dict(
                        text="Peak height (ADC counts)",
                        font=dict(size=18)  # Set your desired font size here
                    ),
                    tickformat="%H:%M:%S.%f",
                    range=[peak_min, peak_max]
                ),
                yaxis=dict(
                    title=dict(
                        text="Number of peaks per bin",
                        font=dict(size=18)
                    ),
                    type="log",
                    range=[0,5],
                    minor = dict(dtick="D1"),
                ),
                margin={"l": 40, "r": 10, "t": 100, "b": 40},
            ))

    return empty_peak

def draw_peaks():
    
    empty_all = initialise_plot()
    empty_all.update_layout(
            title=f"Histogram of peaks since start of run")
    empty_window = initialise_plot()
    empty_window.update_layout(
            title=f"Histogram of {window_s} s of peaks")

    # Read the memory block and extract data
    data, status = reader.read_updated()
    if not data:
        return status, empty_all, empty_window
    elif len(data) != 3 and data == "no update":
        return status, dash.no_update, dash.no_update

    ((timestamp_ns, nbins), hist_all, hist_window) = data
    
    # Get date and time from timestamp
    dt = datetime.fromtimestamp(timestamp_ns / 1e9)

    # --- Draw peak hist ---
    peak_all = go.Figure(empty_all)
    peak_all.add_trace(go.Bar(
        y=hist_all,
        x=bin_centres,
    ))
    peak_all.update_layout(
            title=f"Histogram of peaks since start of run",
            yaxis=dict(
                range=[0,math.ceil(np.log10(max(hist_all, default=1e5)*1.2))]
    ))
    
    peak_window = go.Figure(empty_window)
    peak_window.add_trace(go.Bar(
        y=hist_window,
        x=bin_centres
    ))
    peak_window.update_layout(
            title=f"Histogram of {window_s} s of peaks",
            yaxis=dict(
                range=[0,math.ceil(np.log10(max(hist_window, default=1e5)*1.2))]
    ))


    return status, peak_all, peak_window

