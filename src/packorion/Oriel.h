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
#include <boost/algorithm/string/split.hpp>
#include <boost/algorithm/string/classification.hpp>
#include <sse/crypto/hash.hpp>
#include <boost/algorithm/string.hpp>
#include <openssl/conf.h>
#include <openssl/evp.h>
#include <openssl/err.h>
#include <sse/crypto/prg.hpp>

typedef uint64_t index_type;

using namespace std;

#ifndef AES_KEY_SIZE
#define AES_KEY_SIZE CryptoPP::AES::DEFAULT_KEYLENGTH
#define FILE_SIZE 1024
typedef array<uint8_t, AES_KEY_SIZE> prf_type;
typedef array<uint8_t, 1024> file_type;
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
    OMAPf *ac , *I, *fcnt;
    //OMAPf *del;
    map<prf_type, FileNode*> DictF;
    map<Bid, string> Imap, acmap, fcntmap;    
    map<int,int> localAC;
    bool local;
    bytes<Key> key;
    int clen_size;
public:
    Bid createBid(string keyword,int number);
    Bid createBid(string keyword,string id);

    void insertWrap(string cont, int ind, bool batch);
    void insertWrapper(vector<string> kws, string cont, int ind, bool batch);
    void insertkw(string keyword, int ind);
    void setupinsertkw(string keyword, int ind); 
    void insertFile(int ind, string content,bool batch);
    void setupInsertkw(string keyword, int ind) ;
    
    void remove(int ind); 
    void removekw(vector<string> keyword, int ind);
    
    vector<string> simplebatchSearch(string keyword);
    vector<string> search(string keyword);
    
    void print();

    Oriel(bool useHDD,int maxSize, int filesize, bool local);    
    virtual ~Oriel();

    void getAESRandomValue(unsigned char* keyword, int op, int srcCnt, int fileCnt, unsigned char* result);
    void endSetup();
};

#endif /* ORIEL_H */

