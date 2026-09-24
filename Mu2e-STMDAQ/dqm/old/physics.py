from dash import html, register_page

# Register Physics page
register_page(__name__, path="/physics", name="Physics Plots")

# Placeholder layout
layout = html.Div([html.H1("Physics Plots Page Placeholder")
                   ], className="page-content")                        # Load the style page layout
