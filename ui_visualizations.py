"""
Visualization Module
Handles all data structure visualizations using NetworkX and Matplotlib
"""

import streamlit as st

try:
    import networkx as nx
    import matplotlib.pyplot as plt
    import matplotlib
    matplotlib.use('Agg')
    NETWORKX_AVAILABLE = True
except ImportError:
    NETWORKX_AVAILABLE = False


def visualize_avltree(edges, nodes, analysis):
    """Visualize AVL Tree structure"""
    if not edges or not nodes:
        st.info("Tree structure is empty.")
        return
    
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
    
    if root_id and NETWORKX_AVAILABLE:
        # Use NetworkX + Matplotlib for visualization
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
        
        # Draw labels
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
        st.info("💡 **Tip:** Install networkx for tree visualization: `pip install networkx matplotlib`")
        if root_id:
            st.code("\n".join(edges[:30]))


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
    
    if NETWORKX_AVAILABLE:
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
        st.info("Install networkx for graph visualization: `pip install networkx matplotlib`")
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

