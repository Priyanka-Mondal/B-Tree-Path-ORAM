#ifndef AVLTREE_H
#define AVLTREE_H
#include <iostream>
#include <functional>
#include <fstream>
#include <cstdio>
#include <cstring>
#include <iostream>
#include <array>
#include <memory>
#include <type_traits>
#include <iomanip>
#include <bits/stdc++.h>
#include "ORAM.hpp"
#include "Bid.h"
#include <random>
using namespace std;

class AVLTree {
private:
    ORAM *oram;
    std::random_device rd;
    std::mt19937 mt;
    std::uniform_int_distribution<int> dis;
    int totalleaves;
    int setupleaf= -1;

    int height(Bid N, int& leaf);
    int max(int a, int b);
    Node* newNode(Bid key, string value);
    Node* setupnewNode(Bid key, string value);
    Node* rightRotate(Node* y);
    Node* leftRotate(Node* x);
    int getBalance(Node* N);
    int RandomPath();
    int notsoRandomPath();
    int setupheight(Bid N, int& leaf);
    Node* setuprightRotate(Node* y, Bid rootKey, int& pos);
    Node* setupleftRotate(Node* x, Bid rootKey, int& pos);
    int setupgetBalance(Node* N);

public:
    AVLTree(int maxSize, bytes<Key> key);
    virtual ~AVLTree();
    Bid insert(Bid rootKey, int& pos, Bid key, string value);
    Node* search(Node* head, Bid key);
    void batchSearch(Node* head, vector<Bid> keys, vector<Node*>* results);
    void printTree(Node* root, int indent);
    void startOperation(bool batchWrite = false);
    void finishOperation(bool find, Bid& rootKey, int& rootPos);
    
    //deletion related functions
    int deleteNode(Node* nodef);
    Node* parentOf(Bid parentKey, int ppos, Bid childKey, int cpos, Bid key);
    Node* minValueNode(Bid rootKey, int pos, Node* rootroot);
    Bid balance(Node* node, int &pos);
    Bid balanceDel(Bid minKey, int& minpos, Node* parmin);
    Bid removeMain(Bid rootKey,int& pos, Bid delKey);
    Bid removeDel(Bid rootKey,int& pos,Bid delKey,int delPos,Node* paren);
    Bid removeRoot(Bid rootKey, int& pos);
    Bid realDelete(Node* paren,Bid delKey,int delPos);


    Node* setupsearch(Node* head, Bid key);
    Bid setupinsert(Bid rootKey, int& pos, Bid key, string value);
};

#endif /* AVLTREE_H */




