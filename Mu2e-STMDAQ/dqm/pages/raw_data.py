import dash
import numpy as np
from dash import html, dcc, callback, Output, Input, State
import plotly.graph_objs as go
from datetime import datetime, timedelta
from utils.shared_memory import SharedMemoryReader
from utils.config import get_xml_node_value

# Register this script as a Dash page 
dash.register_page(__name__, path="/raw-data", name="Raw Data", layout=lambda: layout())

# For raw data
dec_value = 100 # Data decimation value
fADC = float(get_xml_node_value("fw/fADC")) # ADC frequency (MHz)
raw_period = float(get_xml_node_value("dqm/raw_data/raw_length")) # Raw data period (us)
raw_len = int(raw_period*fADC) # Raw data length (ADCs)
x_times = [i / fADC for i in range(raw_len)]  # µs
x_times = x_times[::dec_value] # Decimate data

# Define the expected shared memory layout
# <Q Q d*6 Q => (uint64_t, uint64_t, data,  uint64_t)
STRUCT_FORMAT = f"<Q Q {raw_len}h Q"

# Shared memory block reader for ADC baseline
reader = SharedMemoryReader("/dqm_raw_data", STRUCT_FORMAT)

# Signal first display
first_display = True

# Lazy-loaded layout function to reduce startup cost
def layout():
    return html.Div([

        # Track URL so we can poll only when on this page
        dcc.Location(id="raw-url", refresh=False),

        # Title and status message
        html.Div([
            html.H2("Raw Detector Data", style={"margin": 0}),
            html.Div([
                html.Span("Status: ", style={"color": "black", "fontWeight": "bold"}),
                html.Span(id="raw-status-message")
            ], style={"marginLeft": "auto", "display": "flex", "alignItems": "center", "gap": "5px"})
        ], style={"display": "flex", "justifyContent": "space-between", "alignItems": "center", "padding": "0 20px", "marginBottom": "10px"}),
        
        # Main graph showing raw data evolution
        html.Div(
            dcc.Graph(id="raw-data-graph"),
            style={"width": "100%", "height": "1400px", "margin": "auto"}
        ),
        
        # Persistent store holding time history 
        dcc.Store(id="time-history", data={
            "x": [],
            "last_timestamp": 0
        }),

        # Latest shared memory read for callbacks
        dcc.Store(id="raw-latest", data={}),

        # Timer to trigger polling shared memory
        # Poll slower to avoid callback overlap (650 ms read latency => 2000 ms interval)
        dcc.Interval(id="raw-interval-component", interval=1000, n_intervals=0),
    ])


# Read and parse the latest shared memory values every interval
@callback(
    Output("raw-latest", "data"),
    Input("raw-url", "pathname"),
    Input("raw-interval-component", "n_intervals")
)
def poll_shared_memory(pathname, n):
    """
    Polls shared memory once per interval. Returns a parsed struct or wait status.
    This callback only runs when the user is on this specific page.
    """
    if pathname != "/raw-data":
        return dash.no_update

    # Read the memory block and extract data
    data, status = reader.read_updated()
    if not data or len(data) != raw_len+3:  
        return {"status": status}
    
    (
        gen_start,
        timestamp_ns,
        *rest
    ) = data
    
    # Last element is gen_end
    raw_samples = rest[:-1]
    gen_end = rest[-1]
    
    return {
        "timestamp": timestamp_ns,
        "raw_samples": raw_samples,
        "status": status
    }

# Update the graph and accumulated baseline history
@callback(
    Output("raw-data-graph", "figure"),
    Output("time-history", "data"),
    Input("raw-latest", "data"),
    State("time-history", "data")
)
def update_graph(latest, history):
    global first_display
    
    """
    Updates the plot with the latest shared memory data.
    Uses history to retain a sliding window of values.
    Skips updates if timestamp hasn't changed.
    """
    # if not latest or "timestamp" not in latest:
    if not latest or "timestamp" not in latest:
        # If this is the first display
        if (first_display == True):
            # Return empty layout figures with titles and axes but no data
            empty_raw = go.Figure(layout=dict(
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
                    range=[-3000,200]  # Set your fixed y-axis range here
                ),
                height=800
            ))
            
            return empty_raw, history
        
        # If this is NOT the first display
        else:
            return dash.no_update, history
        

    # Set first display to false
    first_display = False
    
    # Get the timestamp
    timestamp = latest["timestamp"]

    # Skip update if the timestamp hasn't changed since last read
    if timestamp == history.get("last_timestamp", 0):
        return dash.no_update, history

    # Convert timestamp to datetime object
    #dt = datetime.datetime.fromtimestamp(timestamp / 1e9)
    dt = datetime.fromtimestamp(timestamp / 1e9)

    # Append new values to time series lists
    history["x"].append(dt)
    history["last_timestamp"] = timestamp

    # (Optional) Keep only last N points to limit graph load
    max_points = 2
    for key in history:
        if isinstance(history[key], list):
            history[key] = history[key][-max_points:]
    
    # --- Raw graph ---
    raw_data = latest["raw_samples"]
    raw_figure = go.Figure()
    
    # Decimate data
    dec_raw_data = raw_data[::dec_value]
    
    raw_figure.add_trace(go.Scatter(
        x=x_times,
        y=dec_raw_data,
        mode='markers',
        name='Raw samples'
    ))
    raw_figure.update_layout(
        title=dict(
            text=f"Raw ADC Samples: {raw_period} us from {dt.strftime('%H:%M:%S.')}{dt.microsecond // 10000:02d}",
            font=dict(size=20, family="Arial", color="black"),
            x=0.5,  # Center title (optional)
            xanchor="center"
        ),
        xaxis=dict(
            title=dict(
                text="Time (µs)",
                font=dict(size=18)  # Set your desired font size here
            ),
            tickfont=dict(size=16),
            tickformat="%H:%M:%S.%f",
            range=[min(x_times), max(x_times)]
        ),
        yaxis=dict(
            title=dict(
                text="ADC Counts",
                font=dict(size=18)
            ),
            range=[-3000,200]  # Set your fixed y-axis range here            
        ),
        height=800
    )
    
    return raw_figure, history


# Show the current read status message above the graph
@callback(
    Output("raw-status-message", "children"),
    Output("raw-status-message", "style"),
    Input("raw-latest", "data")
)
def update_status(latest):
    """
    Displays a color-coded status message above the graph.
    """
    
    if latest is None:
        return "", {}
    
    status = latest.get("status", "")
    if status == "waiting for shared memory":
        return "No shared memory connection...", {"color": "red", "fontWeight": "bold"}
    elif status == "waiting for new data":
        return "Waiting for raw data...", {"color": "red", "fontWeight": "bold"}
    else:
        return "Receiving raw data...", {"color": "green", "fontWeight": "bold"}
