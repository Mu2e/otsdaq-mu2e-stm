import psutil
import dash
from dash import html, dcc, callback, Output, Input, State
import dash_mantine_components as dmc
import plotly.graph_objs as go
from worker_manager import manager

# Register this script as a Dash page 
dash.register_page(__name__, path="/daq-info", name="DAQ info", layout=lambda: layout())

# Lazy-loaded layout function to reduce startup cost
def layout():
    return html.Div([

        # Track URL so we can poll only when on this page
        dcc.Location(id="cpu-url", refresh=False),

        # Title and status message
        html.Div([
            html.H2("DAQ status", style={"margin": 0}),
            html.Div([
                html.Span("Status: ", style={"color": "black", "fontWeight": "bold"}),
                html.Span(id="cpu-status-message")
            ], style={"marginLeft": "auto", "display": "flex", "alignItems": "center", "gap": "5px"})
        ], style={"display": "flex", "justifyContent": "space-between", "alignItems": "center", "padding": "0 20px", "marginBottom": "10px"}),
        
        # ADC temp and packets dropped plot
        html.Div([
            html.Div(
                dcc.Graph(id="temp-graph",
                    responsive=True,
                    style={"height": "100%", "width": "100%"}
                    ),
                style={"width": "50%", "height": "400px", "margin": "auto",'display': 'inline-block'}
            ),

            html.Div(
                dcc.Graph(id="packets-graph",
                    responsive=True,
                    style={"height": "100%", "width": "100%"}
                    ),
                style={"width": "50%", "height": "400px", "margin": "auto",'display': 'inline-block'}
            ),
        ]),
        
        # Grid of CPU speeds
        html.Div(
            id="speedo-container",
            #style={"width": "100%", "margin": "auto"}
            style={"display": "grid",
                    'gridTemplateColumns': 'repeat(auto-fill, minmax(300px, 1fr))',
                    "gridAutoRows": "250px",  # Sets the height
                    'gap': '20px'}
        ),

        # Latest shared memory read for callbacks
        dcc.Store(id="cpu-status", data={}),

        # Timer to trigger polling shared memory
        dcc.Interval(id="cpu-interval-component", interval=5000, n_intervals=0),
    ])


@callback(
    Output("speedo-container", "children"),
    Output("temp-graph", "figure"),
    Output("packets-graph", "figure"),
    Output("cpu-status", "data"),
    Output("notif-container","sendNotifications", allow_duplicate=True),
    Input("cpu-url", "pathname"),
    Input("cpu-interval-component", "n_intervals"),
    State("daq-history", "data"),
    prevent_initial_call = True
)
def update_cpu(pathname, n, history):
    if pathname != "/daq-info":
        return *([dash.no_update] * 3), None, dash.no_update

    cpu_task_queue, cpu_result_queue = manager.get_queues("cpu_speeds")

    cpu_task_queue.put(history)

    status, speedos, temp_plot, drop_plot, slow_alarm = cpu_result_queue.get()

    if not speedos:
        return *([dash.no_update] * 3), status, dash.no_update

    elif len(slow_alarm) == 0: 
        return speedos, temp_plot, drop_plot, status, dash.no_update

    else:
        return speedos, temp_plot, drop_plot, status, slow_alarm

# Show the current read status message above the graph
@callback(
    Output("cpu-status-message", "children"),
    Output("cpu-status-message", "style"),
    Input("cpu-status", "data")
)
def update_cpu_status(status):
    """
    Displays a color-coded status message above the graph.
    """
    
    if status is None:
        return "", {}
    
    if status == "waiting for shared memory":
        return "No shared memory connection...", {"color": "red", "fontWeight": "bold"}
    elif status == "waiting for new data":
        return "Waiting for CPU speed data...", {"color": "red", "fontWeight": "bold"}
    else:
        return "Receiving CPU speed data...", {"color": "green", "fontWeight": "bold"}
