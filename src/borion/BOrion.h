#ifndef BORION_H
#define BORION_H
#include "OMAP.h"
#include<iostream>
#include<sstream>
#include <bits/stdc++.h>

using namespace std;

class BOrion {
private:
    bool useHDD;
    map<Bid,string > setupPairs1;
    map<Bid,string > setupPairs2;
    OMAP* srch; //*updt;
    map<string, int> UpdtCnt;
    //map<string, int> LastIND;        
    
public:
    Bid createBid(string keyword,int number);
    void insertWrapper(vector<string> kws, vector<string> blocks, string ind);
    void insert(string keyword, string ind);
    void insertFile(string keyword, vector<string> blocks);
    //void insertblk(string keyword, int ind);
    void setupInsert(string keyword, string ind);
    //void remove(string keyword, int ind); /* LATER*/
    //void setupRemove(string keyword, int ind);
    //vector<string> searchWrapper(string keyword);
    vector<string> search(string keyword);
    BOrion(bool useHDD,int maxSize);    
    virtual ~BOrion();
    void beginSetup();
    void endSetup();

};

#endif /* BORION_H */

