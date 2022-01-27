#ifndef FORAM_H
#define FORAM_H
#include "OMAP.h"
#include "OMAPf.h"
#include<iostream>
#include<sstream>
#include<utility>
#include <bits/stdc++.h>

using namespace std;

class Foram {
private:
    bool useHDD;
    map<Bid,string> setupPairs1;
    map<Bid,string> setupPairs2;
    OMAP* srch;
    OMAPf *updt;
    map<string, int> UpdtCnt;
    //map<string, int> LastIND;        
    
public:
    Bid createBid(string keyword,int number);
    Bid createBid(string keyword,string id);
    //Bid createBidS(string keyword,string id);
    void insert(vector<string> kws, vector<string> blocks, string ind);
    void insertdel(string ind);
    //void insertblk(string keyword, int ind);
    void setupInsert(vector<string> keyword, vector<string> blocks, string ind);
    string removefileblock(string ind); /* LATER*/
    void removekw(vector<string> keyword, string ind); /* LATER*/
    //void setupRemove(string keyword, int ind);
    //vector<string> searchWrapper(string keyword);
    vector<string> searchkw(string keyword);
    string searchfileblock(string ind, int blk);
    map<string,string> search(string keyword);
    void print();

    Foram(bool useHDD,int maxSize, int updSize);    
    virtual ~Foram();
    void beginSetup();
    void endSetup();
};

#endif /* BORION_H */

