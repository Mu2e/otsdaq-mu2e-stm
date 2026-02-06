# Import required modules from Dash and standard libraries
import dash                                  # Dash core module
from dash import html, dcc, page_container  # HTML and component container tools
import psutil                                # Used to pin process to CPU core
import os                                    # For environment variable access
import xml.etree.ElementTree as ET           # For parsing XML configuration files
from utils.config import get_xml_node_value

# Create the Dash app and enable page system
app = dash.Dash(__name__,
                use_pages=True,
                suppress_callback_exceptions=True,
                prevent_initial_callbacks=True)

 # Attempt to extract the value of <stm><starting_core_id>
core_id = int(get_xml_node_value("starting_core_id"))

# Attempt to pin this process to the specified CPU core
p = psutil.Process()  # Get current process
try:
    p.cpu_affinity([core_id])  # Set CPU affinity
    print(f"Dash DQM pinned to CPU core {core_id}")
except Exception as e:
    print(f"Warning: Could not set CPU affinity: {e}")

# Top bar with title on the left and logos on the right
topbar = html.Div([
    # Left section: logo1 + DQM title + run info
    html.Div([
        html.Img(src="/assets/mu2e.png", className="logo-left"),
        html.Div([
            html.Div("Mu2e STM DQM", className="title"),
            html.Div("Run XXXX Event XXX", className="subtitle")
        ])
    ], className="topbar-left"),

    # Right section: absolutely positioned container for logos 2 and 3
    html.Div([
        html.Img(src="/assets/mcr.png", className="logo-right"),
        html.Img(src="/assets/ucl.png", className="logo-right")
    ], className="topbar-right")
], className="topbar")

# Navigation bar
navbar = html.Div([
    # dcc.Link("Overview", href="/overview", className="nav-link"),
    # dcc.Link("Station 2", href="/station2", className="nav-link"),
    # dcc.Link("Physics Plots", href="/physics", className="nav-link"),
    dcc.Link("Raw ADC Data", href="/raw-data", className="nav-link"),
    dcc.Link("ADC Baseline and Noise", href="/adc-baseline", className="nav-link"),
    # html.Button("Clear Histos", id="clear-button", className="nav-button")
], className="navbar")

# App layout
app.layout = html.Div([
    topbar,
    navbar,
    html.Div(className="page-content",
             children=page_container)
])

# Start the Dash server
if __name__ == '__main__':
    app.run(debug=True)
