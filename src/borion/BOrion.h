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
    map<Bid,pair<string,string>> srchmap;
    map<Bid,string> updmap;
    OMAP* srch;
    OMAPf *updt;
    
public:
    Bid createBid(string keyword,int number);
    Bid createBid(string keyword,string id);
    void insertWrap(string cont, string ind, bool batch);
    void insertWrapper(vector<string> kws, vector<string> blocks, string ind);
    void insert(string keyword, string ind); // inserts a keyword for ind
    void insertdel(string ind);
    void insertFile(string keyword, vector<string> blocks);
    void setupInsert(string keyword, pair<int,string> ind);
    void remove(string ind); 
    void removekw(vector<string> keyword, string ind); /* LATER*/
    //void setupRemove(string keyword, int ind);
    vector<pair<string,string>> searchkw(string keyword);
    pair<int,string> searchfileblock(string ind, int blk);
    string searchfileblocknum(string ind);
    map<string,string> searchWrapper(string keyword);
   void setupInsertWrapper(vector<string> kws,vector<string> blocks,string ind);
    void setupInsertkws(vector<string> kws, string ind);
    void setupInsertFile(string ind, vector<string> blocks);
    void print();

    BOrion(bool useHDD,int maxSize);    
    virtual ~BOrion();
    void beginSetup();
    void endSetup();

};

#endif /* BORION_H */

