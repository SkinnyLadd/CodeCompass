# just do pip install streamlit and pandas, i ain't making a requirements.txt for just two libs
# waise ngl with how much time it took me to write that line I could've made the file

# EHHHHHHHHHHHH

import streamlit as st
import subprocess
import pandas as pd
import atexit
import time
import os
import platform

# =========================================================
# CONFIG & THEME (Red/Black Dark Mode)
# =========================================================
st.set_page_config(
    page_title="CodeCompass",
    page_icon="🧭",
    layout="wide",
    initial_sidebar_state="expanded"
)

# Custom CSS for Dark/Red Theme
st.markdown("""
    <style>
    /* Main Background */
    .stApp {
        background-color: #0e0e0e;
        color: #e0e0e0;
    }
    
    /* Sidebar Background */
    section[data-testid="stSidebar"] {
        background-color: #1a1a1a;
        border-right: 1px solid #333;
    }

    /* Red Accents for Headers */
    h1, h2, h3 {
        color: #ff4b4b !important;
        font-family: 'Helvetica', sans-serif;
    }

    /* Input Fields */
    .stTextInput input {
        background-color: #262626;
        color: white;
        border: 1px solid #444;
    }
    
    /* Buttons (Red Style) */
    div.stButton > button {
        background-color: #d90429;
        color: white;
        border: none;
        border-radius: 4px;
        font-weight: bold;
    }
    div.stButton > button:hover {
        background-color: #ef233c;
        border-color: #ef233c;
        color: white;
    }

    /* Table Styling */
    div[data-testid="stDataFrame"] {
        background-color: #1a1a1a;
    }
    </style>
""", unsafe_allow_html=True)

# =========================================================
# PROCESS MANAGEMENT (Persistent C++ Connection)
# =========================================================
def start_cpp_engine():
    # 1. Detect OS and Executable Name
    system_os = platform.system()
    exe_name = "codecompass_engine.exe" if system_os == "Windows" else "codecompass_engine"
    run_cmd = f"./{exe_name}" if system_os != "Windows" else exe_name

    # 2. Automate Compilation (if missing)
    if not os.path.exists(exe_name):
        placeholder = st.empty()
        placeholder.info("⚙️ Compiling C++ Engine... (One-time setup)")

        # Compile Command
        compile_cmd = [
            "g++",
            "src/main.cpp",
            "src/CSVParser.cpp",
            "-o", "codecompass_engine"
        ]

        try:
            result = subprocess.run(compile_cmd, capture_output=True, text=True)
            if result.returncode != 0:
                placeholder.error("❌ Compilation Failed!")
                st.code(result.stderr)
                return None
            placeholder.success("✅ Compilation Successful!")
            time.sleep(1) # Brief pause to show success
            placeholder.empty()
        except FileNotFoundError:
            placeholder.error("❌ g++ compiler not found. Please install MinGW (Windows) or G++ (Linux/Mac).")
            return None

    # 3. Launch Process
    try:
        process = subprocess.Popen(
            [run_cmd],  # Command to run C++
            stdin=subprocess.PIPE,
            stdout=subprocess.PIPE,
            text=True,
            bufsize=0  # Unbuffered I/O
        )
        return process
    except FileNotFoundError:
        st.error(f"❌ C++ Engine not found at {exe_name}!")
        return None
    except Exception as e:
        st.error(f"❌ Error starting engine: {e}")
        return None

# Initialize Session State
if 'process' not in st.session_state:
    st.session_state.process = start_cpp_engine()
    st.session_state.history = []  # Stack for Navigation History
    st.session_state.last_query = ""

# Cleanup on exit
def cleanup():
    if 'process' in st.session_state and st.session_state.process:
        st.session_state.process.terminate()
atexit.register(cleanup)

# =========================================================
# HELPER FUNCTIONS
# =========================================================
def query_cpp(search_term):
    if not st.session_state.process:
        return []

    # 1. Send query to C++ stdin
    try:
        st.session_state.process.stdin.write(search_term + "\n")
        st.session_state.process.stdin.flush()
    except BrokenPipeError:
        st.error("C++ Engine crashed. Please restart.")
        return []

    # 2. Read results from C++ stdout until 'END_OF_RESULTS'
    results = []
    while True:
        line = st.session_state.process.stdout.readline().strip()
        if line == "END_OF_RESULTS":
            break
        if line == "NO_RESULTS":
            return []
        if line.startswith("[CPP]"): # Skip debug logs
            continue
        if "|" in line:
            parts = line.split("|")
            # Format: ID|Title|URL|Difficulty|Rating
            if len(parts) >= 5:
                results.append({
                    "ID": parts[0],
                    "Title": parts[1],
                    "Resource": parts[2],
                    "Difficulty": int(parts[3]),
                    "Rating": f"⭐ {parts[4]}"
                })
    return results

# =========================================================
# UI LAYOUT
# =========================================================

# --- SIDEBAR: NAVIGATION HISTORY (Stack) ---
with st.sidebar:
    st.title("📜 History Stack")
    st.markdown("Your navigation path:")

    if st.session_state.history:
        # Display stack visually (Last In, First Out)
        for i, item in enumerate(reversed(st.session_state.history)):
            st.code(f"{len(st.session_state.history) - i}. {item}")

        if st.button("Pop Stack (Back)"):
            st.session_state.history.pop()
            st.rerun()
    else:
        st.info("Stack is empty.")

    st.markdown("---")
    st.markdown("**System Status**")
    if st.session_state.process:
        st.success("🟢 C++ Engine Running")
    else:
        st.error("🔴 C++ Engine Offline")

# --- MAIN AREA ---
st.title("CodeCompass 🧭")
st.markdown("### Cache-Efficient DSA Resource Recommender")

# 1. SEARCH BAR (Trie / Cache Interaction)
col1, col2 = st.columns([3, 1])
with col1:
    search_query = st.text_input("Search for a Topic (e.g., 'Array', 'Graph')", value=st.session_state.last_query)
with col2:
    sort_option = st.selectbox("Sort Results By", ["Difficulty (QuickSort)", "Title (MergeSort)"])

if st.button("🔍 Search Resources") or search_query:
    if search_query:
        # Update History Stack
        if not st.session_state.history or st.session_state.history[-1] != search_query:
            st.session_state.history.append(search_query)

        # Query C++
        start_time = time.time()
        data = query_cpp(search_query)
        end_time = time.time()

        # 2. METRICS DISPLAY
        st.markdown("---")
        m1, m2, m3 = st.columns(3)
        m1.metric("Time Taken", f"{(end_time - start_time)*1000:.2f} ms")
        m2.metric("Source", "C++ Backend (Mock)") # Later: Cache vs DB
        m3.metric("Results Found", len(data))

        # 3. RESULTS TABLE
        if data:
            df = pd.DataFrame(data)

            # Simulate Sorting logic visually if C++ isn't doing it yet
            if "Difficulty" in sort_option:
                df = df.sort_values("Difficulty")
            else:
                df = df.sort_values("Title")

            # Display interactive table with clickable links
            st.dataframe(
                df,
                column_config={
                    "Resource": st.column_config.LinkColumn("Link"),
                    "Difficulty": st.column_config.ProgressColumn(
                        "Difficulty", min_value=0, max_value=100, format="%d/100"
                    )
                },
                use_container_width=True,
                hide_index=True
            )

            # 4. RECOMMENDATIONS (Graph / Dijkstra Placeholder)
            st.markdown("### 🔗 Recommended Learning Path")
            st.info(f"To master **{search_query}**, our Knowledge Graph suggests learning these first:")

            # This is where Dijkstra output will go later
            # Mocking a path for now
            path_cols = st.columns(4)
            path_cols[0].button("1. C++ Basics", disabled=True)
            path_cols[1].markdown("➝")
            path_cols[2].button("2. Pointers", disabled=True)
            path_cols[3].markdown(f"➝ **{search_query}**")

        else:
            st.warning("No resources found in the C++ database.")