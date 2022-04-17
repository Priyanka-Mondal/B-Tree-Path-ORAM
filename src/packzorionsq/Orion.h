#ifndef ORION_H
#define ORION_H
#include "FileORAM.hpp"
#include "IndexORAM.hpp"
#include<iostream>
#include <random>
#include "utils/Utilities.h"
using namespace std;

class Orion {
private:
    bool useHDD;
    bool local;
    IndexORAM* srch;
    FileORAM *fileoram;
    map<Bid, int> UpdtCnt;
    map<Bid,Node*> srchmap;
    vector<Node*> srchnodes;
    vector<Fnode*> filenodes;
    map<Bid,string> fcntbids;
    map<Bid,int> updtbids;
    map<string,string> localFCNT;//sc-fc
    map<string,int> localFC;//sc-fc
    map<string,int> localSC;//sc-fc
    map<string,string> localACNT; //ac-bc
    map<int,int> localAC;//sc-fc
    map<int,int> localBC;//sc-fc
    int RandomPath(int id, int ac, int bc, int leaves);
    int RandomSeedPath(string id,int fc, int sc, int leaves);
    std::random_device rd;
    std::mt19937 mt;
    std::uniform_int_distribution<int> dis;
    int fileleaves;
    int indexleaves;
public:
    void finalizeindex(int sc, int fc);
    Fbid createFbid(int id,int number);
    Bid createBid(string keyword,int number);
    
    //Node* newNode(Bid key, string value, int pos);
    Node* newNode(Bid key, array<byte_t, NODE_SIZE>, int pos);
    Fnode* newFnode(Fbid key, string value, int pos);
    void insertWrap(string cont, int fileid, bool batch);
    void insert(vector<string> kws, vector<string> blocks, int ind);
    void setupinsert(vector<string> kws, vector<string> blocks, int ind);
    void batchInsert(vector<string> kws, vector<string> blocks, int ind) ;
    
    void remove(int ind);
    void removekw(vector <string> kws, int id);
    
    vector<string> batchSearch(string keyword);
    pair<int,vector<string>> simplebatchSearch(string keyword);
    vector<pair<int,string>> search(string keyword);
    
    Orion(bool useHDD,int maxSize, int filesize, bool local);    
    
    virtual ~Orion();
    void endSetup();
    void print();
};

#endif /* ORION_H */

