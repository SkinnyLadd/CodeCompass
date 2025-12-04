cmake_minimum_required(VERSION 3.17)
project(CodeCompass)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# 1. Tell the compiler where to look for headers
include_directories(include)

# 2. Explicitly list headers so CLion recognizes them as project files
set(HEADERS
        include/Resource.h
        include/AVLTree.h
        include/LRUCache.h
        include/Trie.h
        include/KnowledgeGraph.h
        include/MaxHeap.h
        include/Stack.h
        include/Sorters.h
        include/DoublyLinkedList.h
        include/CSVParser.h
)

# =========================================================
# MAIN ENGINE
# =========================================================
add_executable(codecompass_engine
        src/main.cpp
        src/CSVParser.cpp
        ${HEADERS}
)

# =========================================================
# MODULAR TESTS (Sandbox Mode)
# =========================================================
# These create separate run configurations in CLion.

# Member 1: Linear Structures
if(EXISTS "${CMAKE_SOURCE_DIR}/tests/test_linear.cpp")
    add_executable(test_linear tests/test_linear.cpp ${HEADERS})
endif()

# Member 2: Tree Structures
if(EXISTS "${CMAKE_SOURCE_DIR}/tests/test_tree.cpp")
    add_executable(test_tree tests/test_tree.cpp ${HEADERS})
endif()

# Member 3: Graph Structures
if(EXISTS "${CMAKE_SOURCE_DIR}/tests/test_graph.cpp")
    add_executable(test_graph tests/test_graph.cpp ${HEADERS})
endif()

# Member 4: Algorithms
if(EXISTS "${CMAKE_SOURCE_DIR}/tests/test_algo.cpp")
    add_executable(test_algo tests/test_algo.cpp ${HEADERS})
endif()

if(EXISTS "${CMAKE_SOURCE_DIR}/tests/test_parser.cpp")
    add_executable(test_parser tests/test_parser.cpp src/CSVParser.cpp ${HEADERS})
endif()