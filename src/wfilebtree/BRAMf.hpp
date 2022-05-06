#pragma once
#ifndef BRAMF_H
#define BRAMF_H

#include "AES.hpp"
#include <random>
#include <vector>
#include <unordered_map>
#include <string>
#include <iostream>
#include "RAMStore.hpp"
#include "BTreeNodef.h"
#include <map>
#include <set>
#include <bits/stdc++.h>

using namespace std;

struct Block 
{
    int id;
    block data;
};

using Bucket = std::array<Block, Z>;

class BRAMf {
private:
    RAMStore* store;
    size_t depth;
    int pad;
    size_t blockSize;
    map<int, BTreeNodef*> cache;
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
    int GetBTreeNodefOnPath(int leaf, int depth);
    vector<int> GetIntersectingBlocks(int leaf, int depth);

    void FetchPath(int leaf);
    void WritePath(int leaf, int level);

    BTreeNodef* ReadData(int bid);
    void WriteData(int bid, BTreeNodef* b);
    void DeleteData(int bid, BTreeNodef* b);

    block SerialiseBucket(Bucket bucket);
    Bucket DeserialiseBucket(block buffer);

    Bucket ReadBucket(int pos);
    void WriteBucket(int pos, Bucket bucket);
    void Access(int bid, BTreeNodef*& node, int lastLeaf, int newLeaf);
    void Access(int bid, BTreeNodef*& node);


    size_t plaintext_size;
    size_t bucketCount;
    size_t clen_size;
    bool batchWrite = false;

    bool WasSerialised();
    void Print();

public:
    int searchf_bytes;
    int rtt;
    BRAMf(int maxSize, bytes<Key> key);
    ~BRAMf();
    int maxheight;
    BTreeNodef* ReadBTreeNodef(int bid, int lastLeaf);
    BTreeNodef* ReadBTreeNodef(int bid);
    int WriteBTreeNodef(int bid, BTreeNodef* n);
    int DeleteBTreeNodef(int bid, BTreeNodef* n);
    void start(bool batchWrite);
    void finalize(int& rootKey, int& rootPos);
    void finalizedel(int& brootKey, int& brootPos);
    static BTreeNodef* convertBlockToBTreeNodef(block b);
    void convertBlockToBTreeNodef(BTreeNodef*& node,block b);
    static block convertBTreeNodefToBlock(BTreeNodef* node);
    void getbid(block b, int &bID) ;

    //void setupInsert(vector<BTreeNodef*> nodes);
};

#endif
