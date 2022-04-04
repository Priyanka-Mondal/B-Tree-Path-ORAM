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
    void insert(Bid key, int value);
    int find(Bid key);
    void printTree();
    //void batchInsert(map<Bid, string> pairs);
    vector<int> batchSearch(vector<Bid> keys);
    void remove(Bid delKey);


    void setupinsert(Bid key, int value);
    int setupfind(Bid key);
    void setupInsert(map<Bid, int> pairs);
    int findAndIncrement(Bid key); 
};

#endif /* OMAPf_H */

