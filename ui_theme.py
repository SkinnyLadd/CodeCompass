"""
Theme Configuration Module
Handles light/dark mode styling for the Streamlit UI
"""

import streamlit as st


def initialize_theme():
    """Initialize theme in session state"""
    if 'dark_mode' not in st.session_state:
        st.session_state.dark_mode = True


def apply_theme():
    """Apply theme styles based on dark_mode state"""
    if st.session_state.dark_mode:
        st.markdown("""
            <style>
            /* Dark Mode */
            .stApp { background-color: #0e0e0e; color: #e0e0e0; }
            .main > div { background-color: #0e0e0e; }
            div[data-testid="stSidebar"] { background-color: #1a1a1a; }
            div.stButton > button { background-color: #d90429; color: white; border: none; font-weight: bold; }
            div.stButton > button:hover { background-color: #ef233c; color: white; }
            a { color: #ff4b4b !important; text-decoration: none; }
            a:hover { text-decoration: underline; }
            div[data-testid="stMetricValue"] { color: #e0e0e0; }
            div[data-testid="stMetricLabel"] { color: #b0b0b0; }
            code { background-color: #2a2a2a; color: #e0e0e0; }
            pre { background-color: #2a2a2a; color: #e0e0e0; }
            .stMarkdown { color: #e0e0e0; }
            div[data-baseweb="base-input"] { background-color: #2a2a2a; }
            </style>
        """, unsafe_allow_html=True)
    else:
        st.markdown("""
            <style>
            /* Light Mode - Improved visibility */
            .stApp { background-color: #f8f9fa; color: #212529; }
            .main > div { background-color: #f8f9fa; }
            div[data-testid="stSidebar"] { background-color: #ffffff; border-right: 1px solid #dee2e6; }
            div.stButton > button { background-color: #d90429; color: white; border: none; font-weight: bold; box-shadow: 0 2px 4px rgba(0,0,0,0.1); }
            div.stButton > button:hover { background-color: #ef233c; color: white; box-shadow: 0 4px 8px rgba(0,0,0,0.15); }
            a { color: #c8102e !important; text-decoration: none; font-weight: 500; }
            a:hover { text-decoration: underline; color: #a00d25 !important; }
            div[data-testid="stMetricValue"] { color: #212529; font-weight: 600; }
            div[data-testid="stMetricLabel"] { color: #6c757d; font-weight: 500; }
            code { background-color: #e9ecef; color: #212529; padding: 2px 6px; border-radius: 3px; }
            pre { background-color: #e9ecef; color: #212529; border: 1px solid #dee2e6; padding: 12px; border-radius: 5px; }
            .stMarkdown { color: #212529; }
            div[data-baseweb="base-input"] { background-color: #ffffff; }
            
            /* DataFrames and Tables - Improved styling */
            .stDataFrame { background-color: #ffffff !important; border: 1px solid #dee2e6 !important; border-radius: 8px !important; overflow: hidden !important; }
            .stDataFrame > div { background-color: #ffffff !important; }
            .stDataFrame table { background-color: #ffffff !important; color: #212529 !important; border-collapse: collapse !important; }
            .stDataFrame th { background-color: #e9ecef !important; color: #212529 !important; font-weight: 700 !important; padding: 12px !important; border-bottom: 2px solid #dee2e6 !important; }
            .stDataFrame td { background-color: #ffffff !important; color: #212529 !important; padding: 10px 12px !important; border-bottom: 1px solid #f0f0f0 !important; }
            .stDataFrame tr:nth-child(even) td { background-color: #f8f9fa !important; }
            .stDataFrame tr:hover td { background-color: #e9ecef !important; }
            
            /* Streamlit tables */
            table { background-color: #ffffff !important; color: #212529 !important; border-collapse: collapse !important; width: 100% !important; }
            table th { background-color: #e9ecef !important; color: #212529 !important; font-weight: 700 !important; padding: 12px !important; border: 1px solid #dee2e6 !important; }
            table td { background-color: #ffffff !important; color: #212529 !important; padding: 10px 12px !important; border: 1px solid #dee2e6 !important; }
            table tr:nth-child(even) td { background-color: #f8f9fa !important; }
            table tr:hover td { background-color: #e9ecef !important; }
            
            /* Dataframe container */
            div[data-testid="stDataFrameContainer"] { border: 1px solid #dee2e6 !important; border-radius: 8px !important; }
            
            /* Text inputs and selectboxes */
            .stSelectbox label, .stTextInput label, .stSlider label { color: #212529 !important; font-weight: 500; }
            div[data-baseweb="select"] { background-color: #ffffff !important; }
            div[data-baseweb="select"] input { color: #212529 !important; }
            
            /* Info/Warning/Success boxes */
            .stAlert { background-color: #ffffff !important; border: 1px solid #dee2e6 !important; }
            div[data-baseweb="notification"] { background-color: #ffffff !important; color: #212529 !important; }
            
            /* Captions and help text */
            .stCaption { color: #6c757d !important; }
            
            /* Tabs */
            .stTabs [data-baseweb="tab"] { background-color: #ffffff !important; color: #212529 !important; }
            .stTabs [data-baseweb="tab"][aria-selected="true"] { background-color: #f8f9fa !important; color: #d90429 !important; font-weight: 600; }
            
            /* Dividers */
            hr { border-color: #dee2e6 !important; }
            </style>
        """, unsafe_allow_html=True)


def render_theme_toggle():
    """Render theme toggle in sidebar"""
    st.header("⚙ Settings")
    dark_mode = st.toggle("🌙 Dark Mode", value=st.session_state.dark_mode, key="theme_toggle")
    if dark_mode != st.session_state.dark_mode:
        st.session_state.dark_mode = dark_mode
        st.rerun()

