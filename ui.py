import streamlit as st
import subprocess
import os
import pandas as pd
import platform

st.set_page_config(page_title="CodeCompass", layout="wide", page_icon="🧭")

# =========================================================
# THEME CONFIGURATION
# =========================================================
st.markdown("""
    <style>
    .stApp { background-color: #0e0e0e; color: #e0e0e0; }
    div.stButton > button { background-color: #d90429; color: white; border: none; font-weight: bold; }
    div.stButton > button:hover { background-color: #ef233c; color: white; }
    </style>
""", unsafe_allow_html=True)

# =========================================================
# BACKEND CONNECTION
# =========================================================
def get_exe():
    name = "codecompass_engine.exe" if os.name == 'nt' else "codecompass_engine"
    # Look in current dir and build folders
    paths = [name, os.path.join("cmake-build-debug", name), os.path.join("..", "cmake-build-debug", name)]
    for p in paths:
        if os.path.exists(p): return os.path.abspath(p)
    return None

def compile_engine():
    """Compiles the C++ engine with static linking to avoid DLL errors."""
    st.toast("⚙️ Compiling C++ Engine...", icon="🔨")

    output_name = "codecompass_engine.exe" if os.name == 'nt' else "codecompass_engine"

    # Compilation command with -static to prevent 0xC0000139 errors
    cmd = [
        "g++",
        "src/main.cpp",
        "src/CSVParser.cpp",
        "src/Engine.cpp",
        "-I", "include",
        "-static",  # <--- CRITICAL FIX for Windows
        "-o", output_name
    ]

    try:
        res = subprocess.run(cmd, capture_output=True, text=True)
        if res.returncode == 0:
            st.toast("Compilation Successful!", icon="✅")
            return True
        else:
            st.error(f"Compilation Failed:\n{res.stderr}")
            return False
    except FileNotFoundError:
        st.error("g++ not found. Please install MinGW/GCC.")
        return False

EXE_PATH = get_exe()

def run_cpp(cmd):
    # Try to find exe, if not, try to compile
    local_exe = EXE_PATH
    if not local_exe:
        if compile_engine():
            local_exe = os.path.abspath("codecompass_engine.exe" if os.name == 'nt' else "codecompass_engine")
        else:
            return []

    try:
        # Hide console window on Windows
        si = None
        if os.name == 'nt':
            si = subprocess.STARTUPINFO()
            si.dwFlags |= subprocess.STARTF_USESHOWWINDOW

        full_cmd = [local_exe, cmd]

        # Run command and capture output
        res = subprocess.run(full_cmd, capture_output=True, text=True, startupinfo=si)

        # DEBUGGING BLOCK -----------------------------------------
        if res.returncode != 0 or not res.stdout.strip():
            with st.expander("⚠️ Debug: Backend Execution Details", expanded=True):
                st.write(f"**Command:** `{full_cmd}`")
                st.write(f"**Return Code:** `{res.returncode}` (Hex: {hex(res.returncode) if res.returncode else 'N/A'})")

                if res.returncode == 3221225785 or res.returncode == -1073741511:
                    st.error("💥 Error 0xC0000135/9: DLL Missing. Please click 'Recompile Engine' in sidebar.")

                st.write("**STDOUT:**")
                st.code(res.stdout if res.stdout else "(empty)")
                st.write("**STDERR:**")
                st.code(res.stderr if res.stderr else "(empty)")
        # ---------------------------------------------------------

        if res.returncode != 0:
            return []

        return res.stdout.strip().splitlines()
    except Exception as e:
        st.error(f"Backend Connection Error: {e}")
        return []

def parse_csv_lines(lines):
    if not lines: return []

    header_idx = -1
    for i, line in enumerate(lines):
        if line.strip().startswith("ID,Title"):
            header_idx = i
            break

    if header_idx == -1:
        return []

    headers = [h.strip() for h in lines[header_idx].split(',')]

    data = []
    for line in lines[header_idx+1:]:
        if not line.strip(): continue
        if line.startswith("---") or line.startswith("[CPP]"): continue

        parts = line.split(',')
        if len(parts) < len(headers): continue

        row = {}
        for i, header in enumerate(headers):
            if i < len(parts):
                row[header] = parts[i].strip()
        data.append(row)
    return data

# =========================================================
# UI LAYOUT
# =========================================================
st.title("🧭 CodeCompass")

# Sidebar for controls
with st.sidebar:
    st.header("⚙️ Settings")
    if st.button("🔨 Recompile Engine"):
        compile_engine()
        st.rerun()

    st.info("If the app hangs or crashes, try Recompiling.")

if not EXE_PATH and not os.path.exists("codecompass_engine.exe"):
    st.warning("⚠️ Engine not found. Compiling now...")
    if compile_engine():
        st.rerun()
    else:
        st.stop()

# Initialize session state
if 'full_library' not in st.session_state:
    st.session_state.full_library = []

# --- TABS ---
tab1, tab2, tab3 = st.tabs(["📘 Full Library", "🎓 Curriculum Planner", "⚡ Exam Cram"])

# ---------------------------------------------------------
# TAB 1: LIBRARY & DATA LOADER
# ---------------------------------------------------------
with tab1:
    st.header("Resource Library")

    col1, col2 = st.columns([1, 4])
    with col1:
        if st.button("🔄 Load/Refresh Data"):
            with st.spinner(" querying C++ Engine..."):
                raw_lines = run_cpp("LIST")
                if not raw_lines:
                    st.warning("Engine returned no output.")
                else:
                    st.session_state.full_library = parse_csv_lines(raw_lines)

    if st.session_state.full_library:
        df = pd.DataFrame(st.session_state.full_library)
        st.success(f"Loaded {len(df)} resources.")

        st.dataframe(
            df,
            column_config={
                "URL": st.column_config.LinkColumn("Link"),
                "Difficulty": st.column_config.ProgressColumn("Difficulty", format="%s/100", min_value=0, max_value=100),
                "Rating": st.column_config.NumberColumn("Rating", format="⭐ %.1f")
            },
            use_container_width=True,
            hide_index=True
        )
    else:
        st.info("Click 'Load/Refresh Data' to fetch resources from the C++ Engine.")

# ---------------------------------------------------------
# TAB 2: CURRICULUM (Topological Sort)
# ---------------------------------------------------------
with tab2:
    st.header("Generate Learning Path")
    st.markdown("Find the optimal dependency order to learn a specific topic.")

    titles = [item['Title'] for item in st.session_state.full_library]
    if not titles:
        raw_titles = run_cpp("TITLES")
        titles = [t.strip() for t in raw_titles if t.strip() and not t.startswith("LRUCache") and not t.startswith("[CPP]")]

    if titles:
        target_title = st.selectbox("I want to master:", titles)

        if st.button("📍 Generate Plan"):
            cmd = f"PLAN|{target_title}"
            raw_plan = run_cpp(cmd)
            plan_data = parse_csv_lines(raw_plan)

            if plan_data:
                st.success(f"Optimal Path found: {len(plan_data)} steps")
                for i, step in enumerate(plan_data):
                    st.markdown(f"**{i+1}. {step['Title']}**")
                    st.caption(f"Topic: {step['Topic']} | Difficulty: {step['Difficulty']}")
                    if i < len(plan_data) - 1:
                        st.markdown("⬇️ *unlocks*")
            else:
                st.warning("No dependencies found or item has no prerequisites.")
    else:
        st.warning("Please load the library in Tab 1 first.")

# ---------------------------------------------------------
# TAB 3: EXAM CRAM (Knapsack Optimizer)
# ---------------------------------------------------------
with tab3:
    st.header("Exam Cram Optimizer")
    st.markdown("Maximize your learning score within a limited time.")

    available_topics = []
    if st.session_state.full_library:
        available_topics = sorted(list(set(item['Topic'] for item in st.session_state.full_library)))

    col_t, col_s = st.columns(2)
    with col_t:
        if not available_topics: available_topics = ["Pointers", "LinkedList", "Trees", "Graphs", "Sorting", "Heaps", "Recursion"]
        selected_topic = st.selectbox("Topic to Cram:", available_topics)

    with col_s:
        time_limit = st.slider("Time Available (minutes):", 10, 180, 60)

    if st.button("⚡ Maximize Score"):
        cmd = f"CRAM|{selected_topic}|{time_limit}"
        with st.spinner("Calculating optimal schedule..."):
            raw_cram = run_cpp(cmd)
            cram_data = parse_csv_lines(raw_cram)

        if cram_data:
            total_score = sum(float(item['Rating']) for item in cram_data)
            total_time = sum(int(item['Duration']) for item in cram_data)

            m1, m2, m3 = st.columns(3)
            m1.metric("Selected Items", len(cram_data))
            m2.metric("Total Rating", f"{total_score:.1f}")
            m3.metric("Time Used", f"{total_time} min", delta=f"{time_limit - total_time} min left")

            st.table(cram_data)
        else:
            st.error("No suitable resources found for this time limit.")