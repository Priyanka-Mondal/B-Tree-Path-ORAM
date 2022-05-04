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
    BTreeNode* newBTreeNode(int nextBid, int pos);
    int RandomPath();
    int nextBid();
    int nextbid = 0;
    map<int,BTreeNode*> setupNodes;
    vector<BTreeNode*> sn;
    int minHeight;

public:
    int brootKey;
    int brootPos;
    bool isleaf(BTreeNode* node);
    int keynum(BTreeNode* node);

    void insert(Bid kw, int id);
    int insertkw(Bid kw, int id, int rootBid, int &rootPos);
    void insertNFull(Bid kw,int id, BTreeNode*& bt ); 
    void splitChild(BTreeNode *&par, int i, BTreeNode *&y, BTreeNode *&z);
    int search(Bid kw);
    void searchkw(int brootKey, int brootPos, Bid kw, int &res, int &mh);
    vector<int> batchSearch(vector<Bid> bids);
   

    void deletion(Bid k, BTreeNode *&node, int mh);
    void remove(Bid);
    int findKey(Bid k, BTreeNode* node);
    void removekw(Bid);
    void removeFromLeaf(int idx, BTreeNode *&node) ;
    void removeFromNonLeaf(int, BTreeNode*&, int mh);
    pair<Bid,int> getPredecessor(int, BTreeNode*, int mh);
    pair<Bid,int> getSuccessor(int, BTreeNode*, int mh);
    void fill(int, BTreeNode*&, int mh);
    void borrowFromPrev(int, BTreeNode*&, int mh);
    void borrowFromNext(int, BTreeNode*&, int mh);
    void merge(int, BTreeNode*&, int mh);

    BTree(int maxSize, bytes<Key> key);
    BTree();
   ~BTree();
    
    void setupInsert(map<Bid,int> pairs);
    int create_node(int nextbid, int &leafpos, map<Bid,int> input, int maxHeight);
    void endSetup();
    //void setupInsert(int& rootKey, int& rootPos, map<int, string> pairs);
    int sortedArrayToBST(int start, int end, int& pos, int& node);
};

#endif /* BTREE_H */




