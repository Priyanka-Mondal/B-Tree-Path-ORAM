#ifndef ORAMf_H
#define ORAMf_H

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

using namespace std;

class Nodef {
public:

    Nodef() {
    }

    ~Nodef() {
    }
    Bid key;
    std::array< byte_t, 64> value; 
    int pos;
    Bid leftID;
    int leftPos;
    Bid rightID;
    int rightPos;
    unsigned int height;
};

struct Blockf {
    Bid id;
    block data;
};

using Bucketf = std::array<Blockf, Z>;

class ORAMf {
private:
    RAMStore* store;
    using Stash = std::unordered_map<Bid, block>;
    size_t depth;
    size_t blockSize;
    map<Bid, Nodef*> cache;
    vector<int> leafList;
    vector<int> readviewmap;
    vector<int> writeviewmap;
    set<Bid> modified;
    int readCnt = 0;
    bytes<Key> key;

    // Randomness
    std::random_device rd;
    std::mt19937 mt;
    std::uniform_int_distribution<int> dis;

    int RandomPath();
    int GetNodefOnPath(int leaf, int depth);
    std::vector<Bid> GetIntersectingBlocks(int x, int depth);

    void FetchPath(int leaf);
    void WritePath(int leaf, int level);

    Nodef* ReadData(Bid bid);
    void WriteData(Bid bid, Nodef* b);

    block SerialiseBucket(Bucketf bucket);
    Bucketf DeserialiseBucket(block buffer);

    Bucketf ReadBucket(int pos);
    void WriteBucket(int pos, Bucketf bucket);
    void Access(Bid bid, Nodef*& node, int lastLeaf, int newLeaf);
    void Access(Bid bid, Nodef*& node);


    size_t plaintext_size;
    size_t bucketCount;
    size_t clen_size;
    bool batchWrite = false;

    bool WasSerialised();
    void Print();

public:
    ORAMf(int maxSize, bytes<Key> key);
    ~ORAMf();

    Nodef* ReadNodef(Bid bid, int lastLeaf, int newLeaf);
    Nodef* ReadNodef(Bid bid);
    int WriteNodef(Bid bid, Nodef* n);
    void start(bool batchWrite);
    void finilize(bool find, Bid& rootKey, int& rootPos);
    static Nodef* convertBlockToNodef(block b);
    static block convertNodefToBlock(Nodef* node);
};

#endif
