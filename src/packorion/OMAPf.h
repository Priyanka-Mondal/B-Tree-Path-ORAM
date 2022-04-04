#ifndef OMAPf_H
#define OMAPf_H
#include <iostream>
#include "ORAMf.hpp"
#include "RAMStore.hpp"
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
    AVLTreef* treeHandler;

public:
    OMAPf(int maxSize, bytes<Key> key);
    virtual ~OMAPf();
    void insert(Bid key, string val);
    string find(Bid key);
    void printTree();
    //void batchInsert(map<Bid, string> pairs);
    vector<string> batchSearch(vector<Bid> keys);
    void remove(Bid delKey);
    void setupInsert(map<Bid, string> pairs);
    string findAndIncrement(Bid key) ;

};

#endif /* OMAPf_H */

