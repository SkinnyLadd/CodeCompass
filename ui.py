import streamlit as st
import subprocess
import os

st.set_page_config(page_title="CodeCompass", layout="wide")

# 1. FIND BACKEND
def get_exe():
    name = "codecompass_engine.exe" if os.name == 'nt' else "codecompass_engine"
    paths = [name, os.path.join("cmake-build-debug", name), os.path.join("..", "cmake-build-debug", name)]
    for p in paths:
        if os.path.exists(p): return os.path.abspath(p)
    return None

EXE_PATH = get_exe()

# 2. RUN COMMAND
def run_cpp(cmd):
    if not EXE_PATH: return []
    try:
        # Hide console on Windows
        si = None
        if os.name == 'nt':
            si = subprocess.STARTUPINFO()
            si.dwFlags |= subprocess.STARTF_USESHOWWINDOW

        res = subprocess.run([EXE_PATH, cmd], capture_output=True, text=True, startupinfo=si)
        return res.stdout.strip().splitlines()
    except: return []

# 3. CSV PARSER (Simple Split)
def parse_csv_lines(lines):
    if not lines or len(lines) < 2: return [] # Need header + data

    # Header: ID,Title,URL,Topic,Difficulty,Rating
    data = []
    for line in lines[1:]: # Skip header
        parts = line.split(',')
        if len(parts) >= 6:
            data.append({
                "ID": parts[0],
                "Title": parts[1],
                "URL": parts[2],
                "Topic": parts[3],
                "Difficulty": parts[4],
                "Rating": parts[5]
            })
    return data

# --- UI START ---
st.title("🧭 CodeCompass")

if not EXE_PATH:
    st.error("Backend not found. Please build the C++ project.")
    st.stop()

# --- TABS ---
tab1, tab2 = st.tabs(["📘 Full Library", "🎓 Curriculum Planner"])

# =========================================================
# TAB 1: PRINTS EVERYTHING ABOUT RESOURCES
# =========================================================
with tab1:
    st.header("All Resources")

    if st.button("Load Library", key="btn_load"):
        # COMMAND: LIST
        raw_lines = run_cpp("LIST")

        # Parse and Display
        rows = parse_csv_lines(raw_lines)
        if rows:
            st.success(f"Loaded {len(rows)} items.")
            st.table(rows) # Prints everything in a clean table
        else:
            st.warning("Database empty.")

# =========================================================
# TAB 2: TITLE SELECTION -> PLAN
# =========================================================
with tab2:
    st.header("Generate Plan")

    # 1. Get ONLY Titles for the dropdown
    all_titles = run_cpp("TITLES")

    if not all_titles:
        st.warning("No titles found.")
    else:
        # 2. User selects a Title
        target_title = st.selectbox("I want to learn:", all_titles)

        # 3. User clicks Generate
        if st.button("Get Plan for Title"):
            # COMMAND: PLAN|Title
            cmd = f"PLAN|{target_title}"
            raw_plan = run_cpp(cmd)

            plan_rows = parse_csv_lines(raw_plan)

            if plan_rows:
                st.success(f"Curriculum for: {target_title}")
                # Print the full details of the plan
                st.table(plan_rows)
            else:
                st.error("No plan found.")