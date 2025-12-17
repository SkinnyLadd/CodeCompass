"""
CodeCompass UI - Main Application
Streamlit-based UI for CodeCompass DSA Exam Prep Tool
"""

import streamlit as st
import pandas as pd

# Import modules
from ui_theme import initialize_theme, apply_theme, render_theme_toggle
from ui_backend import get_exe, run_cpp, parse_csv_lines, parse_analysis
from ui_visualizations import (
    visualize_avltree, visualize_trie, visualize_maxheap,
    visualize_graph, visualize_cache, visualize_stack
)

# Page configuration
st.set_page_config(page_title="CodeCompass", layout="wide", page_icon="🧭")

# Initialize and apply theme
initialize_theme()
apply_theme()

# =========================================================
# BACKEND CONNECTION
# =========================================================
EXE_PATH = get_exe()

# =========================================================
# UI LAYOUT & STATE
# =========================================================
st.title("🧭 CodeCompass: DSA Exam Prep")

if not EXE_PATH:
    st.error("Backend not found. Please compile 'codecompass_engine.exe' manually.")
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
if 'current_analysis' not in st.session_state:
    st.session_state.current_analysis = None
if 'show_analysis' not in st.session_state:
    st.session_state.show_analysis = False

# --- SIDEBAR: STUDY TRACKER ---
with st.sidebar:
    # Theme Toggle
    render_theme_toggle()

    st.divider()
    st.header("Study Session")

    if st.session_state.study_session:
        total_mins = sum(int(item.get('Duration', 0)) for item in st.session_state.study_session)
        st.metric("Total Time Committed", f"{total_mins} min")

        st.markdown("### Selected Topics:")
        for i, item in enumerate(st.session_state.study_session):
            col_text, col_x = st.columns([4, 1])
            with col_text:
                st.markdown(f"**{i+1}.** [{item['Title']}]({item['URL']})")
                st.caption(f"{item.get('Duration','?')}m")
            with col_x:
                if st.button("❌", key=f"del_{i}", help="Remove from session"):
                    st.session_state.study_session.pop(i)
                    st.rerun()
            st.divider()

        if st.button("Clear Session"):
            st.session_state.study_session = []
            st.rerun()
    else:
        st.info("Add topics from the Planner or Cram tabs to build your study list.")

# --- TABS ---
tab1, tab2, tab3, tab4, tab5 = st.tabs([" Resource Library ", " Dependency Roadmap ", " Exam Cram Optimizer ", " Smart Recommendations ", " DSA Analysis "])

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

    if st.button("Search / Filter"):
        cmd = "LIST"
        if search_query:
            cmd = f"SEARCH|{search_query}"
        elif sort_mode == "Difficulty (Easy->Hard)":
            cmd = "LIST|DIFFICULTY"
        elif sort_mode == "Topic":
            cmd = "LIST|TOPIC"

        raw_lines = run_cpp(cmd)
        st.session_state.view_library = parse_csv_lines(raw_lines)
        st.session_state.current_analysis = parse_analysis(raw_lines)

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
            hide_index=True,
            height=400
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

        if st.button("Generate Path"):
            cmd = f"PLAN|{target_title}"
            raw_plan = run_cpp(cmd)
            st.session_state.current_plan_data = parse_csv_lines(raw_plan)
            st.session_state.current_analysis = parse_analysis(raw_plan)

        # Display persistent results
        if st.session_state.current_plan_data:
            plan_data = st.session_state.current_plan_data

            # --- NEW: Bulk Add Header ---
            c1, c2 = st.columns([3, 1])
            with c1:
                st.success(f"Optimal Path: {len(plan_data)} Steps")
            with c2:
                if st.button("➕ Add All to Session"):
                    # Add all, avoid duplicates based on Title
                    current_titles = {i['Title'] for i in st.session_state.study_session}
                    count = 0
                    for step in plan_data:
                        if step['Title'] not in current_titles:
                            st.session_state.study_session.append(step)
                            count += 1
                    st.toast(f"Added {count} new items to session!", icon="📚")
                    st.rerun() # Refresh sidebar instantly
            # ---------------------------

            for i, step in enumerate(plan_data):
                col_a, col_b = st.columns([4, 1])
                with col_a:
                    st.markdown(f"**{i+1}. [{step['Title']}]({step['URL']})**")
                    st.caption(f"Topic: {step['Topic']} | Difficulty: {step['Difficulty']} | {step.get('Duration','?')} mins")
                with col_b:
                    # Check if already added
                    is_added = any(s['Title'] == step['Title'] for s in st.session_state.study_session)
                    if is_added:
                        st.button("✅", key=f"add_plan_{i}", disabled=True)
                    else:
                        if st.button("➕", key=f"add_plan_{i}"):
                            st.session_state.study_session.append(step)
                            st.rerun()

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
        if not available_topics:
            available_topics = ["Pointers", "Trees", "Graphs"]
        selected_topic = st.selectbox("Topic to Cram:", available_topics)

    with col_s:
        time_limit = st.slider("Time Available (minutes):", 15, 180, 60, step=15)

    if st.button("⚡ Maximize Score"):
        cmd = f"CRAM|{selected_topic}|{time_limit}"
        raw_cram = run_cpp(cmd)
        st.session_state.current_cram_data = parse_csv_lines(raw_cram)
        st.session_state.current_analysis = parse_analysis(raw_cram)

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
                    st.rerun()  # Force UI Update
    elif st.session_state.current_cram_data is not None:
        st.error("No suitable resources found. Try increasing time limit.")

# ---------------------------------------------------------
# TAB 4: RECOMMENDATIONS (MaxHeap)
# ---------------------------------------------------------
with tab4:
    st.subheader("Smart Recommendations")
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
        st.session_state.current_analysis = parse_analysis(raw_recs)

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
                        st.rerun()  # Force UI Update
                st.divider()

# ---------------------------------------------------------
# TAB 5: DSA ANALYSIS
# ---------------------------------------------------------
with tab5:
    st.subheader("Data Structure & Algorithm Analysis")
    st.markdown("**Educational Purpose:** View the internal state and performance metrics of data structures used in operations.")

    if st.session_state.current_analysis:
        analysis = st.session_state.current_analysis

        # Operation Info
        operation = analysis.get('OPERATION', 'Unknown')
        st.markdown(f"### Last Operation: `{operation}`")

        # Tree Statistics
        if 'TREE_HEIGHT' in analysis or 'NODE_COUNT' in analysis:
            st.markdown("#### AVL Tree State")
            col1, col2, col3, col4 = st.columns(4)

            with col1:
                tree_height = analysis.get('TREE_HEIGHT', 'N/A')
                st.metric("Tree Height", tree_height)
                st.caption("O(log N) search guarantee")

            with col2:
                node_count = analysis.get('NODE_COUNT', 'N/A')
                st.metric("Node Count", node_count)
                st.caption("Total nodes in tree")

            with col3:
                max_balance = analysis.get('MAX_BALANCE', 'N/A')
                st.metric("Max Balance", max_balance)
                st.caption("Largest imbalance factor")

            with col4:
                root_balance = analysis.get('ROOT_BALANCE', 'N/A')
                st.metric("Root Balance", root_balance)
                st.caption("Balance at root node")

            # Visual Tree Representation
            if 'TREE_EDGES' in analysis and 'TREE_NODES' in analysis:
                st.markdown("#### Tree Visualization")
                visualize_avltree(analysis['TREE_EDGES'], analysis['TREE_NODES'], analysis)

        # Visualize other data structures based on operation
        operation = analysis.get('OPERATION', '')

        # Trie Visualization (for SEARCH and CRAM)
        if 'TRIE_EDGES' in analysis and 'TRIE_NODES' in analysis and ('SEARCH' in operation or 'CRAM' in operation):
            st.markdown("#### Trie (Prefix Tree) Visualization")
            visualize_trie(analysis['TRIE_EDGES'], analysis['TRIE_NODES'], analysis)

        # MaxHeap Visualization (for SUGGEST)
        if 'HEAP_STRUCTURE' in analysis and 'SUGGEST' in operation:
            st.markdown("#### MaxHeap Visualization")
            visualize_maxheap(analysis['HEAP_STRUCTURE'], analysis)

        # Knowledge Graph Visualization (for PLAN)
        if 'GRAPH_EDGES' in analysis and 'GRAPH_NODES' in analysis and 'PLAN' in operation:
            st.markdown("#### Knowledge Graph Visualization")
            visualize_graph(analysis['GRAPH_EDGES'], analysis['GRAPH_NODES'], analysis)

        # LRU Cache Visualization (for SEARCH and CRAM)
        if 'CACHE_STRUCTURE' in analysis and ('SEARCH' in operation or 'CRAM' in operation):
            st.markdown("#### LRU Cache Visualization")
            visualize_cache(analysis['CACHE_STRUCTURE'], analysis)

        # Stack Visualization (for SEARCH and BACK)
        if 'STACK_STRUCTURE' in analysis and ('SEARCH' in operation or 'BACK' in operation):
            st.markdown("#### Stack Visualization")
            visualize_stack(analysis['STACK_STRUCTURE'], analysis)

        # Performance Metrics
        st.markdown("#### ⏱ Performance Metrics")

        timing_cols = st.columns(3)

        with timing_cols[0]:
            if 'TOTAL_TIME_US' in analysis:
                total_time = float(analysis['TOTAL_TIME_US'])
                st.metric("Total Time", f"{total_time:.2f} μs",
                         help="Total execution time in microseconds")

        with timing_cols[1]:
            if 'TRAVERSAL_TIME_US' in analysis:
                trav_time = float(analysis['TRAVERSAL_TIME_US'])
                st.metric("Traversal Time", f"{trav_time:.2f} μs",
                         help="Time to traverse AVL tree (inorder)")

        with timing_cols[2]:
            if 'SORT_TIME_US' in analysis:
                sort_time = float(analysis['SORT_TIME_US'])
                st.metric("Sort Time", f"{sort_time:.2f} μs",
                         help="Time for sorting algorithm")

        # Algorithm Information
        if 'SORT_ALGORITHM' in analysis:
            st.markdown("#### Sorting Algorithm")
            algo = analysis['SORT_ALGORITHM']
            if algo == "QUICKSORT":
                st.info(f"**Algorithm:** {algo} | **Time Complexity:** O(n log n) average, O(n²) worst | **Space:** O(log n)")
            elif algo == "MERGESORT":
                st.info(f"**Algorithm:** {algo} | **Time Complexity:** O(n log n) | **Space:** O(n)")
            else:
                st.info(f"**Algorithm:** {algo}")

        # Data Structure Usage
        if 'DATA_STRUCTURE' in analysis:
            st.markdown("#### Data Structure Used")
            ds = analysis['DATA_STRUCTURE']
            if ds == "MAXHEAP":
                st.info(f"**Structure:** {ds} | **Time Complexity:** O(log n) insert/extract | **Space:** O(n)")
            elif ds == "KNAPSACK_DP":
                st.info(f"**Algorithm:** Dynamic Programming (0/1 Knapsack) | **Time Complexity:** O(n × W) | **Space:** O(n × W)")

        # Additional Metrics
        st.markdown("#### Operation Details")
        detail_cols = st.columns(2)

        with detail_cols[0]:
            if 'RESULT_COUNT' in analysis:
                st.metric("Results Returned", analysis['RESULT_COUNT'])
            if 'TREE_SEARCHES' in analysis:
                st.metric("Tree Searches", analysis['TREE_SEARCHES'])
            if 'TRIE_SEARCH_TIME_US' in analysis:
                trie_time = float(analysis['TRIE_SEARCH_TIME_US'])
                st.metric("Trie Search Time", f"{trie_time:.2f} μs")
            if 'TREE_SEARCH_TIME_US' in analysis:
                tree_search_time = float(analysis['TREE_SEARCH_TIME_US'])
                st.metric("Tree Search Time", f"{tree_search_time:.2f} μs")
            if 'OPTIMIZER_TIME_US' in analysis:
                opt_time = float(analysis['OPTIMIZER_TIME_US'])
                st.metric("Optimizer Time", f"{opt_time:.2f} μs")
            if 'CANDIDATES' in analysis:
                st.metric("Candidates Found", analysis['CANDIDATES'])

        with detail_cols[1]:
            if 'HEAP_OPERATIONS' in analysis:
                st.metric("Heap Operations", analysis['HEAP_OPERATIONS'])
            if 'EXTRACTED_COUNT' in analysis:
                st.metric("Extracted Items", analysis['EXTRACTED_COUNT'])
            if 'GRAPH_SEARCH_TIME_US' in analysis:
                graph_time = float(analysis['GRAPH_SEARCH_TIME_US'])
                st.metric("Graph Search Time", f"{graph_time:.2f} μs")
            if 'FILTER_TIME_US' in analysis:
                filter_time = float(analysis['FILTER_TIME_US'])
                st.metric("Filter Time", f"{filter_time:.2f} μs")

        # Raw Analysis Data (Expandable)
        with st.expander("Raw Analysis Data"):
            st.json(analysis)

    else:
        st.info("Perform an operation (Search, List, Plan, Suggest, or Cram) to see DSA analysis metrics.")
        st.markdown("""
        **What you'll see:**
        - **AVL Tree State:** Height, node count, balance factors
        - ️ **Performance Metrics:** Execution times for each operation phase
        -  **Algorithm Details:** Sorting algorithms used and their complexity
        -  **Data Structure Usage:** Which structures are used and their properties
        -  **Trie Visualization:** Prefix tree for efficient search
        -  **MaxHeap Visualization:** Priority queue for top-rated items
        - ️ **Knowledge Graph Visualization:** Dependency relationships
        -  **LRU Cache Visualization:** Recently accessed items
        -  **Stack Visualization:** Browsing history (LIFO)
        """)
