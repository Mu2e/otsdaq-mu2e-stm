# Import required modules
from dash import html, dcc, register_page
import numpy as np
import plotly.graph_objs as go

# Register this file as a Dash page with a specific path
#register_page(__name__, path="/example", name="Example")

# Simulated event numbers
events = np.arange(1, 351)

# Create module names
modules = [f"Module {i}" for i in range(8)]

# Generate dummy data for hits and event size
hits_total = np.random.normal(3500, 200, size=events.size)
hits_modules = [np.random.normal(500, 50, size=events.size) for _ in range(8)]
hit_time_bins = np.arange(500)
hit_time_counts = np.random.exponential(scale=1000, size=500).astype(int)
amc13_event_size = np.random.normal(5000, 300, size=events.size)

# Define the layout of this page
layout = html.Div([
    html.H1("Example Page"),  # Page title

    # First row of 3 summary graphs
    html.Div([
        html.Div([
            dcc.Graph(
                id="unpack-complete",
                figure=go.Figure(data=go.Scatter(x=events, y=np.ones_like(events), name="UnpackComplete"))
            )
        ], style={'width': '30%', 'display': 'inline-block'}),

        html.Div([
            dcc.Graph(
                id="num-unpack-errors",
                figure=go.Figure(data=go.Scatter(x=events, y=np.zeros_like(events), name="NumUnpackingErrors"))
            )
        ], style={'width': '30%', 'display': 'inline-block'}),

        html.Div([
            dcc.Graph(
                id="tracker-data",
                figure=go.Figure(data=go.Scatter(x=events, y=np.ones_like(events), name="TrackerData"))
            )
        ], style={'width': '30%', 'display': 'inline-block'})
    ]),

    # Number of Hits per Event for all modules
    html.Div([
        dcc.Graph(
            id="hits-per-event",
            figure={
                'data': [go.Scatter(x=events, y=hits_total, name='Total')] +
                [go.Scatter(x=events, y=hits_modules[i], name=modules[i]) for i in range(8)],
                'layout': go.Layout(title='Number of Hits Per Event')
            }
        )
    ]),

    # Histogram of TDC Hit Times
    html.Div([
        dcc.Graph(
            id="tdc-hit-time",
            figure={
                'data': [go.Bar(x=hit_time_bins, y=hit_time_counts)],
                'layout': go.Layout(title='TDC Hit Time (0-500us)', xaxis={'title': 'time bin'})
            }
        )
    ]),

    # TDC Buffer Sizes
    html.Div([
        dcc.Graph(
            id="tdc-buffer",
            figure={
                'data': [go.Scatter(x=np.arange(64), y=np.zeros(64))],
                'layout': go.Layout(title='TDC Buffer Size > 1000', xaxis={'title': 'TDC Number'})
            }
        )
    ]),

    # AMC13 Event Size per Event
    html.Div([
        dcc.Graph(
            id="amc13-size",
            figure={
                'data': [go.Scatter(x=events, y=amc13_event_size)],
                'layout': go.Layout(title='AMC13EventSize', xaxis={'title': 'event number'})
            }
        )
    ])
], className="page-content")                        # Load the style page layout
