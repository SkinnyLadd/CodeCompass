#ifndef AVLTREE_H
#define AVLTREE_H

#include "Resource.h"
#include <algorithm>
#include <iostream>
#include <string>

struct AVLNode {
    Resource* data;
    AVLNode* left;
    AVLNode* right;
    int height;

    AVLNode(Resource* r) : data(r), left(nullptr), right(nullptr), height(1) {}
};

class AVLTree {
private:
    AVLNode* root;

    int height(AVLNode* n) { return n ? n->height : 0; }
    int getBalance(AVLNode* n) { return n ? height(n->left) - height(n->right) : 0; }

    AVLNode* rightRotate(AVLNode* y) {
        AVLNode* x = y->left;
        AVLNode* T2 = x->right;
        x->right = y;
        y->left = T2;
        y->height = std::max(height(y->left), height(y->right)) + 1;
        x->height = std::max(height(x->left), height(x->right)) + 1;
        return x;
    }

    AVLNode* leftRotate(AVLNode* x) {
        AVLNode* y = x->right;
        AVLNode* T2 = y->left;
        y->left = x;
        x->right = T2;
        x->height = std::max(height(x->left), height(x->right)) + 1;
        y->height = std::max(height(y->left), height(y->right)) + 1;
        return y;
    }

    AVLNode* insertNode(AVLNode* node, Resource* r) {
        if (!node) return new AVLNode(r);

        if (r->id < node->data->id)
            node->left = insertNode(node->left, r);
        else if (r->id > node->data->id)
            node->right = insertNode(node->right, r);
        else
            return node;

        node->height = 1 + std::max(height(node->left), height(node->right));
        int balance = getBalance(node);

        if (balance > 1 && r->id < node->left->data->id) return rightRotate(node);
        if (balance < -1 && r->id > node->right->data->id) return leftRotate(node);
        if (balance > 1 && r->id > node->left->data->id) {
            node->left = leftRotate(node->left);
            return rightRotate(node);
        }
        if (balance < -1 && r->id < node->right->data->id) {
            node->right = rightRotate(node->right);
            return leftRotate(node);
        }
        return node;
    }

    Resource* searchRec(AVLNode* node, int id) {
        if (!node) return nullptr;
        if (id == node->data->id) return node->data;
        if (id < node->data->id) return searchRec(node->left, id);
        return searchRec(node->right, id);
    }
void inorderRec(AVLNode* node, std::vector<Resource*>& result) {
        if (!node) return;
        inorderRec(node->left, result);  // Visit Left
        result.push_back(node->data);    // Visit Node
        inorderRec(node->right, result); // Visit Right
    }

    // --- VISUALIZATION HELPER ---
    // Prints:
    // 104
    // |-- 102
    // `-- 106
    void printStructure(AVLNode* node, std::string prefix, bool isLeft) {
        if (!node) return;

        std::cout << prefix;
        std::cout << (isLeft ? "|-- " : "`-- ");
        std::cout << "[" << node->data->id << "] " << node->data->title << std::endl;

        // Enter next level
        printStructure(node->left, prefix + (isLeft ? "|   " : "    "), true);
        printStructure(node->right, prefix + (isLeft ? "|   " : "    "), false);
    }

public:
    AVLTree() : root(nullptr) {}

    void insert(Resource* r) {
        root = insertNode(root, r);
    }

    Resource* search(int id) {
        return searchRec(root, id);
    }
 // [ADDED] Public function to get sorted list
    std::vector<Resource*> inorderTraversal() {
        std::vector<Resource*> result;
        inorderRec(root, result);
        return result;
    }

    // --- PUBLIC PRINT FUNCTION ---
    void printTreeState() {
        if (!root) {
            std::cout << "(Tree is empty)" << std::endl;
        } else {
            // Pass isLeft=false for root to use the "`--" style or just handle root manually
            // Here we just start recursion
            printStructure(root, "", false);
        }
    }
};

#endif
