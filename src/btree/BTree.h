#pragma once
#ifndef BTREE_H
#define BTREE_H
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
#include "BRAM.hpp"

using namespace std;
class BTree 
{
private:
    std::random_device rd;
    std::mt19937 mt;
    std::uniform_int_distribution<int> dis;
    int totleaves;
    int setupleaf= -1;
    int setupProgress = 0;

    BRAM *bram;
    int max(int a, int b);
    //BTreeNode* newBTreeNode(bool leaf);
    int RandomPath();
    int nextBid();
    int nextbid = 0;
    //vector<BTreeNode*> setupBTreeNodes;

public:
    int brootKey;
    int brootPos;
    int insert(string kw);
    int insertkw(string kw, int rootBid, int &rootPos);
    void insertNFull(string kw, BTreeNode*& bt ); 
    BTree(int maxSize, bytes<Key> key);
    BTree();
    virtual ~BTree();
    void startOperation(bool batchWrite = false);
    void finishOperation(bool find, int& rootKey, int& rootPos);
    
    void setupInsert(int& rootKey, int& rootPos, map<int, string> pairs);
    int sortedArrayToBST(int start, int end, int& pos, int& node);
};

#endif /* AVLTREE_H */




