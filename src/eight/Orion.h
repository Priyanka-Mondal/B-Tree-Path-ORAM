#ifndef ORION_H
#define ORION_H
#include "BTree.h"
#include "BTreef.h"
#include<iostream>
using namespace std;

class Orion 
{
private:
    
    bool useHDD;
    bool local;
    BTree* btreeHandler;
    BTreef* fileHandler;
    map<Bid, int> UpdtCnt;
    map<Bid,int> srchbids;
    map<Bid,int> fcntbids;
    map<Bid,int> setup;
    map<Bid,string> setupfile;
    map<string,int> fcntbtree;
    map<Bid,int> updtbids;
    map<Bid,string> filebids;
    map<string,int> localFCNT;
    map<int,int> localBCNT;
public:
    Bid createBid(string keyword,int number);
    
    void insertWrap(string cont, int fileid, bool batch);
    void insert(vector<string> kws, vector<string> blocks, int ind);
    void setupInsert(vector<string> kws, vector<string> blocks, int ind);
    void batchInsert(vector<string> kws, vector<string> blocks, int ind) ;
    
    void remove(string kw);
    void removekw(vector <string> kws, int id);
    
    vector<string> batchSearch(string keyword);
    vector<string> simplebatchSearch(string keyword);
    vector<string> search(string keyword,ofstream &sres, double speed, double latency);
    
    Orion(bool useHDD,int maxSize, int fileSize, bool local);    
    
    virtual ~Orion();
    void endSetup();
    void print();


};

#endif /* ORION_H */

