import dash
from dash import html, dcc, callback, Output, Input, State
import plotly.graph_objs as go
from datetime import datetime, timedelta
from utils.shared_memory import SharedMemoryReader
from utils.config import get_xml_node_value

# Register this script as a Dash page at "/adc-baseline"
# NOTE: layout is now provided as a callable for lazy loading
dash.register_page(__name__, path="/adc-baseline", name="ADC Baseline and Noise", layout=lambda: layout())

# For noise data
fADC = float(get_xml_node_value("fw/fADC")) # ADC frequency (MHz)
noise_period = float(get_xml_node_value("dqm/baseline/noise_length")) # Noise period (us)
noise_len = int(noise_period*fADC) # Noise length (ADCs)
x_times = [i / fADC for i in range(noise_len)]  # µs

# Define the expected shared memory layout
# <Q Q d*6 Q => (uint64_t, uint64_t, 6 doubles, data, uint64_t)
STRUCT_FORMAT = f"<Q Q dddddd {noise_len}h Q"

# Shared memory block reader for ADC baseline
reader = SharedMemoryReader("/dqm_adc_baseline", STRUCT_FORMAT)

# Signal first display
first_display = True

# Lazy-loaded layout function to reduce startup cost
def layout():
    return html.Div([

        # Track URL so we can poll only when on this page
        dcc.Location(id="url", refresh=False),

        # Title and status message
        html.Div([
            html.H2("ADC Baseline and Noise", style={"margin": 0}),
            html.Div([
                html.Span("Status: ", style={"color": "black", "fontWeight": "bold"}),
                html.Span(id="status-message")
            ], style={"marginLeft": "auto", "display": "flex", "alignItems": "center", "gap": "5px"})
        ], style={"display": "flex", "justifyContent": "space-between", "alignItems": "center", "padding": "0 20px", "marginBottom": "10px"}),
        
        # html.H2("ADC Baseline and Noise", style={"paddingLeft": "20px"}),

        # # Status message above graph
        # html.Div(id="status-message"),

        # Main graph showing baseline evolution
        dcc.Graph(id="adc-baseline-graph"),

        # Persistent store holding time history of baseline values
        dcc.Store(id="baseline-history", data={
            "x": [],
            "prev": [],
            "prev_rms": [],
            "avg": [],
            "avg_rms": [],
            "curr": [],
            "curr_rms": [],
            "last_timestamp": 0
        }),

        # Latest shared memory read for callbacks
        dcc.Store(id="baseline-latest", data={}),

        # Noise graph
        dcc.Graph(id='noise-graph'),
        
        # Timer to trigger polling shared memory
        # Poll slower to avoid callback overlap (650 ms read latency => 2000 ms interval)
        dcc.Interval(id="interval-component", interval=1000, n_intervals=0),
    ])


# Read and parse the latest shared memory values every interval
@callback(
    Output("baseline-latest", "data"),
    Input("url", "pathname"),
    Input("interval-component", "n_intervals")
)
def poll_shared_memory(pathname, n):
    """
    Polls shared memory once per interval. Returns a parsed struct or wait status.
    This callback only runs when the user is on this specific page.
    """
    if pathname != "/adc-baseline":
        return dash.no_update

    # Read the memory block and extract data
    data, status = reader.read_updated()
    if not data or len(data) != 3009:  # 9 original + 300 noise samples
        return {"status": status}
    
    (
        gen_start,
        timestamp_ns,
        mean_prev,
        rms_prev,
        mean_avg,
        rms_avg,
        mean_curr,
        rms_curr,
        *rest
    ) = data
    
    # Last element is gen_end
    noise_samples = rest[:-1]
    gen_end = rest[-1]
    
    return {
        "timestamp": timestamp_ns,
        "mean_prev": mean_prev,
        "rms_prev": rms_prev,
        "mean_avg": mean_avg,
        "rms_avg": rms_avg,
        "mean_curr": mean_curr,
        "rms_curr": rms_curr,
        "noise_samples": noise_samples,
        "status": status
    }

# Update the graph and accumulated baseline history
@callback(
    Output("adc-baseline-graph", "figure"),
    Output('noise-graph', 'figure'),
    Output("baseline-history", "data"),
    Input("baseline-latest", "data"),
    State("baseline-history", "data")
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
            empty_time_series = go.Figure(layout=dict(
                title=dict(
                    text="Average of ADC baseline noise over run",
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
                    )
                ),
                height=400,
                margin={"l": 40, "r": 10, "t": 100, "b": 40},
                legend=dict(
                    orientation="h",
                    yanchor="bottom",
                    y=1.02,
                    xanchor="center",
                    x=0.4,
                    font=dict(size=18)
                )
            ))
            
            empty_noise = go.Figure(layout=dict(
                title=dict(
                    text=f"ADC Noise Samples: {noise_period} us from",
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
                    )
                ),
                height=400
            ))
            
            return empty_time_series, empty_noise, history
        
        # If this is NOT the first display
        else:
            return dash.no_update, dash.no_update, history
        

    # Set first display to false
    first_display = False
    
    # Get the timestamp
    timestamp = latest["timestamp"]

    # Skip update if the timestamp hasn't changed since last read
    if timestamp == history.get("last_timestamp", 0):
        return dash.no_update, dash.no_update, history

    # Convert timestamp to datetime object
    #dt = datetime.datetime.fromtimestamp(timestamp / 1e9)
    dt = datetime.fromtimestamp(timestamp / 1e9)

    # Append new values to time series lists
    history["x"].append(dt)
    history["prev"].append(latest["mean_prev"])
    history["prev_rms"].append(latest["rms_prev"])
    history["avg"].append(latest["mean_avg"])
    history["avg_rms"].append(latest["rms_avg"])
    history["curr"].append(latest["mean_curr"])
    history["curr_rms"].append(latest["rms_curr"])
    history["last_timestamp"] = timestamp

    # (Optional) Keep only last N points to limit graph load
    max_points = 100
    for key in history:
        if isinstance(history[key], list):
            history[key] = history[key][-max_points:]

    # Precompute bands
    hx = history["x"]
    prev_upper = [m + r for m, r in zip(history["prev"], history["prev_rms"])]
    prev_lower = [m - r for m, r in zip(history["prev"], history["prev_rms"])]

    # Build figure with WebGL backend for speed
    fig = go.Figure()

    # Get last values
    last_prev = history["prev"][-1]
    last_prev_rms = history["prev_rms"][-1]
    last_avg = history["avg"][-1]
    last_avg_rms = history["avg_rms"][-1]
    last_curr = history["curr"][-1]
    last_curr_rms = history["curr_rms"][-1]
    
    # Previous band
    fig.add_trace(go.Scattergl(x=hx, # upper line
                               y=prev_upper,
                               mode='lines',
                               line=dict(color='blue', dash='dash'),
                               name=f"Reference (e.g. average from previous run) = {last_prev:.2f} ± {last_prev_rms:.2f}\u00A0\u00A0\u00A0"))
    fig.add_trace(go.Scattergl(x=hx, # lower line
                               y=prev_lower,
                               mode='lines',
                               line=dict(color='blue', dash='dash'),
                               showlegend=False, name=""))
        
     # Average since start of run
    fig.add_trace(go.Scatter(x=hx,
                             y=history["avg"],
                             mode='lines+markers',
                             name=f"Rolling average (this run) = {last_avg:.2f} ± {last_avg_rms:.2f}\u00A0\u00A0\u00A0",
                             line=dict(dash='dash'),
                             marker=dict(color='green', size=6),
                             error_y=dict(type='data', array=history["avg_rms"],
                                          visible=True)))

    # Current run points
    fig.add_trace(go.Scatter(x=hx,
                             y=history["curr"],
                             mode='markers',
                             name=f"Single buffer average = {last_curr:.2f} ± {last_curr_rms:.2f}\u00A0\u00A0\u00A0",
                             marker=dict(color='red', size=6),
                             error_y=dict(type='data', array=history["curr_rms"],
                                          visible=True)))

    fig.update_layout(
        title=dict(
            text="Average of ADC baseline noise over run",
            font=dict(size=20, family="Arial", color="black"),
            x=0.5,  # Center title (optional)
            xanchor="center"
        ),
        xaxis=dict(
            title="Time",
            title_font=dict(size=18),
            tickfont=dict(size=16)
        ),
        yaxis=dict(
            title="ADCs",
            title_font=dict(size=18),
            tickfont=dict(size=16)
        ),
        margin={"l": 40, "r": 10, "t": 100, "b": 40},
        height=400,
        legend=dict(
            orientation="h",        # Horizontal legend
            yanchor="bottom",       # Anchor to bottom of the legend box
            y=1.02,                 # Slightly above the top of the plot (adjust as needed)
            xanchor="center",       # Center it horizontally
            x=0.5,                  # Center on x-axis
            font=dict(size=18)      # Optional: adjust font size
        )
    )

    # --- New noise graph ---
    noise_data = latest["noise_samples"]
    noise_figure = go.Figure()
    
    noise_figure.add_trace(go.Scatter(
        x=x_times,
        y=noise_data,
        mode='markers',
        name='Noise samples'
    ))
    noise_figure.update_layout(
        title=dict(
            text=f"ADC Noise Samples: {noise_period} us from {dt.strftime('%H:%M:%S.')}{dt.microsecond // 10000:02d}",
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
            )
        )
    )
    
    return fig, noise_figure, history


# Show the current read status message above the graph
@callback(
    Output("status-message", "children"),
    Output("status-message", "style"),
    Input("baseline-latest", "data")
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
        return "Waiting for baseline data...", {"color": "red", "fontWeight": "bold"}
    else:
        return "Receiving baseline data...", {"color": "green", "fontWeight": "bold"}
