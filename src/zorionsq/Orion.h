#ifndef ORION_H
#define ORION_H
#include "OMAP.h"
#include "OMAPf.h"
#include "FileORAM.hpp"
#include<iostream>
using namespace std;

class Orion {
private:
    bool useHDD;
    bool local;
    OMAPf* srch;
    OMAPf *updt;
    OMAPf *fcnt;
    OMAP* file;
    FileORAM *fileoram;
    map<Bid, int> UpdtCnt;
    map<Bid,int> srchbids;
    map<Bid,int> fcntbids;
    map<Bid,int> updtbids;
    map<Bid,pair<string,int>> filebids;
    map<string,pair<int,int>> localFCNT;
    map<int,int> localBCNT;
public:
    Fbid createFbid(string keyword,int number);
    Bid createBid(string keyword,int number);
    
    Fnode* newNode(Bid key, string value, int pos, int height);
    void insertWrap(string cont, int fileid, bool batch);
    void insert(vector<string> kws, vector<string> blocks, int ind);
    void setupinsert(vector<string> kws, vector<string> blocks, int ind);
    void batchInsert(vector<string> kws, vector<string> blocks, int ind) ;
    
    void remove(int ind);
    void removekw(vector <string> kws, int id);
    
    vector<string> batchSearch(string keyword);
    vector<string> simplebatchSearch(string keyword);
    vector<pair<int,string>> search(string keyword);
    
    Orion(bool useHDD,int maxSize, int filesize, bool local);    
    
    virtual ~Orion();
    void endSetup();
    void print();


};

#endif /* ORION_H */

