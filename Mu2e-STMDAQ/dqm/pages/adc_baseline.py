import datetime
import dash
from dash import html, dcc, callback, Output, Input, State
#import plotly.graph_objs as go
from worker_manager import manager

# Register this script as a Dash page at "/adc-baseline"
# NOTE: layout is now provided as a callable for lazy loading
dash.register_page(__name__, path="/adc-baseline", name="ADC Baseline and Noise", layout=lambda: layout())

# Lazy-loaded layout function to reduce startup cost
def layout():
    return html.Div([

        # Track URL so we can poll only when on this page
        dcc.Location(id="baseline-url", refresh=False),

        # Title and status message
        html.Div([
            html.Div([
                html.Span("Status: ", style={"color": "black", "fontWeight": "bold"}),
                html.Span(id="baseline-status-message")
            ], style={"marginLeft": "auto", "display": "flex", "alignItems": "center", "gap": "5px"})
        ], style={"display": "flex", "justifyContent": "space-between", "alignItems": "center", "padding": "0 20px", "marginBottom": "10px"}),
        
        # html.H2("ADC Baseline and Noise", style={"paddingLeft": "20px"}),

        # Main graph showing baseline evolution
        html.Div([
            html.Div(
                dcc.Graph(id="adc-baseline-graph",
                    responsive=True,
                    style={"height": "100%", "width": "100%"}
                    ),
                style={"width": "50%", "height": "500px", "margin": "auto",'display': 'inline-block'}
            ),

            html.Div(
                dcc.Graph(id="baseline-hist",
                    responsive=True,
                    style={"height": "100%", "width": "100%"}
                    ),
                style={"width": "50%", "height": "500px", "margin": "auto",'display': 'inline-block'}
            ),
        ]),
        
        # Noise graph
        dcc.Graph(id='noise-graph'),

        # Noise FFT
        dcc.Graph(id='noise-fft'),
        
        # Latest shared memory read for callbacks
        dcc.Store(id="baseline-status", data={}),

        # Timer to trigger polling shared memory
        # Poll slower to avoid callback overlap (650 ms read latency => 2000 ms interval)
        dcc.Interval(id="baseline-interval-component", interval=5000, n_intervals=0),
    ])


@callback(
    Output("baseline-status", "data"),
    Output("adc-baseline-graph", "figure"),
    Output("baseline-hist", "figure"),
    Output('noise-graph', "figure"),
    Output('noise-fft', "figure"),
    Output("baseline-history", "data", allow_duplicate=True),
    Input("baseline-url", "pathname"),
    Input("baseline-interval-component", "n_intervals"),
    State("baseline-history", "data"),
    prevent_initial_call=True
)
def update_baseline(pathname, n, history):
    """
    Poll worker once per interval. Returns a plot or wait status.
    This callback only runs when the user is on this specific page.
    """
    if pathname != "/adc-baseline":
        return "", *([dash.no_update] * 4), history

    baseline_task_queue, baseline_result_queue = manager.get_queues("baseline")
    poll_time = datetime.datetime.now()
    baseline_task_queue.put(history)

    status, baseline_plot, baseline_hist, noise_plot, fft_plot, history = baseline_result_queue.get()

    return status, baseline_plot, baseline_hist, noise_plot, fft_plot, history


# Show the current read status message above the graph
@callback(
    Output("baseline-status-message", "children"),
    Output("baseline-status-message", "style"),
    Input("baseline-status", "data")
)
def update_baseline_status(status):
    """
    Displays a color-coded status message above the graph.
    """
    
    if status is None:
        return "", {}
    
    if status == "waiting for shared memory":
        return "No shared memory connection...", {"color": "red", "fontWeight": "bold"}
    elif status == "waiting for new data":
        return "Waiting for baseline data...", {"color": "red", "fontWeight": "bold"}
    else:
        return "Receiving baseline data...", {"color": "green", "fontWeight": "bold"}
