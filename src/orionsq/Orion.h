#ifndef ORION_H
#define ORION_H
#include "OMAP.h"
#include "OMAPf.h"
#include<iostream>
using namespace std;

class Orion {
private:
    bool useHDD;
    map<Bid,string > setupPairs1;
    map<Bid,string > setupPairs2;
    OMAPf* srch,*updt, *fcnt;
    OMAP* file;
    map<string, int> UpdtCnt;
    map<string, int> LastIND;        
    
public:
    Bid createBid(string keyword,int number);
    
    void insertWrap(string cont, int fileid, bool batch);
    void insert(vector<string> kws, vector<string> blocks, int ind);
    void setupinsert(vector<string> kws, vector<string> blocks, int ind);
    
    void remove(int ind);
    void removekw(vector <string> kws, int id);
    
    map<int,string> search(string keyword);
    
    Orion(bool useHDD,int maxSize);    
    
    virtual ~Orion();
    
    void print();

};

#endif /* ORION_H */

