// DUMMY TEST CODE

#include "../include/Trie.h"
#include <iostream>
#include <vector>

int main() {
    Trie trie;

    // 1. Insert Topics
    trie.insert("binary search");
    trie.insert("binary tree");
    trie.insert("bfs");

    // 2. Test Auto-Complete
    std::cout << "Searching prefix 'bin'...\n";
    std::vector<std::string> results = trie.getSuggestions("bin");

    for(const auto& s : results) {
        std::cout << "Found: " << s << "\n";
    }

    // Expected: binary search, binary tree
    return 0;
}
