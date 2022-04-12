#ifndef ORION_H
#define ORION_H
#include "OMAP.h"
#include "OMAPf.h"
#include "FileORAM.hpp"
#include<iostream>
#include <random>
#include "utils/Utilities.h"
using namespace std;

class Orion {
private:
    bool useHDD;
    bool local;
    FileORAM* srch;
    OMAPf *updt;
    OMAPf *fcnt;
    OMAP* file;
    FileORAM *fileoram;
    map<Bid, int> UpdtCnt;
    map<Bid,string> srchbids;
    map<Bid,string> fcntbids;
    map<Bid,int> updtbids;
    map<Bid,pair<string,int>> filebids;
    map<string,string> localFCNT;
    int RandomPath();
    int RandomSeedPath(Fbid kw,int fc, int sc, int indexleaves);
    std::random_device rd;
    std::mt19937 mt;
    std::uniform_int_distribution<int> dis;
    int fileleaves;
    int indexleaves;
public:
    void finalizeindex(int sc, int fc);
    Fbid createFbid(string keyword,int number);
    Bid createBid(string keyword,int number);
    
    Fnode* newNode(Fbid key, string value, int pos, int height);
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

