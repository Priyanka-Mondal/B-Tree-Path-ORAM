#ifndef ORIEL_H
#define ORIEL_H
#include "OMAPf.h"
#include<iostream>
#include<sstream>
#include<utility>
#include <bits/stdc++.h>
#include <cryptopp/rng.h>
#include <cryptopp/sha.h>
#include <cryptopp/hex.h>
#include <cryptopp/modes.h>
#include <cryptopp/aes.h>
#include <vector>
#include "utils/Utilities.h"

typedef uint64_t index_type;

using namespace std;

#ifndef AES_KEY_SIZE
#define AES_KEY_SIZE CryptoPP::AES::DEFAULT_KEYLENGTH
#define FILE_SIZE 1024
typedef array<uint8_t, AES_KEY_SIZE> prf_type;
typedef array<uint8_t, 1024> file_type;
typedef array<uint8_t, 64> fblock;
#endif

class FileNode
{
	public:
		FileNode(){ };
		~FileNode() { };

		fblock data;
		FileNode *next;
};

class Oriel {
private:
    bool useHDD;
    map<Bid,pair<string,string>> Imap;
    map<Bid,string> updmap;
    map<Bid,string> acmap;
    OMAPf *ac , *I, *del;
    map<string,int> idvol;
    map<string,int> tvol;
    map<prf_type, FileNode*> DictF;
    
public:
    Bid createBid(string keyword,int number);
    Bid createBid(string keyword,string id);

    void insertWrap(string cont, int ind, bool batch);
    void insertWrapper(vector<string> kws, vector<string> blocks, int ind);
    void insertkw(string keyword, int ind);
    void insertFile(string keyword, vector<string> blocks);
   
    void setupinsertWrap(vector<string> kws,vector<string> blocks,int ind);
    void setupinsertFile(string keyword, vector<string> blocks);
    void setupinsertkw(string keyword, int ind); 
    vector<int> setupsearch(string keyword);
    
    void remove(string ind); 
    void removekw(vector<string> keyword, string ind);
    
    vector<int> search(string keyword);
    
    void print();

    Oriel(bool useHDD,int maxSize);    
    virtual ~Oriel();
    void beginSetup();
    void endSetup();

    void updateFile(prf_type addr, FileNode* val);
    vector<FileNode*> searchFile(vector<prf_type> KList);
};

#endif /* ORIEL_H */

