#pragma once
#ifndef FileORAM_H
#define FileORAM_H

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
#include "Fbid.h"
#include "Fnode.h"
#include<atomic>
#include<mutex>

using namespace std;


using Fbucket = std::array<Fblock, Z>;
class FileORAM {
public:
std::mutex mut;
    RAMStore* store;
    size_t depth;
    size_t blockSize;
    map<Fbid, Fnode*> cache;
    vector<int> leafList;
    vector<int> readviewmap;
    vector<int> writeviewmap;
    set<Fbid> modified;
    int readCnt = 0;
    bytes<Key> key;

    // Randomness
    std::random_device rd;
    std::mt19937 mt;
    std::uniform_int_distribution<int> dis;

    int RandomPath();
    int GetFnodeOnPath(int leaf, int depth);
    std::vector<Fbid> GetIntersectingFblocks(int x, int depth);

    void FetchPath(int leaf);
    void WritePath(int leaf, int level);

    Fnode* ReadData(Fbid bid);
    void WriteData(Fbid bid, Fnode* b);
    void DeleteData(Fbid bid, Fnode* b);

    block SerialiseFbucket(Fbucket bucket);
    Fbucket DeserialiseFbucket(block buffer);

    Fbucket ReadFbucket(int pos);
    void WriteFbucket(int pos, Fbucket bucket);
    void Access(Fbid bid, Fnode*& node, int lastLeaf, int newLeaf);
    void Access(Fbid bid, Fnode*& node);


    size_t plaintext_size;
    size_t bucketCount;
    size_t clen_size;
    bool batchWrite = false;

    bool WasSerialised();
    void Print();

//public:
    FileORAM(int maxSize, bytes<Key> key);
    ~FileORAM();
    int maxheight;
    map<Fbid,pair<int,int>> localBCNT;
    map<Fbid,int> localPos;
    Fnode* ReadFnode(Fbid bid, int lastLeaf, int newLeaf);
    //Fnode* ReadFnode(Fbid bid);
    int WriteFnode(Fbid bid, Fnode* n);
    int DeleteFnode(Fbid bid, Fnode* n);
    void start(bool batchWrite);
    void finalize();
    static Fnode* convertFblockToFnode(block b);
    void convertFblockToFnode(Fnode*& node,block b);
    static block convertFnodeToFblock(Fnode* node);



    Fnode* setupReadN(Fbid bid, int leaf);
    void setupReadN(Fnode*& n, Fbid bid, int leaf);
    int setupWriteN(Fbid bid, Fnode* n, Fbid rootkey, int& rootPos);
    void setupWriteFbucket(Fbid bid, Fnode* n, Fbid rootKey, int& rootPos);
    void setupInsert(vector<Fnode*> nodes);
};

#endif