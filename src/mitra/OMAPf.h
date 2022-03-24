#ifndef OMAPf_H
#define OMAPf_H
#include <iostream>
#include "ORAMf.hpp"
#include <functional>
#include <fstream>
#include <cstdio>
#include <cstring>
#include <iostream>
#include "AVLTreef.h"
using namespace std;

class OMAPf {
private:
    Bid rootKey;
    int rootPos;

public:
    AVLTreef* treeHandler;
    OMAPf(int maxSize, bytes<Key> key);
    virtual ~OMAPf();
    void insert(Bid key, string value);
    string incrementFileCnt(Bid key);
    string find(Bid key);
    void printTree();
    void batchInsert(map<Bid, string> pairs);
    void setupInsert(map<Bid, string> pairs);
    vector<string> batchSearch(vector<Bid> keys);
};

#endif /* OMAPf_H */
