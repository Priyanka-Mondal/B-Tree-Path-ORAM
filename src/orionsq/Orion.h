#ifndef ORION_H
#define ORION_H
#include "OMAP.h"
#include "OMAPf.h"
#include<iostream>
using namespace std;

class Orion {
private:
    bool useHDD;
    OMAPf* srch;
    OMAPf *updt;
    //OMAPf *fcnt;
    OMAP* file;
    map<Bid, int> UpdtCnt;
    map<Bid,int> srchbids;
    map<Bid,int> updtbids;
    map<Bid,string> filebids;
public:
    Bid createBid(string keyword,int number);
    
    void insertWrap(string cont, int fileid, bool batch);
    void insert(vector<string> kws, vector<string> blocks, int ind);
    void setupinsert(vector<string> kws, vector<string> blocks, int ind);
    void batchInsert(vector<string> kws, vector<string> blocks, int ind) ;
    
    void remove(int ind);
    void removekw(vector <string> kws, int id);
    
    map<int,string> batchSearch(string keyword);
    vector<pair<int,string>> search(string keyword);
    
    Orion(bool useHDD,int maxSize, int filesize);    
    
    virtual ~Orion();
    void endSetup();
    void print();


};

#endif /* ORION_H */

