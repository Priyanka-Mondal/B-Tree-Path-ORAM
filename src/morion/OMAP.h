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
    void insert(Bid key, string value);
    string find(Bid key);
    void printTree();
    void batchInsert(map<Bid, string> pairs);
    vector<string> batchSearch(vector<Bid> keys);
    Bid remove(Bid key);


    string setupfind(Bid key);
    void setupinsert(Bid key, string value);
    void setupInsert(map<Bid, string> pairs);
};

#endif /* OMAP_H */

