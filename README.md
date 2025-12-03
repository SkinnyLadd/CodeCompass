# CodeCompass 🧭

## Cache-Efficient DSA Study Resource Recommendation System

CodeCompass is a high-performance resource recommendation engine designed to help students navigate Data Structures and Algorithms. It utilizes a Hybrid C++/Python architecture to demonstrate manual memory management and advanced algorithmic efficiency.

---

## 🚀 Key Features (DSA Concepts)

This project implements the following concepts from scratch (no STL containers for core logic):

- **Memory Management:** Custom Pointers and Dynamic Memory Allocation throughout.
- **Caching:** An LRU Cache (HashMap + Doubly Linked List) to optimize repeated queries.
- **Indexing:** An AVL Tree (Self-Balancing BST) for $O(\log n)$ resource lookups by ID.
- **Search:** A Trie (Prefix Tree) for instant topic autocomplete.
- **Recommendations:** A Knowledge Graph using BFS and Dijkstra's Algorithm to calculate optimal learning paths between topics.
- **Filtering:** A Max-Heap (Priority Queue) to fetch highest-rated resources.
- **Sorting:** Implementation of QuickSort and MergeSort to rank results by difficulty.
- **History:** A Stack implementation to track user navigation history.

---

## 🛠️ Tech Stack

- **Core Logic:** C++ (Performance, Pointers, Memory Management)
- **Build System:** CMake (for CLion integration) or g++
- **User Interface:** Python Streamlit (Visuals, Interactive Tables)
- **Communication:** Standard I/O Pipes (IPC)
- **Data:** Custom Curated CSV Dataset (Simulated Database)

---

## 📂 Project Structure

```
CodeCompass/
├── data/
│   └── resources.csv      # The knowledge base
├── include/
│   ├── AVLTree.h          # Database Storage
│   ├── DoublyLinkedList.h # Custom List for Cache
│   ├── KnowledgeGraph.h   # Dependency Mapping & Dijkstra
│   ├── LRUCache.h         # Optimization Layer
│   ├── MaxHeap.h          # Priority Queue
│   ├── Resource.h         # Master Data Object (stays in sync with CSV)
│   ├── Sorters.h          # Sorting Algorithms
│   ├── Stack.h            # History Tracking
│   └── Trie.h             # Search Index
├── src/
│   ├── CSVParser.cpp      # File Reader Logic
│   └── main.cpp           # C++ Controller Loop (The Engine)
├── tests/                 # Sandbox for Modular Testing
│   ├── test_algo.cpp      # Tests for Sorting/Parsing
│   ├── test_graph.cpp     # Tests for Graph/Heap
│   ├── test_linear.cpp    # Tests for Cache/Linked List
│   └── test_tree.cpp      # Tests for AVL/Trie
├── .gitignore
├── app.py                 # Python UI Wrapper
├── CMakeLists.txt
└── README.md

```

---

## ⚙️ CLion Configuration (Important!)

### Fix File Loading Issues (Working Directory)

By default, CLion runs executables inside the `cmake-build-debug` folder. This breaks relative paths (e.g., the code cannot find `data/resources.csv`).

**Step 1:** Go to **Run → Edit Configurations...**  
**Step 2:** Select your target (e.g., `codecompass_engine` or `test_algo`).  
**Step 3:** Change the **Working directory** field to your **Project Root** folder (the folder containing `app.py`).  
**Step 4:** Click **Apply**.

---

## Implementation Guide



##  Linear Data Structures
**Files:** `DoublyLinkedList.h`, `Stack.h`, `LRUCache.h`

### TODO:
> Create a DoublyLinkedList class in C++ using raw pointers (Node* prev, Node* next). It needs a `moveToHead(Node* node)` function and `removeTail()` function.
>
> Create a Stack class for navigation history using a Linked List approach.
>
> Create an LRUCache class that uses `std::unordered_map<int, Node*>` and your DoublyLinkedList. It should have `get(int id)` and `put(int id, Resource* res)`. When capacity is full, evict the tail.

---

## Tree Data Structures
**Files:** `AVLTree.h`, `Trie.h`

### TODO:
> Create a Trie class for autocomplete. The TrieNode should have `TrieNode* children[26]` and a `vector<string>` topics. Implement `insert(string keyword)` and `vector<string> getSuggestions(string prefix)`.
>
> Create an AVLTree class to store `Resource*` objects ordered by ID. Implement standard BST insertion with AVL Rotations (LL, RR, LR, RL) to keep it balanced. No STL sets/maps allowed for the tree logic.

---

## Graph & Heaps
**Files:** `KnowledgeGraph.h`, `MaxHeap.h`

### TODO:
> Create a MaxHeap class (Priority Queue) that stores `Resource*` pointers. The heapify logic should sort based on `Resource->rating` (double).
>
> Create a KnowledgeGraph class using an Adjacency List (`map<int, vector<pair<int, int>>>`).
>
> Implement `dijkstra(int startID, int endID)` to find the shortest path between topics based on edge weights. Also implement a simple BFS traversal.

---

## Algorithms & Integration
**Files:** `Sorters.h`, `CSVParser.cpp`

### TODO:
> Create a Sorters namespace. Implement `quickSort(vector<Resource*>& res)` that sorts resources by difficulty (int). Implement `mergeSort` that sorts by title (string).
>
> Create a CSVParser that reads a file line-by-line. Split strings by commas (and handle semicolons for prerequisite lists). Convert parsed data into new `Resource()` objects.

---

## Team & Task Division

- **Member 1 (Linear Specialist):** LRU Cache, Doubly Linked List, Stack.
- **Member 2 (Tree Specialist):** AVL Tree, Trie (Prefix Search).
- **Member 3 (Graph Specialist):** Knowledge Graph, BFS, Dijkstra, Max-Heap.
- **Member 4 (Integrator):** Sorting Algorithms, CSV Parsing, Main Controller, Python UI.
