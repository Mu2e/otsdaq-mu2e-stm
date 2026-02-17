import dash
from dash import html, dcc, callback, Output, Input, State
import dash_mantine_components as dmc
from worker_manager import manager
import plotly.graph_objs as go

# Register this script as a Dash page 
dash.register_page(__name__, path="/", name="Overview", layout=lambda: layout())

# Lazy-loaded layout function to reduce startup cost
def layout():
    return html.Div([

        # Track URL so we can poll only when on this page
        dcc.Location(id="overview-url", refresh=False),

        # Title and status message
        html.Div([
            html.H2("Overview", style={"margin": 0}),
            html.Div([
                html.Span("Status: ", style={"color": "black", "fontWeight": "bold"}),
                html.Span(id="status-message")
            ], style={"marginLeft": "auto", "display": "flex", "alignItems": "center", "gap": "5px"})
        ], style={"display": "flex", "justifyContent": "space-between", "alignItems": "center", "padding": "0 20px", "marginBottom": "10px"}),
        
        # Baseline and peak plot
        html.Div([
            html.Div(
                dcc.Graph(id="baseline-graph",
                    responsive=True,
                    style={"height": "100%", "width": "100%"}
                    ),
                style={"width": "50%", "height": "500px", "margin": "auto",'display': 'inline-block'}
            ),

            html.Div(
                dcc.Graph(id="peak-graph",
                    responsive=True,
                    style={"height": "100%", "width": "100%"}
                    ),
                style={"width": "50%", "height": "500px", "margin": "auto",'display': 'inline-block'}
            ),
        ]),

         html.Div(
            dcc.Graph(id="raw-graph",
                    responsive=True,
                    style={"height": "100%", "width": "100%"}
                    ),
            style={"width": "100%", "height": "400px", "margin": "auto",'display': 'inline-block'}
        ),

         html.Div(
            dcc.Graph(id="noise-plot",
                    responsive=True,
                    style={"height": "100%", "width": "100%"}
                    ),
            style={"width": "100%", "height": "400px", "margin": "auto",'display': 'inline-block'}
        ),

        # Latest shared memory read for callbacks
        dcc.Store(id="status", data={}),

        # Timer to trigger polling shared memory
        dcc.Interval(id="overview-interval-component", interval=10000, n_intervals=0),

    ])

@callback(
    Output("notif-container","sendNotifications", allow_duplicate=True),
    Output("latest-alarm-time", "data"),
    Input("alarm-interval", "n_intervals"),
    State("latest-alarm-time", "data"),
    prevent_initial_call=True
)
def check_alarms(n, latest_alarm):
    alarm_task_queue, alarm_result_queue = manager.get_queues("alarms")
    alarm_task_queue.put(latest_alarm)
    alarms_arr, time_dict = alarm_result_queue.get()
    
    if not alarms_arr or len(alarms_arr) == 0:
        return dash.no_update, dash.no_update
    else:
        return alarms_arr, time_dict

@callback(
    Output("status", "data"),
    Output("raw-graph", "figure"),
    Output("baseline-graph", "figure"),
    Output("noise-plot", "figure"),
    Output("peak-graph", "figure"),
    Output("baseline-history", "data", allow_duplicate=True),
    Input("overview-url", "pathname"),
    Input("overview-interval-component", "n_intervals"),
    State("baseline-history", "data"),
    prevent_initial_call=True
)
def update_overview(pathname, n, baseline_history):
    if pathname != "/":
        return [dash.no_update] * 6

    # Get all the worker queues
    raw_task_queue, raw_result_queue = manager.get_queues("raw_data")
    baseline_task_queue, baseline_result_queue = manager.get_queues("baseline")
    peak_task_queue, peak_result_queue = manager.get_queues("peaks")

    # Start the queue 
    raw_task_queue.put(True)
    baseline_task_queue.put(baseline_history)
    peak_task_queue.put(True)

    # Get the results
    raw_status, raw_fig = raw_result_queue.get()
    baseline_status, time_fig, baseline_hist, noise_fig, noise_fft, history = baseline_result_queue.get()
    peak_status, peak_all, peak_window = peak_result_queue.get()

    return raw_status, raw_fig, baseline_hist, noise_fig, peak_all, history


# Show the current read status message above the graph
@callback(
    Output("status-message", "children"),
    Output("status-message", "style"),
    Input("status", "data"),
)
def update_overview_status(status):
    """
    Displays a color-coded status message above the graph.
    """
    
    if status is None:
        return "", {}
    
    if status == "waiting for shared memory":
        return "No shared memory connection...", {"color": "red", "fontWeight": "bold"}
    elif status == "waiting for new data":
        return "Waiting for data...", {"color": "red", "fontWeight": "bold"}
    else:
        return "Receiving data...", {"color": "green", "fontWeight": "bold"}
