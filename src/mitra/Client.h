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
using namespace std;

enum OP {
    INS, DEL
};

class Client {
private:
    vector<int> fakefileids;
    //vector<int> getfakefileids();
public:
    string Wg;
    inline prf_type bitwiseXOR(int input1, int op, prf_type input2);
    inline prf_type bitwiseXOR(string input1, int op, prf_type input2);
    inline prf_type bitwiseXOR(prf_type input1, prf_type input2);
    inline void getAESRandomValue(unsigned char* keyword, int op, int srcCnt, int counter, unsigned char* result);
    Server* server;
    bool deleteFiles;
    double totalUpdateCommSize;
    double totalSearchCommSize;
    bool localStorage = false;
    OMAP* omapw, *omapf;
    map<prf_type, int> FileCnt;
    map<prf_type, int> SrcCnt;
    map<prf_type, int> AccsCnt;
    map<prf_type, int> BktCnt;
    map<Bid, string> setupOMAP;
    map<Bid, string> setupOMAPf;
    inline Bid getBid(string input);

public:
    Client(Server* server, bool deleteFiles, int keyworsSize, int filecnt);
    Client(bool deleteFiles, int keyworsSize, int filecnt);
    void insertFile(OP op, int ind, string content, bool setup); 
    void insert(string keyword, int ind, bool setup);
    void remove(string keyword, int ind, bool setup);
    map<int,string> searchfile(vector<int> inds) ;
    map<int, string> search(string keyword);
    void updateRequest(OP op, string keyword, int ind, prf_type& address, prf_type& value);
    prf_type searchRequest(string keyword, vector<prf_type>& tokens);
    void searchProcess(vector<prf_type> tokens, prf_type k_w, vector<int>& ids, map<prf_type, prf_type>& cleaningPairs, string keyword);
    virtual ~Client();
    bool isSetupMode() const;
    double getTotalSearchCommSize() const;
    double getTotalUpdateCommSize() const;
    void endSetup();
    void addfakefileid(int fileid);

};

#endif /* CLIENT_H */