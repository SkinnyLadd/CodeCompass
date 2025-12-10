import streamlit as st
import subprocess
import os
import google.generativeai as genai
import json

# ==========================================
# CONFIGURATION & SETUP
# ==========================================
st.set_page_config(page_title="CodeCompass", page_icon="🧭")

# 1. API Setup (Handles environment variable or manual entry)
api_key = os.getenv("AIzaSyDx--zM7sj7aviNB4Z4uKjhaYSkb-9bEEw")
if not api_key:
    # Fallback: Allow user to enter key in sidebar if not in env
    api_key = st.sidebar.text_input("Enter Gemini API Key", type="password")

if api_key:
    genai.configure(api_key=api_key)
    model = genai.GenerativeModel('gemini-2.5-flash')

# 2. Path to C++ Executable (Auto-detects Windows/Mac/Linux)
EXE_NAME = "codecompass_engine"
if os.name == 'nt':
    EXE_NAME += ".exe"

# Look for exe in current folder OR cmake-build-debug
PATHS_TO_CHECK = [
    os.path.join(os.getcwd(), EXE_NAME),
    os.path.join(os.getcwd(), "cmake-build-debug", EXE_NAME)
]

FINAL_EXE_PATH = None
for path in PATHS_TO_CHECK:
    if os.path.exists(path):
        FINAL_EXE_PATH = path
        break

# ==========================================
# BACKEND LOGIC
# ==========================================

def get_gemini_token(user_query):
    """
    Asks AI to translate natural language -> JSON
    """
    if not api_key:
        return None

    prompt = f"""
    Translate this search query into a JSON command for a C++ Engine.
    
    Commands:
    1. SEARCH: {{ "action": "search", "value": "term" }}
    2. FILTER: {{ "action": "filter", "key": "difficulty|rating", "min": 0, "max": 100 }}
       - 'Easy' = diff 1-30. 'Hard' = diff 70-100. 'Best' = rating 4.5-5.0.
    3. COUNT:  {{ "action": "count" }}
    
    User Query: "{user_query}"
    JSON:
    """
    try:
        response = model.generate_content(prompt)
        # Clean up the response to get pure JSON
        token = response.text.replace("```json", "").replace("```", "").strip()
        return token
    except Exception as e:
        st.error(f"AI Error: {e}")
        return None

def run_cpp_backend(token):
    """
    Runs the C++ executable and captures output
    """
    if not FINAL_EXE_PATH:
        return "CRITICAL ERROR: C++ Executable not found. Please build the project in CLion first."

    try:
        result = subprocess.run(
            [FINAL_EXE_PATH, token],
            capture_output=True,
            text=True
        )
        return result.stdout
    except Exception as e:
        return f"System Error: {e}"

# ==========================================
# FRONTEND UI (Streamlit)
# ==========================================

st.title("🧭 CodeCompass AI")
st.markdown("Ask natural questions like *'Show me easy python courses'* or *'Find best rated algorithms'*.")

# Search Bar
query = st.text_input("What do you want to learn?", placeholder="e.g. Data Structures in C++...")

if st.button("Search") or query:
    if not query:
        st.warning("Please enter a query.")
    else:
        # --- STEP 1: PARSING ---
        st.info("🧠 Parsing with Gemini...")

        # 1. Get Token
        token = get_gemini_token(query)

        # LOG TO CONSOLE
        print("\n" + "="*40)
        print(f" [Frontend] User Query: {query}")
        print(f" [Gemini]   Generated Token: {token}")
        print("="*40)

        if token:
            # Show debug info in UI (Optional, cool for demo)
            with st.expander("See how the AI understood you"):
                st.code(token, language="json")

            # --- STEP 2: EXECUTION ---
            st.info("⚙️ Running C++ Engine...")

            # 2. Run C++
            backend_response = run_cpp_backend(token)

            # LOG TO CONSOLE
            print(f" [Backend]  Raw Output:\n{backend_response}")
            print("="*40 + "\n")

            # --- STEP 3: DISPLAY RESULTS ---
            st.success("Results Found:")

            # Display the raw C++ output nicely
            st.text(backend_response)

            # (Optional) If you wanted to make it prettier, you could parse
            # the C++ text output here, but raw text works for now.
        else:
            st.error("Could not generate a valid token. Check your API Key.")

# Footer
st.markdown("---")
st.caption(f"Backend Status: {'🟢 Connected' if FINAL_EXE_PATH else '🔴 Disconnected (Build C++ Project!)'}")
if FINAL_EXE_PATH:
    st.caption(f"Path: `{FINAL_EXE_PATH}`")