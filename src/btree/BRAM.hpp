#pragma once
#ifndef BRAM_H
#define BRAM_H

#include "AES.hpp"
#include <random>
#include <vector>
#include <unordered_map>
#include <string>
#include <iostream>
#include "RAMStore.hpp"
#include "BTreeNode.h"
#include <map>
#include <set>
#include <bits/stdc++.h>

using namespace std;


struct Blockb 
{
    int id;
    block data;
};

using Bucketb = std::array<Blockb, Z>;

class BRAM {
private:
    RAMStore* store;
    size_t depth;
    size_t blockSize;
    map<int, BTreeNode*> cache;
    vector<int> leafList;
    vector<int> readviewmap;
    vector<int> writeviewmap;
    set<int> modified;
    int readCnt = 0;
    bytes<Key> key;

    // Randomness
    std::random_device rd;
    std::mt19937 mt;
    std::uniform_int_distribution<int> dis;

    int RandomPath();
    int GetBTreeNodeOnPath(int leaf, int depth);
    vector<int> GetIntersectingBlockbs(int leaf, int depth);

    void FetchPath(int leaf);
    void WritePath(int leaf, int level);

    BTreeNode* ReadData(int bid);
    void WriteData(int bid, BTreeNode* b);
    void DeleteData(int bid, BTreeNode* b);

    block SerialiseBucketb(Bucketb bucket);
    Bucketb DeserialiseBucketb(block buffer);

    Bucketb ReadBucketb(int pos);
    void WriteBucketb(int pos, Bucketb bucket);
    void Access(int bid, BTreeNode*& node, int lastLeaf, int newLeaf);
    void Access(int bid, BTreeNode*& node);


    size_t plaintext_size;
    size_t bucketCount;
    size_t clen_size;
    bool batchWrite = false;

    bool WasSerialised();
    void Print();

public:
    BRAM(int maxSize, bytes<Key> key);
    ~BRAM();
    int maxheight;
    BTreeNode* ReadBTreeNode(int bid, int lastLeaf, int newLeaf);
    BTreeNode* ReadBTreeNode(int bid);
    int WriteBTreeNode(int bid, BTreeNode* n);
    int DeleteBTreeNode(int bid, BTreeNode* n);
    void start(bool batchWrite);
    void finalize(int& rootKey, int& rootPos);
    static BTreeNode* convertBlockbToBTreeNode(block b);
    void convertBlockbToBTreeNode(BTreeNode*& node,block b);
    static block convertBTreeNodeToBlockb(BTreeNode* node);

    //void setupInsert(vector<BTreeNode*> nodes);
};

#endif
