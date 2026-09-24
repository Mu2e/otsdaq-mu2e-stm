import psutil
import dash
from dash import html, dcc, callback, Output, Input, State
import plotly.graph_objs as go
from worker_manager import manager

# Register this script as a Dash page 
dash.register_page(__name__, path="/raw-data", name="Raw Data", layout=lambda: layout())

# Lazy-loaded layout function to reduce startup cost
def layout():
    return html.Div([

        # Track URL so we can poll only when on this page
        dcc.Location(id="raw-url", refresh=False),

        # Title and status message
        html.Div([
            html.Div([
                html.Span("Status: ", style={"color": "black", "fontWeight": "bold"}),
                html.Span(id="raw-status-message")
            ], style={"marginLeft": "auto", "display": "flex", "alignItems": "center", "gap": "5px"})
        ], style={"display": "flex", "justifyContent": "space-between", "alignItems": "center", "padding": "0 5px", "marginBottom": "10px"}),
        
        # Main graph showing raw data evolution
        html.Div(
            dcc.Graph(id="raw-data-graph"),
            style={"width": "100%", "margin": "auto"}
        ),
        
        # Persistent store holding time history 
        dcc.Store(id="time-history", data={
            "x": [],
            "last_timestamp": 0
        }),

        # Latest shared memory read for callbacks
        dcc.Store(id="raw-status", data={}),

        # Timer to trigger polling shared memory
        # Poll slower to avoid callback overlap (650 ms read latency => 2000 ms interval)
        dcc.Interval(id="raw-interval-component", interval=1000, n_intervals=0),
    ])


@callback(
    Output("raw-data-graph", "figure"),
    Output("raw-status", "data"),
    Input("raw-url", "pathname"),
    Input("raw-interval-component", "n_intervals"),
)
def update_raw(pathname, n):
    if pathname != "/raw-data":
        return dash.no_update

    raw_task_queue, raw_result_queue = manager.get_queues("raw_data")

    raw_task_queue.put(True)

    status, plot = raw_result_queue.get()

    return plot, status


# Show the current read status message above the graph
@callback(
    Output("raw-status-message", "children"),
    Output("raw-status-message", "style"),
    Input("raw-status", "data")
)
def update_raw_status(status):
    """
    Displays a color-coded status message above the graph.
    """
    
    if status is None:
        return "", {}
    
    if status == "waiting for shared memory":
        return "No shared memory connection...", {"color": "red", "fontWeight": "bold"}
    elif status == "waiting for new data":
        return "Waiting for raw data...", {"color": "red", "fontWeight": "bold"}
    else:
        return "Receiving raw data...", {"color": "green", "fontWeight": "bold"}
