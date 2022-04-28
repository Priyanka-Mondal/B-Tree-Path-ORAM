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
    BTreeNode* newBTreeNode(bool leaf, int nextBid, int pos);
    int RandomPath();
    int nextBid();
    int nextbid = 0;
    //vector<BTreeNode*> setupBTreeNodes;

public:
    int brootKey;
    int brootPos;
    int searchi_bytes;
    int insert(Bid kw, int id);
    int insertkw(Bid kw, int id, int rootBid, int &rootPos);
    void insertNFull(Bid kw,int id, BTreeNode*& bt ); 
    void splitChild(BTreeNode *&par, int i, BTreeNode *&y, BTreeNode *&z);
    int search(Bid kw);
    void searchkw(int brootKey, int brootPos, Bid kw, int &res);
   

    void deletion(Bid k, BTreeNode *&node);
    void remove(Bid);
    void removeFromNonLeaf(int, BTreeNode*&);
    Bid getPredecessor(int, BTreeNode*);
    Bid getSuccessor(int, BTreeNode*);
    void fill(int, BTreeNode*&);
    void borrowFromPrev(int, BTreeNode*&);
    void borrowFromNext(int, BTreeNode*&);
    void merge(int, BTreeNode*&);

    BTree(int maxSize, bytes<Key> key);
    BTree();
   ~BTree();
    
    void setupInsert(int& rootKey, int& rootPos, map<int, string> pairs);
    int sortedArrayToBST(int start, int end, int& pos, int& node);
};

#endif /* BTREE_H */




