#pragma once
#ifndef BTREEF_H
#define BTREEF_H
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
#include "BRAMf.hpp"

using namespace std;
class BTreef 
{
private:
    std::random_device rd;
    std::mt19937 mt;
    std::uniform_int_distribution<int> dis;
    int totleaves;
    int setupleaf= -1;
    int setupProgress = 0;

    BRAMf *bram;
    int max(int a, int b);
    //BTreeNodef* newBTreeNodef(bool leaf);
    BTreeNodef* newBTreeNodef(bool leaf, int nextBid, int pos);
    int RandomPath();
    int nextBid();
    int nextbid = 0;
    //vector<BTreeNodef*> setupBTreeNodefs;

public:
    int brootKey;
    int brootPos;
    int searchf_bytes;
    int insert(Bid kw, string blk);
    int insertblk(Bid kw, string blk, int rootBid, int &rootPos);
    void insertNFull(Bid kw,string blk, BTreeNodef*& bt ); 
    void splitChild(BTreeNodef *&par, int i, BTreeNodef *&y, BTreeNodef *&z);
    string search(Bid kw);
    void searchkw(int brootKey, int brootPos, Bid kw, string &res);
   

    void deletion(Bid k, BTreeNodef *&node);
    void remove(Bid);
    void removeFromNonLeaf(int, BTreeNodef*&);
    Bid getPredecessor(int, BTreeNodef*);
    Bid getSuccessor(int, BTreeNodef*);
    void fill(int, BTreeNodef*&);
    void borrowFromPrev(int, BTreeNodef*&);
    void borrowFromNext(int, BTreeNodef*&);
    void merge(int, BTreeNodef*&);

    BTreef(int maxSize, bytes<Key> key);
    BTreef();
   ~BTreef();
    
    void setupInsert(int& rootKey, int& rootPos, map<int, string> pairs);
    int sortedArrayToBST(int start, int end, int& pos, int& node);
};

#endif /* BTREE_H */




