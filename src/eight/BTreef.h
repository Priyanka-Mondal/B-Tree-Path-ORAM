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
    BTreeNodef* newBTreeNodef(int nextBid, int pos);
    int RandomPath();
    int nextBid();
    int nextbid = 0;
    map<int,BTreeNodef*> setupNodes;
    vector<BTreeNodef*> sn;
    int minHeight;

public:
    int brootKey;
    int brootPos;
    bool isleaf(BTreeNodef* node);
    int keynum(BTreeNodef* node);
    int rtt;
    int searchf_bytes;
    int getMaxNodes(int h);

    void insert(Bid kw, string blk);
    int insertblk(Bid kw, string blk, int rootBid, int &rootPos);
    void insertNFull(Bid kw,string blk, BTreeNodef*& bt ); 
    void splitChild(BTreeNodef *&par, int i, BTreeNodef *&y, BTreeNodef *&z);
    string search(Bid kw);
    pair<int,int> searchkw(int brootKey, int brootPos, BTreeNodef* node,Bid kw, string& res, int mh);
    vector<string> batchSearch(vector<Bid> bids);
   

    void deletion(Bid k, BTreeNodef *&node, int mh);
    void remove(Bid);
    int findKey(Bid k, BTreeNodef* node);
    void removekw(Bid);
    void removeFromLeaf(int idx, BTreeNodef *&node) ;
    void removeFromNonLeaf(int, BTreeNodef*&, int mh);
    pair<Bid,array<byte_t,BLOCK>> getPredecessor(int, BTreeNodef*, int mh);
    pair<Bid,array<byte_t,BLOCK>> getSuccessor(int, BTreeNodef*, int mh);
    void fill(int, BTreeNodef*&, int mh);
    void borrowFromPrev(int, BTreeNodef*&, int mh);
    void borrowFromNext(int, BTreeNodef*&, int mh);
    void merge(int, BTreeNodef*&, int mh);

    BTreef(int maxSize, bytes<Key> key);
    BTreef();
   ~BTreef();
    
    void setupInsert(map<Bid,string> pairs);
    int createBTreeNodef(int nextbid, int &leafpos, map<Bid,string> input, int maxHeight);
    void endSetup();
    //void setupInsert(int& rootKey, int& rootPos, map<int, string> pairs);
    int sortedArrayToBST(int start, int end, int& pos, int& node);
};

#endif /* BTREEF_H */



