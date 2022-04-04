#ifndef AVLTREEF_H
#define AVLTREEF_H
#include <iostream>
#include "ORAMf.hpp"
#include "RAMStore.hpp"
#include <functional>
#include <fstream>
#include <cstdio>
#include <cstring>
#include <iostream>
#include <array>
#include <memory>
#include <type_traits>
#include <iomanip>
#include <bits/stdc++.h>
#include "Bid.h"
#include <random>
using namespace std;

class AVLTreef {
private:
    ORAMf *oram;
    std::random_device rd;
    std::mt19937 mt;
    std::uniform_int_distribution<int> dis;
    int totalleaves;
    int setupleaf= -1;
    int setupProgress = 0;
    vector<Nodef*> setupNodes;

    int height(Bid N, int& leaf);
    int setupheight(Bid key, int& leaf);
    int max(int a, int b);
    Nodef* newNodef(Bid key, int value);
    Nodef* setupnewNodef(Bid key, int value);
    Nodef* setuprightRotate(Nodef* y, Bid rootKey, int& pos);
    Nodef* setupleftRotate(Nodef* x, Bid rootKey, int& pos);
    Nodef* rightRotate(Nodef* y);
    Nodef* leftRotate(Nodef* x);
    int getBalance(Nodef* N);
    int setupgetBalance(Nodef* N);
    int RandomPath();
    int notsoRandomPath(); 

public:
    AVLTreef(int maxSize, bytes<Key> key);
    virtual ~AVLTreef();
    Bid remove(Bid rootKey, int& pos, Bid delKey);
    Bid insert(Bid rootKey, int& pos, Bid key, int value);
    Nodef* search(Nodef* head, Bid key);
    void batchSearch(Nodef* head, vector<Bid> keys, vector<Nodef*>* results);
    void printTree(Nodef* root, int indent);
    void startOperation(bool batchWrite = false);
    void finishOperation(bool find, Bid& rootKey, int& rootPos);
    
    //deletion related functions
    int deleteNode(Nodef* nodef);
    Nodef* parentOf(Bid parentKey, int ppos, Bid childKey, int cpos, Bid key);
    Nodef* minValueNode(Bid rootKey, int pos, Nodef* rootroot);
    Bid balance(Nodef* node, int &pos);
    Bid balanceDel(Bid minKey, int& minpos, Nodef* parmin);
    Bid removeMain(Bid rootKey,int& pos, Bid delKey);
    Bid removeDel(Bid rootKey,int& pos,Bid delKey,int delPos,Nodef* paren);
    Bid removeRoot(Bid rootKey, int& pos);
    Bid realDelete(Nodef* paren,Bid delKey,int delPos);


    Nodef* setupsearch(Nodef* head, Bid key);
    Bid setupinsert(Bid rootKey, int& pos, Bid key, int value);
    int sortedArrayToBST(int start, int end, int& pos, Bid& node);
    void setupInsert(Bid& rootKey, int& rootPos, map<Bid, int> pairs);
    string findAndIncrement(Nodef* head, Bid key);
};

#endif /* AVLTREEF_H */




