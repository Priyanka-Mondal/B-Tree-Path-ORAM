#ifndef OMAPf_H
#define OMAPf_H
#include <iostream>
#include <functional>
#include <fstream>
#include <cstdio>
#include <cstring>
#include <iostream>
#include "RAMStore.hpp"
#include "ORAMf.hpp"
#include "AVLTreef.h"
using namespace std;

class OMAPf {
private:
    Bid rootKey;
    int rootPos;
    AVLTreef* treeHandler;

public:
    OMAPf(int maxSize, bytes<Key> key);
    virtual ~OMAPf();
    void remove(Bid delKey);
    void insert(Bid key, string value);
    string find(Bid key);
    void delKey(Bid key);
    void printTree();
    void batchInsert(map<Bid, string> cont);
    vector<string> batchSearch(vector<Bid> keys);


    void setupinsert(Bid key, string value);
    string setupfind(Bid key);
};

#endif /* OMAPf_H */

