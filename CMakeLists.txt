cmake_minimum_required(VERSION 3.17)
project(CodeCompass)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
if(WIN32)
    set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -static -static-libgcc -static-libstdc++")
endif()

# =========================================================
# CRITICAL FIX: STATIC LINKING FOR PORTABILITY
# =========================================================
# This ensures the .exe runs without needing MinGW DLLs
# (libstdc++, libgcc, libwinpthread) in the system PATH.
if(WIN32)
    set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -static -static-libgcc -static-libstdc++")
endif()

# 1. Headers
include_directories(include)

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
        include/Engine.h
        include/Optimizer.h
)

# =========================================================
# MAIN ENGINE
# =========================================================
add_executable(codecompass_engine
        src/main.cpp
        src/Engine.cpp
        src/CSVParser.cpp
        ${HEADERS}
)

# =========================================================
# MODULAR TESTS
# =========================================================
if(EXISTS "${CMAKE_SOURCE_DIR}/tests/test_linear.cpp")
    add_executable(test_linear tests/test_linear.cpp ${HEADERS})
endif()

if(EXISTS "${CMAKE_SOURCE_DIR}/tests/test_tree.cpp")
    add_executable(test_tree tests/test_tree.cpp ${HEADERS})
endif()

if(EXISTS "${CMAKE_SOURCE_DIR}/tests/test_graph.cpp")
    add_executable(test_graph tests/test_graph.cpp ${HEADERS})
endif()

if(EXISTS "${CMAKE_SOURCE_DIR}/tests/test_algo.cpp")
    add_executable(test_algo tests/test_algo.cpp src/CSVParser.cpp ${HEADERS})
endif()

if(EXISTS "${CMAKE_SOURCE_DIR}/tests/test_parser.cpp")
    add_executable(test_parser tests/test_parser.cpp src/CSVParser.cpp ${HEADERS})
endif()

if(EXISTS "${CMAKE_SOURCE_DIR}/tests/test_optimizer.cpp")
    add_executable(test_optimizer tests/test_optimizer.cpp src/CSVParser.cpp ${HEADERS})
endif()

# =========================================================
# COPY DATA FOLDER AUTOMATICALLY AFTER BUILD
# =========================================================
add_custom_command(TARGET codecompass_engine POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_directory
        ${CMAKE_SOURCE_DIR}/data
        ${CMAKE_BINARY_DIR}/data
)
