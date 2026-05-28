import dash
from dash import html, dcc, callback, Output, Input, State
import time
from utils.log_reader import get_log_lines, format_mtime

# Register this script as a Dash page 
dash.register_page(__name__, path="/logs", name="Logs", layout=lambda: layout())

# Max lines to display
MAX_LINES = 500

COLORS = {
    "panel":       "#161b22",   # log panel background
    "border":      "#30363d",   # subtle border
    "text":        "#e6edf3",   # default log text
    "error":       "#f85149",   # error messages
    "warn":        "#d29922",   # WARNING lines
    "info":        "#3fb950",   # INFO lines
    "debug":       "#8b949e",   # DEBUG lines
    "critical":    "#ff7b72",   # CRITICAL / ERROR lines
    "scrollbar":   "#30363d",
}

FONT_MONO = "'JetBrains Mono', 'Fira Code', 'Cascadia Code', monospace"

def _line_style(line: str) -> dict:
    """Return a colour based on log level keywords in the line."""
    upper = line.upper()
    if any(k in upper for k in ("CRITICAL", "FATAL", "ERROR")):
        return {"color": COLORS["critical"]}
    if "WARNING" in upper or "WARN" in upper:
        return {"color": COLORS["warn"]}
    if "INFO" in upper:
        return {"color": COLORS["info"]}
    if "DEBUG" in upper:
        return {"color": COLORS["debug"]}
    return {"color": COLORS["text"]}

# Lazy-loaded layout function to reduce startup cost
def layout():
    return html.Div(

        children=[
            # Track URL so we can poll only when on this page
            dcc.Location(id="log-url", refresh=False),

            # Title and status message
            html.Div([
                # Filter input
                dcc.Input(
                    id="log-filter",
                    type="text",
                    placeholder="Filter lines…",
                    debounce=True,
                    style={
                        "backgroundColor": COLORS["panel"],
                        "border": f"1px solid {COLORS['border']}",
                        "borderRadius": "6px",
                        "color": COLORS["text"],
                        "fontSize": "13px",
                        "fontFamily": FONT_MONO,
                        "padding": "5px 10px",
                        "width": "260px",
                        "outline": "none",
                        "marginLeft": "20px",
                    },
                ),
                html.Div([
                    html.Span("Status: ", style={"color": "black", "fontWeight": "bold"}),
                    html.Span(id="log-status-message"),
                ], style={"marginLeft": "auto", "display": "flex", "alignItems": "center", "gap": "5px"})
            ], style={"display": "flex", "justifyContent": "space-between", "alignItems": "center", "padding": "0 5px", "marginBottom": "5px"}),
        
            html.Div(id="log-panel",
                    style={
                        "backgroundColor": COLORS["panel"],
                        "border": f"1px solid {COLORS['border']}",
                        "borderRadius": "8px",
                        "padding": "14px 16px",
                        "overflowY": "auto",
                        "maxHeight": "calc(100vh - 220px)",
                        "fontFamily": FONT_MONO,
                        "fontSize": "13px",
                        "lineHeight": "1.65",
                        "whiteSpace": "pre",          # preserve indentation
                        "wordBreak": "break-all",     # prevent horizontal overflow
                        # Styled scrollbar (WebKit)
                        "scrollbarWidth": "thin",
                        "scrollbarColor": f"{COLORS['scrollbar']} transparent",
                    },
            ),

            # Latest shared memory read for callbacks
            dcc.Store(id="log-status", data={}),

            # Timer to trigger polling shared memory
            # Poll slower to avoid callback overlap (650 ms read latency => 2000 ms interval)
            dcc.Interval(id="log-interval-component", interval=2000, n_intervals=0),
        ],
    )


# Read and parse the latest shared memory values every interval
@callback(
    Output("log-panel", "children"),
    Output("log-status", "data"),
    Input("log-url", "pathname"),
    Input("log-interval-component", "n_intervals"),
    Input("log-filter", "value"),
)
def update_log(pathname, n, filter_text):
    """
    Polls shared memory once per interval. Returns a parsed struct or wait status.
    This callback only runs when the user is on this specific page.
    """
    if pathname != "/logs":
        return "", dash.no_update

    data = get_log_lines(max_lines=MAX_LINES)

    # ── Error state ───────────────────────────────────────────────────────
    if data["error"] and not data["lines"]:
        panel_children = html.Span(data["error"], style={"color": COLORS["error"]})
        return panel_children, "Error"

    status = f"Reading log, last modified: {format_mtime(data['mtime'])}"

    # ── Apply filter ──────────────────────────────────────────────────────
    lines = data["lines"]
    if filter_text:
        needle = filter_text.lower()
        lines = [ln for ln in lines if needle in ln.lower()]
 
    if not lines:
        panel_children = html.Span(
            "(no lines match filter)" if filter_text else "(log file is empty)",
            style={"color": COLORS["debug"]},
        )
        return panel_children, status

    # ── Render lines ──────────────────────────────────────────────────────
    panel_children = [
        html.Div(line or "\u00a0", style=_line_style(line))   # \u00a0 = &nbsp; keeps empty rows
        for line in reversed(lines)
    ]

    return panel_children, status


# Show the current read status message above the graph
@callback(
    Output("log-status-message", "children"),
    Output("log-status-message", "style"),
    Input("log-status", "data")
)
def update_log_status(status):
    """
    Displays a color-coded status message above the graph.
    """
    
    if status is None:
        return "", {}
    
    if status == "Error":
        return "Error reading log...", {"color": "red", "fontWeight": "bold"}
    else:
        return status, {"color": "green", "fontWeight": "bold"}
