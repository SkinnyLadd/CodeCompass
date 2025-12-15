# CodeCompass: Cache-Efficient DSA Study Resource Recommendation System

CodeCompass is a high-performance resource recommendation engine designed to facilitate the study of Data Structures and Algorithms. The system utilizes a hybrid architecture, combining a C++ backend for core logic with a Python Streamlit frontend for visualization. The primary engineering goal was to demonstrate manual memory management, algorithmic efficiency, and cross-language integration by implementing all core data structures from scratch.

## System Features

### 1. Prefix Search & Library Management

The system provides instant prefix-based search for topics.

* **Implementation:** A custom Trie (Prefix Tree) is used for filtering string keys in O(L) time.

* **Optimization:** An LRU Cache (Hash Map combined with a Doubly Linked List) buffers recently accessed resources to provide O(1) retrieval for frequent queries.

* **Sorting:** The library includes custom implementations of QuickSort (sorting by difficulty) and MergeSort (sorting by title).

### 2. Curriculum Planner

This feature generates a valid, step-by-step learning path for a specific target topic.

* **Implementation:** Topics are modeled as nodes in a Knowledge Graph (Adjacency List).

* **Algorithm:** A Topological Sort algorithm resolves dependencies to ensure prerequisite concepts are scheduled before advanced topics.

### 3. Exam Cram Optimizer

A time-management tool that selects the optimal set of resources to maximize learning value within a user-defined time constraint.

* **Algorithm:** This solves the 0/1 Knapsack Problem using a Dynamic Programming approach to maximize the total rating score without exceeding the time limit.

### 4. Smart Recommendations

The system fetches the highest-rated resources from the database, capable of filtering by difficulty level.

* **Implementation:** A Max-Heap (Priority Queue) is used to efficiently manage and extract the top-k highest-rated items.

## Technical Architecture

* **Core Engine:** C++ (Implements custom data structures using raw pointers and manual memory management).

* **Frontend:** Python Streamlit (Handles user input and data visualization).

* **Communication:** The frontend communicates with the backend via stateless CLI execution using standard I/O pipes.

* **Build System:** CMake and MinGW g++.

## Build and Run Instructions

### Prerequisites

* **C++ Compiler:** MinGW (Windows) or G++ (Linux/Mac).

* **Python:** Python 3.8+ with the `streamlit` and `pandas` libraries installed.

### Option 1: Automatic Execution

Run the Python UI directly. The script includes logic to detect the environment and compile the C++ engine automatically if the executable is missing.

```
streamlit run ui.py
```

### Option 2: Manual Compilation

If the automatic build fails, compile the backend manually using the following commands. Note that static linking is used to prevent DLL dependency errors on Windows.

**Windows (PowerShell):**
```
g++ src/main.cpp src/CSVParser.cpp src/Engine.cpp -I include -static -o codecompass_engine.exe
```

**Linux/Mac:**
```
g++ src/main.cpp src/CSVParser.cpp src/Engine.cpp -I include -o codecompass_engine
```

Once compiled, launch the interface:

```
streamlit run ui.py
```

## Team Responsibilities

* **Rudaina (Member 1): Linear Data Structures**

    * Implementation of Doubly Linked List and Stack classes.

    * Development of the LRU Cache logic.

* **Hamza (Member 2): Tree Data Structures**

    * Implementation of the AVL Tree (Self-Balancing BST) for database storage.

    * Development of the Trie for search indexing.

* **Ahsan (Member 3): Backend Architecture & Algorithms**

    * Implementation of the Knowledge Graph and Topological Sort.

    * Development of the Dynamic Programming Optimizer.

    * Integration of all modules into the main Engine class.

* **Abdullah (Member 4): Frontend & Visualization**

    * Development of the Python Streamlit interface.

    * Implementation of the dark mode theme and interactive data tables.