#ifndef MORION_H
#define MORION_H
#include "OMAP.h"
#include "OMAPf.h"
#include<iostream>
#include<sstream>
#include<utility>
#include <bits/stdc++.h>

using namespace std;

class MOrion {
private:
    bool useHDD;
    map<Bid,string> srchmap;
    //map<Bid,int> updtmap;
    map<Bid,int>fcntmap;
    OMAP* srch;
    OMAPf *updt , *fcnt;
    //map<string,int> idvol;
    //map<string,int> tvol;
    bool local;
    bool notfile;
    map<string,int> localFCNT;
    map<int,int> localBCNT;
public:
    Bid createBid(string keyword,int number);
    Bid createBid(string keyword,string id);

    void insertWrap(string cont, int ind, bool batch);
    void insertWrapper(vector<string> kws, vector<string> blocks, int ind);
    void insertkw(string keyword, int ind);
    void insertFile(int id, vector<string> blocks);
    
  void setupinsertWrapper(vector<string> kws,vector<string> blocks,int ind);
    void setupinsertkw(string keyword, int ind); 
    void setupinsertFile(int id, vector<string> blocks);
    
    void remove(int ind); 
    void removekw(vector<string> keyword, int ind);
    
    //map<string,string> searchWrapper(string keyword);
    vector<string> search(string keyword);
    //vector<pair<string,string>> searchsimple(string keyword);
    //vector<pair<string,string>> setupsearch(string keyword);
    vector<string> batchSearch(string keyword);
    pair<int,vector<string>> simplebatchSearch(string keyword);
    
    void print();

    MOrion(bool useHDD,int maxSize, int count , bool local, bool notfile);    
    virtual ~MOrion();
    void beginSetup();
    void endSetup();

};

#endif /* BORION_H */

