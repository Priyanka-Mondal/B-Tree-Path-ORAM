#ifndef OMAP_H
#define OMAP_H
#include <iostream>
#include<utility>
#include "ORAM.hpp"
#include "RAMStore.hpp"
#include <functional>
#include <fstream>
#include <cstdio>
#include <cstring>
#include <iostream>
#include "AVLTree.h"
using namespace std;

class OMAP {
private:
    Bid rootKey;
    int rootPos;
    AVLTree* treeHandler;

public:
    OMAP(int maxSize, bytes<Key> key);
    virtual ~OMAP();
    void insert(Bid key, pair<int,string> value);
    pair<int,string> find(Bid key);
    void printTree();
    void batchInsert(map<Bid, pair<int,string>> pairs);
    vector<pair<int,string>> batchSearch(vector<Bid> keys);
    void remove(Bid key);
};

#endif /* OMAP_H */

