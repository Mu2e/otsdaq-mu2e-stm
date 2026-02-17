import dash
from dash import html, dcc, callback, Output, Input, State
import plotly.graph_objs as go
from worker_manager import manager

# Register this script as a Dash page 
dash.register_page(__name__, path="/peak-data", name="Peak Data", layout=lambda: layout())

# Lazy-loaded layout function to reduce startup cost
def layout():
    return html.Div([

        # Track URL so we can poll only when on this page
        dcc.Location(id="peak-url", refresh=False),

        # Title and status message
        html.Div([
            html.H2("Peak data", style={"margin": 0}),
            html.Div([
                html.Span("Status: ", style={"color": "black", "fontWeight": "bold"}),
                html.Span(id="peak-status-message")
            ], style={"marginLeft": "auto", "display": "flex", "alignItems": "center", "gap": "5px"})
        ], style={"display": "flex", "justifyContent": "space-between", "alignItems": "center", "padding": "0 20px", "marginBottom": "10px"}),
        
        html.Div([
            # Main graph showing all peak data 
            html.Div(
                dcc.Graph(id="peak-all-hist",
                    responsive=True,
                    style={"height": "100%", "width": "100%"}
                    ),
                style={"width": "50%", "height": "600px", "margin": "auto",'display': 'inline-block'}
            ),

            # Graph showing window peak data
            html.Div(
                dcc.Graph(id="peak-window-hist",
                    responsive=True,
                    style={"height": "100%", "width": "100%"}
                    ),
                style={"width": "50%", "height": "600px", "margin": "auto",'display': 'inline-block'}
            ),
        ]),

        # Latest shared memory read for callbacks
        dcc.Store(id="peak-status", data={}),

        # Timer to trigger polling shared memory
        # Poll slower to avoid callback overlap (650 ms read latency => 2000 ms interval)
        dcc.Interval(id="peak-interval-component", interval=10000, n_intervals=0),
    ])


# Read and parse the latest shared memory values every interval
@callback(
    Output("peak-status", "data"),
    Output("peak-all-hist", "figure"),
    Output("peak-window-hist", "figure"),
    Input("peak-url", "pathname"),
    Input("peak-interval-component", "n_intervals"),
)
def update_peak(pathname, n):
    """
    Polls shared memory once per interval. Returns a parsed struct or wait status.
    This callback only runs when the user is on this specific page.
    """
    if pathname != "/peak-data":
        return "", dash.no_update, dash.no_update

    task_queue, result_queue = manager.get_queues("peaks")
    task_queue.put(True)
    
    status, all_histogram, window_histogram = result_queue.get()

    if all_histogram is None:
        return status, dash.no_update, dash.no_update

    return status, all_histogram, window_histogram


# Show the current read status message above the graph
@callback(
    Output("peak-status-message", "children"),
    Output("peak-status-message", "style"),
    Input("peak-status", "data")
)
def update_peak_status(status):
    """
    Displays a color-coded status message above the graph.
    """
    
    if status is None:
        return "", {}
    
    if status == "waiting for shared memory":
        return "No shared memory connection...", {"color": "red", "fontWeight": "bold"}
    elif status == "waiting for new data":
        return "Waiting for peak data...", {"color": "red", "fontWeight": "bold"}
    else:
        return "Receiving peak data...", {"color": "green", "fontWeight": "bold"}
