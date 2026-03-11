import psutil
import queue
import os
import time
import dash
import plotly.graph_objects as go
from datetime import datetime, timedelta
from utils.shared_memory import SharedMemoryReader
from utils.config import get_xml_node_value

# For raw data
dec_value = 100 # Data decimation value
fADC = float(get_xml_node_value("fw/fADC")) # ADC frequency (MHz)
raw_period = float(get_xml_node_value("dqm/raw_data/raw_length")) # Raw data period (us)
raw_len = int(raw_period*fADC) # Raw data length (ADCs)
x_times = [i / fADC for i in range(raw_len)]  # µs
x_times = x_times[::dec_value] # Decimate data

# Define the expected shared memory layout
# <Q Q Q => (uint64_t, size_t, uint64_t)
HEADER_FORMAT = f"<Q Q Q"

# Shared memory block reader for ADC baseline
reader = SharedMemoryReader("/dqm_raw_data", HEADER_FORMAT)

def run_worker(task_queue, result_queue, core_id):
    # Pin worker to CPU core
    p = psutil.Process(os.getpid())
    try:
        p.cpu_affinity([core_id])
        print(f"DQM raw worker pinned to core {core_id}")
    except Exception as e:
        print(f"DQM raw worker could no set affinity: {e}")

    while True:
        try:
            task = task_queue.get(timeout=0.5)
        except queue.Empty:
            continue

        if task is None:
            print(f"DQM raw worker received shutdown signal. Exiting...")

        try:
            status, plot = draw_raw()
            result_queue.put((status, plot))
        except Exception as e:
            print(f"Error in raw worker: {e}")
            result_queue.put((None, None))


def draw_raw():

    # Empty layout figures with titles and axes but no data
    empty_raw = go.Figure()
    empty_raw.update_layout(
        title=dict(
            text=f"Raw ADC Data Samples: {raw_period} us from",
            font=dict(size=20, family="Arial", color="black"),
            x=0.5,  # Center title (optional)
            xanchor="center"
        ),
        xaxis=dict(
            title=dict(
                text="Time (µs)",
                font=dict(size=18)  # Set your desired font size here
            ),
            tickformat="%H:%M:%S.%f",
            range=[min(x_times), max(x_times)]
        ),
        yaxis=dict(
            title=dict(
                text="ADC Counts",
                font=dict(size=18)
            ),
            range=[-3000,500]  # Set your fixed y-axis range here
        ),
        margin = {"r": 10}
    )

    # Read the memory block and extract data
    data, status = reader.read_updated()
    if not data:
        return status, empty_raw
    elif len(data) != 3 and data == "no update":
        return status, dash.no_update

    (timestamp_ns,raw_len,raw_samples) = data
    
    # Get date and time from timestamp
    dt = datetime.fromtimestamp(timestamp_ns / 1e9)

    raw_figure = go.Figure(empty_raw)

    # Decimate data
    dec_raw_data = raw_samples[::dec_value]

    raw_figure.add_trace(go.Scatter(
        x=x_times,
        y=dec_raw_data,
        mode='lines',
        name='Raw samples'
    ))
    raw_figure.update_layout(
        title=dict(
            text=f"Raw ADC Samples: {raw_period} us from {dt.strftime('%H:%M:%S.')}{dt.microsecond // 10000:02d}",
        ),
        xaxis=dict(
            range=[min(x_times), max(x_times)]
        ),
    )

    return status, raw_figure

