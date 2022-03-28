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
using namespace std;

enum OP {
    INS, DEL
};

class Client {
public:
    string Wg;
    inline prf_type bitwiseXOR(int input1, int op, prf_type input2);
    inline prf_type bitwiseXOR(string input1, int op, prf_type input2);
    inline prf_type bitwiseXOR(prf_type input1, prf_type input2);
    inline void getAESRandomValue(unsigned char* keyword, int op, int srcCnt, int counter, unsigned char* result);
    Server* server;
    bool deleteFiles;
    bool localStorage = false;

public:
    Client(Server* server, bool deleteFiles, int keyworsSize, int filecnt);
    Client(bool deleteFiles, int keyworsSize, int filecnt);
    void insertFile(int ind, string content); 
    string getfile(int id);
    virtual ~Client();

};

#endif /* CLIENT_H */
