#ifndef AVLTREEf_H
#define AVLTREEf_H
#include "ORAMf.hpp"
#include <functional>
#include <fstream>
#include <cstdio>
#include <cstring>
#include <array>
#include <memory>
#include <type_traits>
#include <iomanip>
#include "Bid.h"
#include <random>
#include <algorithm>
#include <stdlib.h>
using namespace std;

class AVLTreef {
private:
    int setupProgress = 0;
    int height(Bid N, int& leaf);
    int max(int a, int b);
    Node* newNode(Bid key, string value);
    Node* rightRotate(Node* y);
    Node* leftRotate(Node* x);
    int getBalance(Node* N);
    int RandomPath();
    int maxOfRandom;
    int sortedArrayToBST(int start, int end, int& pos, Bid& node);
    vector<Node*> setupNodes;

public:
    ORAMf *oram;
    string incrementFileCnt(Node* head, Bid key);
    AVLTreef(int maxSize, bytes<Key> key);
    virtual ~AVLTreef();
    Bid insert(Bid rootKey, int& pos, Bid key, string value);
    Node* search(Node* head, Bid key);
    void batchSearch(Node* head, vector<Bid> keys, vector<Node*>* results);
    void printTree(Node* root, int indent);
    void startOperation(bool batchWrite = false);
    void finishOperation(bool find, Bid& rootKey, int& rootPos);
    void setupInsert(Bid& rootKey, int& rootPos, map<Bid, string> pairs);
};

#endif /* AVLTREEf_H */




