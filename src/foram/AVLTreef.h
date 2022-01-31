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

    int height(Bid N, int& leaf);
    int max(int a, int b);
    Nodef* newNodef(Bid key, string value);
    Nodef* rightRotate(Nodef* y);
    Nodef* leftRotate(Nodef* x);
    int getBalance(Nodef* N);
    int RandomPath();

public:
    AVLTreef(int maxSize, bytes<Key> key);
    virtual ~AVLTreef();
    Bid insert(Bid rootKey, int& pos, Bid key, string value);
    Nodef* search(Nodef* head, Bid key);
    void batchSearch(Nodef* head, vector<Bid> keys, vector<Nodef*>* results);
    void printTree(Nodef* root, int indent);
    void startOperation(bool batchWrite = false);
    void finishOperation(bool find, Bid& rootKey, int& rootPos);
   
   //deletion related functions 
    Bid deleteNode(Nodef* nodef);
    Nodef* parentOf(Bid parentKey, int ppos, Bid childKey, int cpos, Bid key);
    Nodef* minValueNode(Bid rootKey, int pos, Nodef* rootroot);
    Bid balance(Nodef* node, int &pos);
    Bid balanceDel(Bid minKey, int& minpos, Nodef* parmin);
    Bid removeMain(Bid rootKey,int& pos, Bid delKey);
    Bid removeDel(Bid rootKey,int& pos,Bid delKey,int delPos,Nodef* paren);
    Bid removeRoot(Bid rootKey, int& pos);
    Bid realDelete(Nodef* paren,Bid delKey,int delPos);
    Bid deleteKey(Nodef* node, Bid key);
};

#endif /* AVLTREEF_H */




