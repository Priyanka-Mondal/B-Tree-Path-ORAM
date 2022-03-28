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
#include <sse/crypto/hash.hpp>
#include "OMAP.h"
#include "OMAPf.h"
using namespace std;

enum OP {
  INS, DEL
};

typedef array<uint8_t, 1024> file_type;
typedef array<uint8_t, 128> fblock;

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
   map<prf_type, FileNode*> DictF;
    string Wg;
   inline prf_type bitwiseXOR(int input1, int op, prf_type input2);
    inline prf_type bitwiseXOR(prf_type input1, prf_type input2);
    inline void getAESRandomValue(unsigned char* keyword, int op, int srcCnt, int counter, unsigned char* result);
    Server* server;
    bool deleteFiles;
    double totalUpdateCommSize;
    double totalSearchCommSize;
    bool localStorage = false;
    OMAP* omap;
    OMAPf* ac;
    map<int,int> accCnt;
    map<prf_type, int> FileCnt;
    map<prf_type, int> SrcCnt;
    map<Bid, string> setupOMAP, setupAC;
    inline Bid getBid(string input);
public:
    Client(Server* server, bool deleteFiles, int keyworsSize,int fileSize);
    Client(bool deleteFiles, int keyworsSize, int fileSize);
    void update(OP op, string keyword, int ind, bool setup);
    map<int,string> search(string keyword);
    void updateRequest(OP op, string keyword, int ind, prf_type& address, prf_type& value);
    prf_type searchRequest(string keyword, vector<prf_type>& tokens);
    void searchProcess(vector<prf_type> tokens, prf_type k_w, vector<int>& ids, map<prf_type, prf_type>& cleaningPairs, string keyword);
    virtual ~Client();
    bool isSetupMode() const;
    double getTotalSearchCommSize() const;
    double getTotalUpdateCommSize() const;
    void endSetup();

    //void insertFile(OP op, int ind, string content, bool setup); 
    void insert(vector<string> keyword, int ind, bool setup);
    void insertFile(int ind, string content, bool setup);
    void remove(string keyword, int ind, bool setup);
};

#endif /* CLIENT_H */
