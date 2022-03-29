#ifndef CLIENT_H
#define CLIENT_H
#include <string>
#include <map>
#include <vector>
#include <array>
#include "Server.h"
#include <iostream>
#include <sstream>
#include "mitra/Server.h"
#include "utils/Utilities.h"
#include <boost/algorithm/string/split.hpp>
#include <boost/algorithm/string/classification.hpp>
#include <boost/algorithm/string.hpp>
#include <sse/crypto/hash.hpp>
#include "AES.hpp"

using namespace std;

enum OP {
    INS, DEL
};

class FileNode
{
 public:
 FileNode(){ };
    ~FileNode() { };

   fblock data;
     FileNode *next;
};


class Client {
public:
    map<prf_type, FileNode* > DictF;
    string Wg;
    inline prf_type bitwiseXOR(int input1, int op, prf_type input2);
    inline prf_type bitwiseXOR(string input1, int op, prf_type input2);
    inline prf_type bitwiseXOR(prf_type input1, prf_type input2);
    inline void getAESRandomValue(unsigned char* keyword, int op, int srcCnt, int counter, unsigned char* result);
    Server* server;
    bool deleteFiles;
    bool localStorage = false;
    size_t clen_size;
    bytes<Key> key;

public:
    Client(Server* server, bool deleteFiles, int keyworsSize, int filecnt);
    Client(bool deleteFiles, int keyworsSize, int filecnt);
    void insertFile(int ind, string content); 
    string getfile(FileNode* fn);
    int getfreq(string kw, int fileid);
    virtual ~Client();

};

#endif /* CLIENT_H */
