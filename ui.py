import streamlit as st
import subprocess
import os
import pandas as pd

st.set_page_config(page_title="CodeCompass", layout="wide", page_icon="🧭")

# =========================================================
# THEME CONFIGURATION
# =========================================================
st.markdown("""
    <style>
    .stApp { background-color: #0e0e0e; color: #e0e0e0; }
    div.stButton > button { background-color: #d90429; color: white; border: none; font-weight: bold; }
    div.stButton > button:hover { background-color: #ef233c; color: white; }
    /* Link styling */
    a { color: #ff4b4b !important; text-decoration: none; }
    a:hover { text-decoration: underline; }
    </style>
""", unsafe_allow_html=True)

# =========================================================
# BACKEND CONNECTION
# =========================================================
def get_exe():
    name = "codecompass_engine.exe" if os.name == 'nt' else "codecompass_engine"
    paths = [name, os.path.join("cmake-build-debug", name), os.path.join("..", "cmake-build-debug", name)]
    for p in paths:
        if os.path.exists(p): return os.path.abspath(p)
    return None

EXE_PATH = get_exe()

def run_cpp(cmd):
    if not EXE_PATH: return []
    try:
        si = None
        if os.name == 'nt':
            si = subprocess.STARTUPINFO()
            si.dwFlags |= subprocess.STARTF_USESHOWWINDOW

        full_cmd = [EXE_PATH, cmd]
        res = subprocess.run(full_cmd, capture_output=True, text=True, startupinfo=si)

        if res.returncode != 0 or not res.stdout.strip():
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

    if header_idx == -1: return []

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
# UI LAYOUT & STATE
# =========================================================
st.title("🧭 CodeCompass: DSA Exam Prep")

if not EXE_PATH:
    st.error("⚠️ Backend not found. Please compile 'codecompass_engine.exe' manually.")
    st.stop()

# STATE MANAGEMENT
if 'master_library' not in st.session_state:
    st.session_state.master_library = []
    raw = run_cpp("LIST")
    st.session_state.master_library = parse_csv_lines(raw)

if 'view_library' not in st.session_state:
    st.session_state.view_library = []

if 'study_session' not in st.session_state:
    st.session_state.study_session = []

if 'current_plan_data' not in st.session_state:
    st.session_state.current_plan_data = None
if 'current_cram_data' not in st.session_state:
    st.session_state.current_cram_data = None
if 'current_recs_data' not in st.session_state:
    st.session_state.current_recs_data = None

# --- SIDEBAR: STUDY TRACKER ---
with st.sidebar:
    st.header("📚 Study Session")

    if st.session_state.study_session:
        total_mins = sum(int(item.get('Duration', 0)) for item in st.session_state.study_session)
        st.metric("Total Time Committed", f"{total_mins} min")

        st.markdown("### Selected Topics:")
        for i, item in enumerate(st.session_state.study_session):
            st.markdown(f"{i+1}. [{item['Title']}]({item['URL']}) ({item.get('Duration','?')}m)")

        if st.button("Clear Session"):
            st.session_state.study_session = []
            st.rerun()
    else:
        st.info("Add topics from the Planner or Cram tabs to build your study list.")

# --- TABS ---
tab1, tab2, tab3, tab4 = st.tabs(["🔎 Resource Library", "🗺️ Dependency Roadmap", "⚡ Exam Cram Optimizer", "🏆 Smart Recommendations"])

# ---------------------------------------------------------
# TAB 1: LIBRARY & SEARCH
# ---------------------------------------------------------
with tab1:
    st.subheader("Browse Resources")

    col_search, col_sort = st.columns([3, 1])
    with col_search:
        search_query = st.text_input("Search Topics (Prefix Search):", placeholder="e.g. Graph, Tree, Sort")
    with col_sort:
        sort_mode = st.selectbox("Sort By:", ["Default", "Difficulty (Easy->Hard)", "Topic"])

    if st.button("🚀 Search / Filter"):
        cmd = "LIST"
        if search_query:
            cmd = f"SEARCH|{search_query}"
        elif sort_mode == "Difficulty (Easy->Hard)":
            cmd = "LIST|DIFFICULTY"
        elif sort_mode == "Topic":
            cmd = "LIST|TOPIC"

        raw_lines = run_cpp(cmd)
        st.session_state.view_library = parse_csv_lines(raw_lines)

    data_source = st.session_state.view_library if st.session_state.view_library else st.session_state.master_library

    if data_source:
        df = pd.DataFrame(data_source)
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
        st.info("Library empty. Check backend connection.")

# ---------------------------------------------------------
# TAB 2: CURRICULUM
# ---------------------------------------------------------
with tab2:
    st.subheader("Prerequisite Planner")
    st.markdown("Select a target topic to generate a valid learning path (Topological Sort).")

    titles = [item['Title'] for item in st.session_state.master_library]

    if titles:
        target_title = st.selectbox("I need to learn:", titles)

        if st.button("📍 Generate Path"):
            cmd = f"PLAN|{target_title}"
            raw_plan = run_cpp(cmd)
            st.session_state.current_plan_data = parse_csv_lines(raw_plan)

        if st.session_state.current_plan_data:
            plan_data = st.session_state.current_plan_data
            st.success(f"Optimal Path: {len(plan_data)} Steps")

            for i, step in enumerate(plan_data):
                col_a, col_b = st.columns([4, 1])
                with col_a:
                    st.markdown(f"**{i+1}. [{step['Title']}]({step['URL']})**")
                    st.caption(f"Topic: {step['Topic']} | Difficulty: {step['Difficulty']} | {step.get('Duration','?')} mins")
                with col_b:
                    if st.button("➕ Add", key=f"add_plan_{i}"):
                        st.session_state.study_session.append(step)
                        st.rerun() # Force UI Update

                if i < len(plan_data) - 1:
                    st.markdown("⬇️ *unlocks*")
        elif st.session_state.current_plan_data is not None:
            st.warning("No dependencies found (Item is foundational).")

    else:
        st.warning("Library loading...")

# ---------------------------------------------------------
# TAB 3: EXAM CRAM
# ---------------------------------------------------------
with tab3:
    st.subheader("Time-Constrained Optimization")
    st.markdown("Use **Dynamic Programming** (Knapsack) to find the highest-rated topics for your available time.")

    available_topics = sorted(list(set(item['Topic'] for item in st.session_state.master_library)))

    col_t, col_s = st.columns(2)
    with col_t:
        if not available_topics: available_topics = ["Pointers", "Trees", "Graphs"]
        selected_topic = st.selectbox("Topic to Cram:", available_topics)

    with col_s:
        time_limit = st.slider("Time Available (minutes):", 15, 180, 60, step=15)

    if st.button("⚡ Maximize Score"):
        cmd = f"CRAM|{selected_topic}|{time_limit}"
        raw_cram = run_cpp(cmd)
        st.session_state.current_cram_data = parse_csv_lines(raw_cram)

    if st.session_state.current_cram_data:
        cram_data = st.session_state.current_cram_data
        total_score = sum(float(item['Rating']) for item in cram_data)
        total_time = sum(int(item['Duration']) for item in cram_data)

        m1, m2, m3 = st.columns(3)
        m1.metric("Items", len(cram_data))
        m2.metric("Rating Score", f"{total_score:.1f}")
        m3.metric("Time Used", f"{total_time}m / {time_limit}m")

        st.markdown("### Suggested Study List:")
        for item in cram_data:
            c1, c2 = st.columns([4, 1])
            with c1:
                st.markdown(f"**[{item['Title']}]({item['URL']})**")
                st.caption(f"{item.get('Duration','?')} min | ⭐ {item['Rating']}")
            with c2:
                if st.button("➕ Add", key=f"add_cram_{item['ID']}"):
                    st.session_state.study_session.append(item)
                    st.rerun() # Force UI Update
    elif st.session_state.current_cram_data is not None:
        st.error("No suitable resources found. Try increasing time limit.")

# ---------------------------------------------------------
# TAB 4: RECOMMENDATIONS (MaxHeap)
# ---------------------------------------------------------
with tab4:
    st.subheader("🏆 Smart Recommendations")
    st.markdown("Filter items by difficulty level and fetch the highest rated ones using a **Priority Queue**.")

    col_k, col_diff = st.columns(2)
    with col_k:
        k = st.slider("How many suggestions?", 1, 10, 5)
    with col_diff:
        level = st.selectbox("Difficulty Level:", ["ALL", "BEGINNER", "INTERMEDIATE", "ADVANCED"])

    if st.button("🌟 Get Top Rated"):
        cmd = f"SUGGEST|{k}|{level}"
        raw_recs = run_cpp(cmd)
        st.session_state.current_recs_data = parse_csv_lines(raw_recs)

    if st.session_state.current_recs_data:
        recs = st.session_state.current_recs_data

        for i, item in enumerate(recs):
            with st.container():
                c1, c2, c3 = st.columns([3, 1, 1])
                with c1:
                    st.markdown(f"### {i+1}. [{item['Title']}]({item['URL']})")
                    st.caption(f"Topic: {item['Topic']}")
                with c2:
                    st.metric("Rating", f"⭐ {item['Rating']}")
                with c3:
                    st.metric("Diff", f"{item['Difficulty']}/100")
                    if st.button("➕ Add", key=f"add_rec_{item['ID']}"):
                        st.session_state.study_session.append(item)
                        st.rerun() # Force UI Update
                st.divider()