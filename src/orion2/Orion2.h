#ifndef ORION2_H
#define ORION2_H
#include "OMAP.h"
#include "OMAPf.h"
#include<iostream>
using namespace std;

class Orion2 {
private:
    bool useHDD;
    map<Bid,string > setupPairs1;
    map<Bid,string > setupPairs2;
    OMAP* srch,*updt;
    OMAPf* files;
    map<string, int> UpdtCnt;
    map<string, int> LastIND;        
    
public:
    Bid createBid(string keyword,int number);
    Bid createBidF(int ind, int blocknum);
    void insert(string keyword, int ind);
    void insertFile(int ind, vector<string> blocks);
    void setupInsert(string keyword, int ind);
    void remove(string keyword, int ind);
    void setupRemove(string keyword, int ind);
    vector<int> search(string keyword);
    vector<string> searchFile(int ind);
    Orion2(bool useHDD,int kwSize, int fileSize);    
    virtual ~Orion2();
    void beginSetup();
    void endSetup();

};

#endif /* ORION2_H */

