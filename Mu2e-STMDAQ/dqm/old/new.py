import dash
from dash import dcc, html, Input, Output, callback, page_container, register_page
import plotly.graph_objs as go
import numpy as np
import datetime
import os
import mmap
import struct


# Initialize Dash app with pages
app = dash.Dash(__name__, use_pages=True, pages_folder="")

# Generate example data
events = np.arange(1, 351)
modules = [f"Module {i}" for i in range(8)]

hits_total = np.random.normal(3500, 200, size=events.size)
hits_modules = [np.random.normal(500, 50, size=events.size) for _ in range(8)]

hit_time_bins = np.arange(500)
hit_time_counts = np.random.exponential(scale=1000, size=500).astype(int)

amc13_event_size = np.random.normal(5000, 300, size=events.size)

# Register pages
register_page("overview", path="/overview", layout=html.Div([
    html.H1("Overview Page"),
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

    html.Div([
        dcc.Graph(
            id="tdc-hit-time",
            figure={
                'data': [go.Bar(x=hit_time_bins, y=hit_time_counts)],
                'layout': go.Layout(title='TDC Hit Time (0-500us)', xaxis={'title': 'time bin'})
            }
        )
    ]),

    html.Div([
        dcc.Graph(
            id="tdc-buffer",
            figure={
                'data': [go.Scatter(x=np.arange(64), y=np.zeros(64))],
                'layout': go.Layout(title='TDC Buffer Size > 1000', xaxis={'title': 'TDC Number'})
            }
        )
    ]),

    html.Div([
        dcc.Graph(
            id="amc13-size",
            figure={
                'data': [go.Scatter(x=events, y=amc13_event_size)],
                'layout': go.Layout(title='AMC13EventSize', xaxis={'title': 'event number'})
            }
        )
    ])
]))

register_page("adc_baseline", path="/adc-baseline", layout=html.Div([
    html.H1("ADC Baseline and Noise"),

    dcc.Graph(id="adc-baseline-graph"),

    dcc.Interval(
        id='interval-component',
        interval=5*1000,  # milliseconds, update every 5 seconds (configurable)
        n_intervals=0
    )
]))

register_page("station2", path="/station2", layout=html.Div([html.H1("Station 2 Page Placeholder")]))
register_page("physics", path="/physics", layout=html.Div([html.H1("Physics Plots Page Placeholder")]))

# Navigation bar
navbar = html.Nav([
    html.Div("Mu2e STM DQM", style={"fontWeight": "bold", "fontSize": "24px", "padding": "10px", "color": "white"}),
    html.Div([
        dcc.Link("Overview", href="/overview", style={"padding": "10px", "color": "white", "textDecoration": "none"}),
        dcc.Link("Station 2", href="/station2", style={"padding": "10px", "color": "white", "textDecoration": "none"}),
        dcc.Link("Physics Plots", href="/physics", style={"padding": "10px", "color": "white", "textDecoration": "none"}),
        dcc.Link("ADC Baseline and Noise", href="/adc-baseline", style={"padding": "10px", "color": "white", "textDecoration": "none"}),
        html.Button("Clear Histos", id="clear-button", style={"marginLeft": "20px", "backgroundColor": "#DC3545", "color": "white", "padding": "10px", "border": "none", "borderRadius": "5px"})
    ], style={"display": "flex", "alignItems": "center"})
], style={"display": "flex", "justifyContent": "space-between", "alignItems": "center", "backgroundColor": "#333", "padding": "10px"})

app.layout = html.Div([
    navbar,
    html.Div("Run 1912 Event 352", style={"padding": "10px"}),
    page_container
])

@app.callback(
    Output("adc-baseline-graph", "figure"),
    Input("interval-component", "n_intervals")
)
def update_adc_plot(n):
    try:
        shm_name = "/dqm_shared_memory"  # must match your C++ config!
        shm_size = 4096  # bytes, must match config

        with open(f"/dev/shm{shm_name}", "rb") as f:
            mm = mmap.mmap(f.fileno(), shm_size, access=mmap.ACCESS_READ)

            # Extract relevant data — this assumes you store it at fixed offsets.
            # Adjust the unpacking depending on how the struct is laid out.
            # Example assumes doubles for means/rms and a list of N instantaneous (time, mean, rms)

            offset = 0
            fmt = "4d"  # prev_mean, prev_rms, curr_mean, curr_rms
            prev_mean, prev_rms, curr_mean, curr_rms = struct.unpack_from(fmt, mm, offset)
            offset += struct.calcsize(fmt)

            # Example: 10 points max
            inst_count = struct.unpack_from("i", mm, offset)[0]
            offset += 4

            inst_times = struct.unpack_from(f"{inst_count}Q", mm, offset)
            offset += 8 * inst_count
            inst_means = struct.unpack_from(f"{inst_count}d", mm, offset)
            offset += 8 * inst_count
            inst_rmss = struct.unpack_from(f"{inst_count}d", mm, offset)
            offset += 8 * inst_count

            # Convert times to datetime
            time_vals = [datetime.datetime.fromtimestamp(t / 1e9) for t in inst_times]

            # Plotting
            fig = go.Figure()

            # 1. Previous run (band)
            fig.add_trace(go.Scatter(
                x=[time_vals[0], time_vals[-1]],
                y=[prev_mean, prev_mean],
                name="Previous Run Mean",
                mode="lines",
                line=dict(color="blue"),
                fill="tonexty",
                fillcolor="rgba(0, 0, 255, 0.2)",
            ))
            fig.add_trace(go.Scatter(
                x=[time_vals[0], time_vals[-1]],
                y=[prev_mean + prev_rms, prev_mean + prev_rms],
                mode="lines",
                line=dict(width=0),
                showlegend=False
            ))
            fig.add_trace(go.Scatter(
                x=[time_vals[0], time_vals[-1]],
                y=[prev_mean - prev_rms, prev_mean - prev_rms],
                mode="lines",
                line=dict(width=0),
                fill="tonexty",
                fillcolor="rgba(0, 0, 255, 0.2)",
                showlegend=False
            ))

            # 2. Current run (band)
            fig.add_trace(go.Scatter(
                x=[time_vals[0], time_vals[-1]],
                y=[curr_mean, curr_mean],
                name="Current Run Mean",
                mode="lines",
                line=dict(color="green", dash="dash"),
                fill="tonexty",
                fillcolor="rgba(0, 255, 0, 0.15)"
            ))
            fig.add_trace(go.Scatter(
                x=[time_vals[0], time_vals[-1]],
                y=[curr_mean + curr_rms, curr_mean + curr_rms],
                mode="lines",
                line=dict(width=0),
                showlegend=False
            ))
            fig.add_trace(go.Scatter(
                x=[time_vals[0], time_vals[-1]],
                y=[curr_mean - curr_rms, curr_mean - curr_rms],
                mode="lines",
                line=dict(width=0),
                fill="tonexty",
                fillcolor="rgba(0, 255, 0, 0.15)",
                showlegend=False
            ))

            # 3. Instantaneous points with error bars
            fig.add_trace(go.Scatter(
                x=time_vals,
                y=inst_means,
                error_y=dict(type="data", array=inst_rmss, visible=True),
                mode="markers+lines",
                name="Instantaneous",
                marker=dict(color="red")
            ))

            fig.update_layout(
                title="ADC Baseline and Noise",
                xaxis_title="Time",
                yaxis_title="ADC Value",
                legend_title="Data Type",
                template="plotly_white"
            )

            return fig

    except Exception as e:
        return go.Figure().update_layout(title=f"Error: {e}")


if __name__ == '__main__':
    app.run(debug=True)
