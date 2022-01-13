#ifndef BORION_H
#define BORION_H
#include "OMAP.h"
#include "OMAPf.h"
#include<iostream>
#include<sstream>
#include<utility>
#include <bits/stdc++.h>

using namespace std;

class BOrion {
private:
    bool useHDD;
    map<Bid,pair<int,string>> setupPairs1;
    map<Bid,pair<int,string>> setupPairs2;
    OMAP* srch;
    OMAPf *updt;
    map<string, int> UpdtCnt;
    //map<string, int> LastIND;        
    
public:
    Bid createBid(string keyword,int number);
    Bid createBid(string keyword,string id);
    //Bid createBidS(string keyword,string id);
    void insertWrapper(vector<string> kws, vector<string> blocks, string ind);
    void insert(string keyword, string ind);
    void insertFile(string keyword, vector<string> blocks);
    //void insertblk(string keyword, int ind);
    void setupInsert(string keyword, pair<int,string> ind);
    void removekw(string keyword, string ind); /* LATER*/
    //void setupRemove(string keyword, int ind);
    //vector<string> searchWrapper(string keyword);
    vector<pair<int,string>> search(string keyword);
    map<string,string> searchWrapper(string keyword);

    BOrion(bool useHDD,int maxSize);    
    virtual ~BOrion();
    void beginSetup();
    void endSetup();

};

#endif /* BORION_H */

