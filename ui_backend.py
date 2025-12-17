"""
Backend Connection Module
Handles communication with C++ backend and data parsing
"""

import streamlit as st
import subprocess
import os


def get_exe():
    """Find the compiled C++ executable"""
    name = "codecompass_engine.exe" if os.name == 'nt' else "codecompass_engine"
    paths = [name, os.path.join("cmake-build-debug", name), os.path.join("..", "cmake-build-debug", name)]
    for p in paths:
        if os.path.exists(p):
            return os.path.abspath(p)
    return None

def run_cpp(cmd):
    """Execute C++ backend command and return output lines"""
    exe_path = get_exe()
    if not exe_path:
        return []
    
    try:
        # Initialize navigation history if it doesn't exist
        if 'navigation_history' not in st.session_state:
            st.session_state.navigation_history = []
            
        # Handle BACK command
        if cmd.strip().upper() == "BACK":
            if not st.session_state.navigation_history:
                return []
                
            # Remove current from history
            st.session_state.navigation_history.pop()
            
            # If there's a previous state, return it
            if st.session_state.navigation_history:
                prev_data = st.session_state.navigation_history[-1]
                return [f"ID,Title,URL,Topic,Difficulty,Rating,Duration",
                       f"{prev_data['resource'].get('id')},{prev_data['resource'].get('title')}," +
                       f"{prev_data['resource'].get('url')},{prev_data['resource'].get('topic')}," +
                       f"{prev_data['resource'].get('difficulty')}," +
                       f"{prev_data['resource'].get('rating')}," +
                       f"{prev_data['resource'].get('duration')}"]
            return []

        # Rest of the function remains the same
        si = None
        if os.name == 'nt':
            si = subprocess.STARTUPINFO()
            si.dwFlags |= subprocess.STARTF_USESHOWWINDOW

        full_cmd = [exe_path, cmd]
        res = subprocess.run(full_cmd, capture_output=True, text=True, startupinfo=si)

        if res.returncode != 0 or not res.stdout.strip():
            return []

        return res.stdout.strip().splitlines()
    except Exception as e:
        st.error(f"Backend Connection Error: {e}")
        return []

def parse_csv_lines(lines):
    """Parse CSV formatted lines from backend output"""
    if not lines:
        return []
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
        if not line.strip():
            continue
        if line.startswith("---") or line.startswith("[CPP]"):
            continue
        parts = line.split(',')
        if len(parts) < len(headers):
            continue
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

