#pragma once
#ifndef IndexORAM_H
#define IndexORAM_H

#include "AES.hpp"
#include <random>
#include <vector>
#include <unordered_map>
#include <string>
#include <iostream>
#include "RAMStore.hpp"
#include <map>
#include <set>
#include <bits/stdc++.h>
#include "Bid.h"
#include "Node.h"

using namespace std;


using Ibucket = std::array<Iblock, Z>;

class IndexORAM {
private:
    RAMStore* store;
    size_t blockSize;
    int leaves;
    int readCnt = 0;
    bytes<Key> key;

    // Randomness
    std::random_device rd;
    std::mt19937 mt;
    std::uniform_int_distribution<int> dis;

    int RandomPath();
    int GetNodeOnPath(int leaf, int depth);
    std::vector<Bid> GetIntersectingIblocks(int x, int depth);

    void FetchPath(int leaf);

    Node* ReadData(Bid bid);
    void WriteData(Bid bid, Node* b);
    void DeleteData(Bid bid, Node* b);

    block SerialiseIbucket(Ibucket bucket);
    Ibucket DeserialiseIbucket(block buffer);

    Ibucket ReadIbucket(int pos);
    void WriteIbucket(int pos, Ibucket bucket);
    void Access(Bid bid, Node*& node, int lastLeaf, int newLeaf);
    void Access(Bid bid, Node*& node);


    size_t plaintext_size;
    size_t bucketCount;
    size_t clen_size;
    bool batchWrite = false;

    bool WasSerialised();
    void Print();

public:
    void WritePath(int leaf, int level);
    size_t depth;
    map<Bid, Node*> cache;
    map<Bid, int> posCache;
    vector<int> leafList;
    vector<int> readviewmap;
    vector<int> writeviewmap;
    set<Bid> modified;
    IndexORAM(int maxSize, bytes<Key> key);
    ~IndexORAM();
    int maxheight;
    int searchi_bytes;
    map<Bid,pair<int,int>> localBCNT;
    Node* ReadNode(Bid bid, int lastLeaf, int newLeaf);
    Node* ReadNode(Bid bid);
    int WriteNode(Bid bid, Node* n);
    int DeleteNode(Bid bid, Node* n);
    void start(bool batchWrite);
    void finalize();
    void finalizeI();
    void WriteCache();
    static Node* convertIblockToNode(block b);
    void convertIblockToNode(Node*& node,block b);
    static block convertNodeToIblock(Node* node);
    int RandomSeedPath(Bid kw,int sc, int fc, int indexleaves);



    Node* setupReadN(Bid bid, int leaf);
    void setupReadN(Node*& n, Bid bid, int leaf);
    int setupWriteN(Bid bid, Node* n, Bid rootkey, int& rootPos);
    void setupWriteIbucket(Bid bid, Node* n, Bid rootKey, int& rootPos);
    void setupInsert(vector<Node*> nodes);
};

#endif
