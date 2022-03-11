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
    map<Bid,string>fcntmap;
    OMAP* srch;
    OMAPf *updt , *fcnt;
    map<string,int> idvol;
    map<string,int> tvol;
    
public:
    Bid createBid(string keyword,int number);
    Bid createBid(string keyword,string id);

    void insertWrap(string cont, string ind, bool batch);
    void insertWrapper(vector<string> kws, vector<string> blocks, string ind);
    void insertkw(string keyword, string ind);
    void insertdel(string ind);
    void insertFile(string keyword, vector<string> blocks);
    void setupinsertFile(string keyword, vector<string> blocks);
    
    void remove(string ind); 
    void removekw(vector<string> keyword, string ind);
    //void setupRemove(string keyword, int ind);
    
    map<string,string> searchWrapper(string keyword);
    vector<pair<string,string>> search(string keyword);
    
    void print();

    BOrion(bool useHDD,int maxSize);    
    virtual ~BOrion();
    void beginSetup();
    void endSetup();

    vector<pair<string,string>> setupsearch(string keyword);
  void setupinsertWrapper(vector<string> kws,vector<string> blocks,string ind);
    void setupinsertkw(string keyword, string ind); 
};

#endif /* BORION_H */

