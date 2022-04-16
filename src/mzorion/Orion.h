#ifndef ORION_H
#define ORION_H
#include "ORAM.hpp"
#include<iostream>
#include <random>
#include "utils/Utilities.h"
using namespace std;

class Orion {
private:
    bool local;
    ORAM* oram;
    map<Bid, int> UpdtCnt;
    map<Bid,Node*> indexmap;
    vector<Node*> oramnodes;
    map<Bid,string> fcntbids;
    map<Bid,int> updtbids;
    map<string,string> localFCNT;//sc-fc
    map<string,int> localFC;
    map<string,int> localSC;
    map<string,string> localACNT; //ac-bc
    map<int,int> localAC;
    map<int,int> localBC;
    int RandomPath(int id, int ac, int bc, int leaves);
    int RandomIndPath(string id,int fc, int sc, int leaves);
    std::random_device rd;
    std::mt19937 mt;
    std::uniform_int_distribution<int> dis;
    int leaves;
public:
    void finalizeindex(int sc, int fc);
    Bid createBid(string keyword,int number);
    Bid createBid(int id,int number);
    
    //Node* newNode(Bid key, string value, int pos);
    Node* newNode(Bid key, array<byte_t, BLOCK>, int pos);
    void insertWrap(string cont, int fileid, bool batch);
    void insert(vector<string> kws, vector<string> blocks, int ind);
    void setupinsert(vector<string> kws, vector<string> blocks, int ind);
    void batchInsert(vector<string> kws, vector<string> blocks, int ind) ;
    
    void remove(int ind);
    void removekw(vector <string> kws, int id);
    
    vector<string> batchSearch(string keyword);
    vector<string> simplebatchSearch(string keyword);
    vector<pair<int,string>> search(string keyword);
    
    Orion(int size, bool local);    
    
    virtual ~Orion();
    void endSetup();
    void print();
};

#endif /* ORION_H */

