import streamlit as st
import subprocess
import os
import pandas as pd
try:
    import graphviz
    GRAPHVIZ_AVAILABLE = True
except ImportError:
    GRAPHVIZ_AVAILABLE = False
    try:
        import networkx as nx
        import matplotlib.pyplot as plt
        import matplotlib
        matplotlib.use('Agg')
        NETWORKX_AVAILABLE = True
    except ImportError:
        NETWORKX_AVAILABLE = False

st.set_page_config(page_title="CodeCompass", layout="wide", page_icon="🧭")

# =========================================================
# THEME CONFIGURATION
# =========================================================

# Initialize theme in session state
if 'dark_mode' not in st.session_state:
    st.session_state.dark_mode = True

# Apply theme based on state
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

def parse_analysis(lines):
    """Parse analysis data from backend output"""
    analysis = {}
    in_analysis = False
    
    # State tracking for different structures
    current_section = None
    current_data = []
    
    for line in lines:
        line = line.strip()
        if line == "---ANALYSIS---":
            in_analysis = True
            continue
        if line == "---END_ANALYSIS---":
            break
        
        # Handle structure sections
        if line.endswith("_START"):
            current_section = line.replace("_START", "")
            current_data = []
            continue
        if line.endswith("_END"):
            section_name = line.replace("_END", "")
            if section_name == current_section:
                analysis[current_section] = current_data
                current_section = None
                current_data = []
            continue
        
        if current_section:
            current_data.append(line)
        elif in_analysis and ':' in line:
            key, value = line.split(':', 1)
            analysis[key.strip()] = value.strip()
    
    return analysis

# =========================================================
# VISUALIZATION FUNCTIONS
# =========================================================

def visualize_trie(edges, nodes, analysis):
    """Visualize Trie structure - Clean text-based tree representation"""
    if not edges or not nodes:
        st.info("Trie structure is empty.")
        return
    
    node_info = {}
    for node_str in nodes:
        parts = node_str.rsplit(':', 2)
        if len(parts) >= 3:
            node_id = parts[0]
            is_end = parts[1] == "1"
            resource_count = parts[2]
            node_info[node_id] = (is_end, resource_count)
    
    # Build tree structure
    tree_structure = {}
    root_id = "ROOT"
    node_depth = {root_id: 0}
    
    # Build parent-child relationships
    for edge_str in edges:
        if '->' in edge_str:
            parts = edge_str.split('->')
            if len(parts) >= 2:
                parent = parts[0]
                child_info = parts[1].rsplit(':', 1)
                child = child_info[0]
                char = child_info[1] if len(child_info) > 1 else ''
                
                if parent not in tree_structure:
                    tree_structure[parent] = []
                tree_structure[parent].append((child, char))
                node_depth[child] = node_depth.get(parent, 0) + 1
    
    # Count statistics
    total_nodes = len(node_info)
    end_nodes = sum(1 for is_end, _ in node_info.values() if is_end)
    
    # Show statistics
    col1, col2, col3 = st.columns(3)
    with col1:
        st.metric("Total Nodes", total_nodes)
    with col2:
        st.metric("End Nodes", end_nodes, help="Nodes representing complete words")
    with col3:
        avg_children = sum(len(tree_structure.get(node, [])) for node in node_info) / max(total_nodes, 1)
        st.metric("Avg Children", f"{avg_children:.1f}")
    
    # Text-based tree representation
    def build_tree_text(node, prefix="", is_last=True, depth=0, max_depth=4, max_children=10):
        """Build tree text representation"""
        if depth > max_depth:
            return []
        
        lines = []
        is_end, res_count = node_info.get(node, (False, "0"))
        
        # Node marker
        connector = "└── " if is_last else "├── "
        
        # Display format
        if node == "ROOT":
            display_text = "ROOT"
        else:
            # Show last 25 characters of the prefix path
            display_text = node[-25:] if len(node) > 25 else node
        
        # Markers for node type
        if is_end:
            marker = "🟣"  # Purple circle for end nodes
            end_marker = f" [{res_count} IDs]" if int(res_count) > 0 else " [END]"
        else:
            marker = "⚪"  # White circle for intermediate
            end_marker = ""
        
        lines.append(f"{prefix}{connector}{marker} {display_text}{end_marker}")
        
        # Process children
        if node in tree_structure:
            children = tree_structure[node]
            # Sort by character for better readability
            sorted_children = sorted(children, key=lambda x: (x[1], x[0]))[:max_children]
            
            next_prefix = prefix + ("    " if is_last else "│   ")
            
            for i, (child, char) in enumerate(sorted_children):
                is_last_child = (i == len(sorted_children) - 1)
                
                # Show character label
                char_label = f"'{char}' → " if char else ""
                
                # Recursively build child tree
                child_lines = build_tree_text(child, next_prefix, is_last_child, depth + 1, max_depth, max_children)
                
                # Add character label to first child line if needed
                if child_lines and char:
                    first_line = child_lines[0]
                    # Insert char label after connector
                    parts = first_line.split(' ', 1)
                    if len(parts) == 2:
                        child_lines[0] = parts[0] + f" {char_label}" + parts[1]
                
                lines.extend(child_lines)
            
            # Show if there are more children
            if len(children) > max_children:
                more_count = len(children) - max_children
                lines.append(f"{next_prefix}... ({more_count} more children)")
        
        return lines
    
    # Build and display tree
    if root_id in node_info or root_id in tree_structure:
        # Show limited view by default
        with st.expander("🌲 Trie Tree Structure", expanded=True):
            col1, col2 = st.columns([3, 1])
            with col1:
                depth_option = st.selectbox("Max Depth:", [3, 4, 5, 6, 10], index=1, key="trie_depth")
            with col2:
                children_option = st.selectbox("Max Children:", [5, 8, 10, 15, 20], index=1, key="trie_children")
            
            tree_lines = build_tree_text(root_id, max_depth=depth_option, max_children=children_option)
            
            if tree_lines:
                # Display as formatted text
                tree_text = "\n".join(tree_lines)
                st.code(tree_text, language="text")
                
                st.markdown("""
                **Legend:**
                - 🟣 = End node (complete word, contains resource IDs)
                - ⚪ = Intermediate node (prefix, not a complete word)
                - `'c' →` = Character on edge (shows the path taken)
                - `[N IDs]` = Number of resource IDs stored at this node
                """)
            else:
                st.info("Tree is empty")
    else:
        st.info("ROOT node not found in Trie structure")

def visualize_maxheap(heap_data, analysis):
    """Visualize MaxHeap structure"""
    if not heap_data:
        st.info("Heap is empty.")
        return
    
    # Parse heap structure
    nodes = {}
    edges = []
    heap_size = 0
    
    for item in heap_data:
        if item.startswith("HEAP_SIZE:"):
            heap_size = int(item.split(':')[1])
        elif item.startswith("EDGE:"):
            parts = item.replace("EDGE:", "").split('->')
            if len(parts) >= 2:
                parent = parts[0]
                child_info = parts[1].split(':')
                child = child_info[0]
                edge_type = child_info[1] if len(child_info) > 1 else ''
                edges.append((parent, child, edge_type))
        elif item.startswith("NODE:"):
            parts = item.replace("NODE:", "").split(':')
            if len(parts) >= 3:
                node_id = parts[0]
                rating = parts[1]
                index = parts[2]
                nodes[node_id] = (rating, index)
    
    if NETWORKX_AVAILABLE:
        G = nx.DiGraph()
        
        # Add all nodes
        for node_id, (rating, index) in nodes.items():
            G.add_node(node_id, rating=rating, index=index)
        
        # Add edges
        for parent, child, edge_type in edges:
            G.add_edge(parent, child, label=edge_type)
        
        # Create hierarchical tree layout for heap
        if len(G.nodes()) > 0:
            # Find root (node with no incoming edges)
            roots = [n for n in G.nodes() if G.in_degree(n) == 0]
            root = roots[0] if roots else list(G.nodes())[0]
            
            # Create hierarchical positions
            def hierarchy_pos(G, root, width=1.0, vert_gap=0.3, vert_loc=0, xcenter=0.5):
                def _hierarchy_pos(G, root, width=1., vert_gap=0.3, vert_loc=0, xcenter=0.5, pos=None, parent=None):
                    if pos is None:
                        pos = {root: (xcenter, vert_loc)}
                    else:
                        pos[root] = (xcenter, vert_loc)
                    children = list(G.successors(root))
                    if len(children) != 0:
                        dx = width / len(children)
                        nextx = xcenter - width/2 - dx/2
                        for child in children:
                            nextx += dx
                            pos = _hierarchy_pos(G, child, width=dx, vert_gap=vert_gap,
                                                vert_loc=vert_loc-vert_gap, xcenter=nextx,
                                                pos=pos, parent=root)
                    return pos
                return _hierarchy_pos(G, root, width, vert_gap, vert_loc, xcenter)
            
            try:
                pos = hierarchy_pos(G, root, width=10, vert_gap=0.8)
            except:
                pos = nx.spring_layout(G, k=2.5, iterations=150, seed=42)
        else:
            pos = {}
        
        if len(G.nodes()) > 0:
            fig, ax = plt.subplots(figsize=(12, 10))
            ax.set_facecolor('#f8f9fa')
            
            # Color nodes: Green theme for MaxHeap (different from Trie purple and Graph blue)
            node_colors = []
            labels = {}
            root_nodes = [n for n in G.nodes() if G.in_degree(n) == 0]
            
            for node_id in G.nodes():
                rating, index = nodes[node_id]
                # Root node gets darker green
                if node_id in root_nodes:
                    node_colors.append('#228B22')  # Forest green for root
                else:
                    node_colors.append('#90EE90')  # Light green for others
                labels[node_id] = f"ID:{node_id}\nR:{rating}\nI:{index}"
            
            # Draw edges
            nx.draw_networkx_edges(G, pos, ax=ax, edge_color='#4a5568', width=2,
                                 arrows=True, arrowsize=15, alpha=0.6)
            
            # Draw nodes
            nx.draw_networkx_nodes(G, pos, ax=ax, node_color=node_colors,
                                 node_size=2500, alpha=0.9,
                                 edgecolors='#2d3748', linewidths=2)
            
            # Draw labels
            nx.draw_networkx_labels(G, pos, labels, ax=ax, font_size=8,
                                  font_weight='bold', font_color='black')
            
            # Draw edge labels (L/R for left/right)
            edge_labels = {(u, v): d.get('label', '') for u, v, d in G.edges(data=True)}
            nx.draw_networkx_edge_labels(G, pos, edge_labels, ax=ax, font_size=9)
            
            ax.axis('off')
            plt.tight_layout()
            st.pyplot(fig)
            st.caption("🟢 **MaxHeap:** Dark green = Root (highest rating) | Light green = Other nodes | Parent rating ≥ Child rating")
            plt.close(fig)
        else:
            st.info("Heap is empty")
    else:
        st.info("Install networkx for MaxHeap visualization: `pip install networkx matplotlib`")
        st.code("\n".join(heap_data[:30]))

def visualize_graph(graph_edges, graph_nodes, analysis):
    """Visualize Knowledge Graph structure"""
    if not graph_edges or not graph_nodes:
        st.info("Graph is empty.")
        return
    
    if GRAPHVIZ_AVAILABLE:
        dot = graphviz.Digraph(comment='Knowledge Graph', format='svg', engine='dot')
        dot.attr(rankdir='LR', size='14,10')
        dot.attr('node', shape='box', style='filled,rounded', fontname='Arial', fontsize='10')
        dot.attr('edge', color='#FF6B6B', penwidth='2', arrowsize='1.2')
        
        # Add all nodes - Blue theme for Knowledge Graph (different from Trie purple and MaxHeap green)
        for node_str in graph_nodes:
            dot.node(node_str, label=f"<b>{node_str}</b>", fillcolor='#4A90E2')
        
        # Add edges
        for edge_str in graph_edges:
            if '->' in edge_str:
                parts = edge_str.split('->')
                if len(parts) >= 2:
                    dot.edge(parts[0], parts[1], label="unlocks", color='#2E5C8A')
        
        st.graphviz_chart(dot.source)
        st.caption("🔵 **Knowledge Graph:** Blue nodes showing prerequisites → dependencies")
    elif NETWORKX_AVAILABLE:
        G = nx.DiGraph()
        for node_str in graph_nodes:
            G.add_node(node_str)
        for edge_str in graph_edges:
            if '->' in edge_str:
                parts = edge_str.split('->')
                if len(parts) >= 2:
                    G.add_edge(parts[0], parts[1])
        
        pos = nx.spring_layout(G, k=2, iterations=100, seed=42)
        fig, ax = plt.subplots(figsize=(12, 8))
        ax.set_facecolor('#f8f9fa')
        
        # Blue theme for Knowledge Graph
        nx.draw_networkx_nodes(G, pos, ax=ax, node_color='#4A90E2', node_size=2000, alpha=0.9, 
                              edgecolors='#2E5C8A', linewidths=2)
        nx.draw_networkx_edges(G, pos, ax=ax, edge_color='#2E5C8A', width=2, arrows=True, arrowsize=20)
        nx.draw_networkx_labels(G, pos, ax=ax, font_size=9, font_weight='bold', font_color='white')
        
        ax.axis('off')
        plt.tight_layout()
        st.pyplot(fig)
        st.caption("🔵 **Knowledge Graph:** Blue nodes showing prerequisites → dependencies")
        plt.close(fig)
    else:
        st.info("Install graphviz or networkx for graph visualization")
        st.code("\n".join(graph_edges[:30]))

def visualize_cache(cache_data, analysis):
    """Visualize LRU Cache structure"""
    if not cache_data:
        st.info("Cache is empty.")
        return
    
    capacity = 5
    size = 0
    items = []
    
    for item in cache_data:
        if item.startswith("CAPACITY:"):
            capacity = int(item.split(':')[1])
        elif item.startswith("SIZE:"):
            size = int(item.split(':')[1])
        elif item.startswith("NODE:"):
            parts = item.split(':')
            if len(parts) >= 4:
                node_id = parts[1]
                title = parts[2]
                position = parts[3]
                items.append((node_id, title, position))
    
    st.markdown(f"**Cache Status:** {size}/{capacity} items (MRU → LRU)")
    
    if items:
        for i, (node_id, title, pos) in enumerate(items):
            # Better contrast: darker backgrounds with white text
            if i == 0:
                # MRU - darker green background with white text
                bg_color = '#228B22'
                text_color = '#FFFFFF'
                border_color = '#006400'
                label = "MRU"
            else:
                # Other items - darker blue background with white text
                bg_color = '#1E3A8A'
                text_color = '#FFFFFF'
                border_color = '#1E40AF'
                label = f"#{i+1}"
            
            st.markdown(f"""
            <div style="background-color: {bg_color}; color: {text_color}; padding: 12px; margin: 5px; 
                        border-radius: 5px; border-left: 5px solid {border_color}; 
                        box-shadow: 0 2px 4px rgba(0,0,0,0.2);">
                <strong>[{label}] ID: {node_id}</strong> - {title[:50]}{'...' if len(title) > 50 else ''}
            </div>
            """, unsafe_allow_html=True)
        st.caption("🟢 **Top (MRU)** = Most Recently Used | 🔵 **Bottom (LRU)** = Least Recently Used (will be evicted next)")
    else:
        st.info("Cache is empty.")

def visualize_stack(stack_data, analysis):
    """Visualize Stack structure"""
    if not stack_data:
        st.info("Stack is empty.")
        return
    
    size = 0
    items = []
    
    for item in stack_data:
        if item.startswith("SIZE:"):
            size = int(item.split(':')[1])
        elif item.startswith("ITEM:"):
            parts = item.split(':')
            if len(parts) >= 4:
                node_id = parts[1]
                title = parts[2]
                position = parts[3]
                items.append((node_id, title, position))
    
    st.markdown(f"**Stack Status:** {size} items (Top → Bottom, LIFO)")
    
    if items:
        for i, (node_id, title, pos) in enumerate(items):
            # Better contrast: darker backgrounds with white text
            if i == 0:
                # Top of stack - darker gold/orange with white text
                bg_color = '#D97706'
                text_color = '#FFFFFF'
                border_color = '#B45309'
                label = "TOP"
            else:
                # Other items - darker blue background with white text
                bg_color = '#1E3A8A'
                text_color = '#FFFFFF'
                border_color = '#1E40AF'
                label = f"#{i+1}"
            
            st.markdown(f"""
            <div style="background-color: {bg_color}; color: {text_color}; padding: 12px; margin: 5px; 
                        border-radius: 5px; border-left: 5px solid {border_color}; 
                        box-shadow: 0 2px 4px rgba(0,0,0,0.2);">
                <strong>[{label}] ID: {node_id}</strong> - {title[:50]}{'...' if len(title) > 50 else ''}
            </div>
            """, unsafe_allow_html=True)
        st.caption("🟠 **Top** = Most recent (will be popped next, LIFO) | 🔵 **Bottom** = Oldest")
    else:
        st.info("Stack is empty.")

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
if 'current_analysis' not in st.session_state:
    st.session_state.current_analysis = None
if 'show_analysis' not in st.session_state:
    st.session_state.show_analysis = False

# --- SIDEBAR: STUDY TRACKER ---
with st.sidebar:
    # Theme Toggle
    st.header("⚙️ Settings")
    dark_mode = st.toggle("🌙 Dark Mode", value=st.session_state.dark_mode, key="theme_toggle")
    if dark_mode != st.session_state.dark_mode:
        st.session_state.dark_mode = dark_mode
        st.rerun()
    
    st.divider()
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
tab1, tab2, tab3, tab4, tab5 = st.tabs(["🔎 Resource Library", "🗺️ Dependency Roadmap", "⚡ Exam Cram Optimizer", "🏆 Smart Recommendations", "📊 DSA Analysis"])

# ---------------------------------------------------------
# TAB 1: LIBRARY & SEARCH
# ---------------------------------------------------------
with tab1:
    st.subheader("Browse Resources")

    col_search, col_sort = st.columns([3, 1])
    with col_search:
        search_query = st.text_input("Search Topics (Prefix Search):", placeholder="e.g. Graph, Tree, Sort")
    with col_sort:
        sort_mode = st.selectbox("Sort By:", ["Default", "Difficulty (Easy->Hard)", "Title (A-Z)"])

    if st.button("🚀 Search / Filter"):
        cmd = "LIST"
        if search_query:
            cmd = f"SEARCH|{search_query}"
        elif sort_mode == "Difficulty (Easy->Hard)":
            cmd = "LIST|DIFFICULTY"
        elif sort_mode == "Title (A-Z)":
            cmd = "LIST|TITLE"

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

        if st.button("📍 Generate Path"):
            cmd = f"PLAN|{target_title}"
            raw_plan = run_cpp(cmd)
            st.session_state.current_plan_data = parse_csv_lines(raw_plan)
            st.session_state.current_analysis = parse_analysis(raw_plan)

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
                        st.rerun() # Force UI Update
                st.divider()

# ---------------------------------------------------------
# TAB 5: DSA ANALYSIS (Hidden by default, shown when analysis exists)
# ---------------------------------------------------------
with tab5:
    st.subheader("📊 Data Structure & Algorithm Analysis")
    st.markdown("**Educational Purpose:** View the internal state and performance metrics of data structures used in operations.")
    
    if st.session_state.current_analysis:
        analysis = st.session_state.current_analysis
        
        # Operation Info
        operation = analysis.get('OPERATION', 'Unknown')
        st.markdown(f"### Last Operation: `{operation}`")
        
        # Tree Statistics
        if 'TREE_HEIGHT' in analysis or 'NODE_COUNT' in analysis:
            st.markdown("#### 🌳 AVL Tree State")
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
                st.markdown("#### 🌲 Tree Visualization")
                
                # Parse tree structure
                edges = analysis['TREE_EDGES']
                nodes = analysis['TREE_NODES']
                
                # Create node info dictionary: id -> (height, balance)
                node_info = {}
                root_id = None
                for node_str in nodes:
                    parts = node_str.split(':')
                    if len(parts) >= 3:
                        node_id = parts[0]
                        height = parts[1]
                        balance = parts[2]
                        node_info[node_id] = (height, balance)
                        if root_id is None:  # First node is root (from preorder)
                            root_id = node_id
                
                # Parse edges and find root
                all_children = set()
                edge_list = []
                all_nodes_set = set(node_info.keys())
                
                for edge_str in edges:
                    if '->' in edge_str:
                        parts = edge_str.split('->')
                        if len(parts) >= 2:
                            parent = parts[0]
                            child_part = parts[1].split(':')[0]  # Remove :L or :R
                            all_children.add(child_part)
                            edge_list.append((parent, child_part))
                
                # Find root (node that is not a child of any other node)
                if not root_id:
                    all_parents = {e[0] for e in edge_list}
                    root_candidates = all_parents - all_children
                    if root_candidates:
                        root_id = root_candidates.pop()
                    elif all_nodes_set:
                        # If no edges, first node is root
                        root_id = list(all_nodes_set)[0]
                
                if root_id and GRAPHVIZ_AVAILABLE:
                    # Use Graphviz for visualization with improved formatting
                    dot = graphviz.Digraph(comment='AVL Tree', format='svg', engine='dot')
                    dot.attr(rankdir='TB', size='14,10', dpi=300)
                    dot.attr('node', shape='ellipse', style='filled,rounded', 
                            fontname='Arial', fontsize='12', fontcolor='black',
                            width='1.2', height='0.8', margin='0.1')
                    dot.attr('edge', color='#4a5568', penwidth='2', arrowsize='0.8')
                    
                    # Add nodes with cleaner labels
                    for node_id, (height, balance) in node_info.items():
                        balance_int = int(balance)
                        if abs(balance_int) <= 1:
                            color = '#90EE90'  # Light green for balanced
                            balance_text = f"B: {balance_int}"
                        else:
                            color = '#FFB6C1'  # Light pink for imbalanced
                            balance_text = f"B: {balance_int} ⚠"
                        
                        label = f"<b>{node_id}</b><br/>H: {height}<br/>{balance_text}"
                        dot.node(node_id, label=f"<{label}>", fillcolor=color)
                    
                    # Add edges with better styling
                    for parent, child in edge_list:
                        dot.edge(parent, child)
                    
                    st.graphviz_chart(dot.source)
                    
                    # Legend
                    col1, col2 = st.columns(2)
                    with col1:
                        st.markdown("**Legend:**")
                        st.markdown("🟢 **Green** = Balanced (|B| ≤ 1)")
                        st.markdown("🔴 **Pink** = Imbalanced (|B| > 1)")
                    with col2:
                        st.markdown("**Node Info:**")
                        st.markdown("**ID** = Node identifier")
                        st.markdown("**H** = Height")
                        st.markdown("**B** = Balance factor")
                
                elif root_id and NETWORKX_AVAILABLE:
                    # Fallback to NetworkX + Matplotlib with improved formatting
                    G = nx.DiGraph()
                    for parent, child in edge_list:
                        G.add_edge(parent, child)
                    
                    # Create hierarchical tree layout manually for better visualization
                    def hierarchical_layout(G, root, width=1.0, vert_gap=0.2, vert_loc=0, xcenter=0.5):
                        """Create hierarchical tree layout"""
                        def _hierarchy_pos(G, root, width=1., vert_gap=0.2, vert_loc=0, xcenter=0.5, pos=None, parent=None, parsed=[]):
                            if pos is None:
                                pos = {root: (xcenter, vert_loc)}
                            else:
                                pos[root] = (xcenter, vert_loc)
                            parsed.append(root)
                            children = list(G.successors(root))
                            if len(children) != 0:
                                dx = width / len(children)
                                nextx = xcenter - width/2 - dx/2
                                for child in children:
                                    nextx += dx
                                    pos = _hierarchy_pos(G, child, width=dx, vert_gap=vert_gap,
                                                        vert_loc=vert_loc-vert_gap, xcenter=nextx,
                                                        pos=pos, parent=root, parsed=parsed)
                            return pos
                        return _hierarchy_pos(G, root, width, vert_gap, vert_loc, xcenter)
                    
                    # Try hierarchical layout, fallback to spring if it fails
                    try:
                        pos = hierarchical_layout(G, root_id, width=10, vert_gap=0.8)
                    except:
                        pos = nx.spring_layout(G, k=2.5, iterations=150, seed=42)
                    
                    fig, ax = plt.subplots(figsize=(14, 10))
                    ax.set_facecolor('#f8f9fa')
                    fig.patch.set_facecolor('white')
                    
                    # Prepare node colors and labels
                    node_colors = []
                    node_sizes = []
                    labels = {}
                    
                    for node_id in G.nodes():
                        if node_id in node_info:
                            height, balance = node_info[node_id]
                            balance_int = int(balance)
                            
                            # Color based on balance
                            if abs(balance_int) <= 1:
                                node_colors.append('#90EE90')  # Light green
                            else:
                                node_colors.append('#FFB6C1')  # Light pink
                            
                            # Size based on height (taller nodes are slightly larger)
                            node_sizes.append(1500 + int(height) * 100)
                            
                            # Clean label format
                            balance_display = f"{balance_int}" if abs(balance_int) <= 1 else f"{balance_int} ⚠"
                            labels[node_id] = f"{node_id}\nH:{height}\nB:{balance_display}"
                        else:
                            node_colors.append('#ADD8E6')
                            node_sizes.append(1500)
                            labels[node_id] = node_id
                    
                    # Draw edges first (behind nodes)
                    nx.draw_networkx_edges(G, pos, ax=ax, edge_color='#4a5568', 
                                         width=2, alpha=0.6, arrows=True, 
                                         arrowsize=20, arrowstyle='->', 
                                         connectionstyle='arc3,rad=0.1')
                    
                    # Draw nodes
                    nx.draw_networkx_nodes(G, pos, ax=ax, node_color=node_colors,
                                          node_size=node_sizes, alpha=0.9,
                                          edgecolors='#2d3748', linewidths=2)
                    
                    # Draw labels (without node IDs since we're using custom labels)
                    nx.draw_networkx_labels(G, pos, labels, ax=ax, font_size=9,
                                          font_weight='bold', font_color='black',
                                          font_family='Arial')
                    
                    ax.axis('off')
                    plt.tight_layout()
                    
                    st.pyplot(fig)
                    
                    # Legend
                    col1, col2 = st.columns(2)
                    with col1:
                        st.markdown("**Legend:**")
                        st.markdown("🟢 **Green** = Balanced (|B| ≤ 1)")
                        st.markdown("🔴 **Pink** = Imbalanced (|B| > 1)")
                    with col2:
                        st.markdown("**Node Info:**")
                        st.markdown("**ID** = Node identifier")
                        st.markdown("**H** = Height")
                        st.markdown("**B** = Balance factor")
                    
                    plt.close(fig)
                
                else:
                    # ASCII tree representation as fallback
                    st.info("💡 **Tip:** Install graphviz for better tree visualization: `pip install graphviz` (also requires Graphviz system package)")
                    st.markdown("**Tree Structure (Edges):**")
                    tree_text = "```\n"
                    for edge_str in edges:
                        tree_text += edge_str + "\n"
                    tree_text += "```"
                    st.markdown(tree_text)
                    
                    st.markdown("**Node Details:**")
                    node_text = "```\n"
                    for node_str in nodes:
                        parts = node_str.split(':')
                        if len(parts) >= 3:
                            node_text += f"Node {parts[0]}: Height={parts[1]}, Balance={parts[2]}\n"
                    node_text += "```"
                    st.markdown(node_text)
            
            # Visualize other data structures based on operation
            operation = analysis.get('OPERATION', '')
            
            # Trie Visualization (for SEARCH and CRAM)
            if 'TRIE_EDGES' in analysis and 'TRIE_NODES' in analysis and ('SEARCH' in operation or 'CRAM' in operation):
                st.markdown("#### 🔤 Trie (Prefix Tree) Visualization")
                visualize_trie(analysis['TRIE_EDGES'], analysis['TRIE_NODES'], analysis)
            
            # MaxHeap Visualization (for SUGGEST)
            if 'HEAP_STRUCTURE' in analysis and 'SUGGEST' in operation:
                st.markdown("#### 📊 MaxHeap Visualization")
                visualize_maxheap(analysis['HEAP_STRUCTURE'], analysis)
            
            # Knowledge Graph Visualization (for PLAN)
            if 'GRAPH_EDGES' in analysis and 'GRAPH_NODES' in analysis and 'PLAN' in operation:
                st.markdown("#### 🗺️ Knowledge Graph Visualization")
                visualize_graph(analysis['GRAPH_EDGES'], analysis['GRAPH_NODES'], analysis)
            
            # LRU Cache Visualization (for SEARCH and CRAM)
            if 'CACHE_STRUCTURE' in analysis and ('SEARCH' in operation or 'CRAM' in operation):
                st.markdown("#### 💾 LRU Cache Visualization")
                visualize_cache(analysis['CACHE_STRUCTURE'], analysis)
            
            # Stack Visualization (for SEARCH and BACK)
            if 'STACK_STRUCTURE' in analysis and ('SEARCH' in operation or 'BACK' in operation):
                st.markdown("#### 📚 Stack Visualization")
                visualize_stack(analysis['STACK_STRUCTURE'], analysis)
        
        # Performance Metrics
        st.markdown("#### ⏱️ Performance Metrics")
        
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
            st.markdown("#### 🔀 Sorting Algorithm")
            algo = analysis['SORT_ALGORITHM']
            if algo == "QUICKSORT":
                st.info(f"**Algorithm:** {algo} | **Time Complexity:** O(n log n) average, O(n²) worst | **Space:** O(log n)")
            elif algo == "MERGESORT":
                st.info(f"**Algorithm:** {algo} | **Time Complexity:** O(n log n) | **Space:** O(n)")
            else:
                st.info(f"**Algorithm:** {algo}")
        
        # Data Structure Usage
        if 'DATA_STRUCTURE' in analysis:
            st.markdown("#### 📚 Data Structure Used")
            ds = analysis['DATA_STRUCTURE']
            if ds == "MAXHEAP":
                st.info(f"**Structure:** {ds} | **Time Complexity:** O(log n) insert/extract | **Space:** O(n)")
            elif ds == "KNAPSACK_DP":
                st.info(f"**Algorithm:** Dynamic Programming (0/1 Knapsack) | **Time Complexity:** O(n × W) | **Space:** O(n × W)")
        
        # Additional Metrics
        st.markdown("#### 📈 Operation Details")
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
        with st.expander("🔍 Raw Analysis Data"):
            st.json(analysis)
    
    else:
        st.info("👆 Perform an operation (Search, List, Plan, Suggest, or Cram) to see DSA analysis metrics.")
        st.markdown("""
        **What you'll see:**
        - 🌳 **AVL Tree State:** Height, node count, balance factors
        - ⏱️ **Performance Metrics:** Execution times for each operation phase
        - 🔀 **Algorithm Details:** Sorting algorithms used and their complexity
        - 📚 **Data Structure Usage:** Which structures are used and their properties
        """)