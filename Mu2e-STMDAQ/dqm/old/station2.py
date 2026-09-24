from dash import html, register_page

# Register Station 2 page
register_page(__name__, path="/station2", name="Station 2")

# Placeholder layout
layout = html.Div([html.H1("Station 2 Page Placeholder")
                   ], className="page-content")                        # Load the style page layout
