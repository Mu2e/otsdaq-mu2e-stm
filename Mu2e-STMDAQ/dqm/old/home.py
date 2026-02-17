from dash import register_page
import pages.overview as overview  # Import overview layout

# Register this file as the root page ("/") but reuse overview layout
register_page(__name__, path="/", name="Home")

# Share the same layout as the Overview page
layout = overview.layout
