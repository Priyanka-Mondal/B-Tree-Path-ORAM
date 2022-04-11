#ifndef ORION_H
#define ORION_H
#include "FileORAM.hpp"
#include<iostream>
#include <random>
using namespace std;

class Orion {
public:
    bool local;
    map<string,int> localFCNT;
    map<int,int> localBCNT;
    int RandomPath();
    std::random_device rd;
    std::mt19937 mt;
    std::uniform_int_distribution<int> dis;
    std::vector<std::thread> threads;
    FileORAM *fileoram;
    Fbid createFbid(string keyword,int number);
    
    Fnode* newNode(Fbid key, string value, int pos);
    void insertWrap(string cont, int fileid, bool batch);
    void batchInsert(vector<string> blocks, int ind) ;
    
    void remove(int ind);
    void removekw(vector <string> kws, int id);
    
    vector<string> batchSearch(string keyword);
    vector<string> syncSearch(vector<int>ids);
    vector<string> asyncSearch(vector<int>ids);
    vector<pair<int,string>> search(string keyword);
    
    Orion(int filesize, bool local);    
    
    virtual ~Orion();
    void endSetup();
    void print();
    int stoI(string updt_cnt);
};

#endif /* ORION_H */

