#ifndef SERVER_H
#define SERVER_H
#include <string>
#include <map>
#include <vector>
#include <array>
#include "utils/rocksdb_wrapper.hpp"
#include <iostream>
#include <unistd.h>
#include <iostream>
#include <sstream>
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

class Server {
private:

    bool deleteFiles;
    bool useRocksDB;

public:
    sse::sophos::RockDBWrapper edb_;
    sse::sophos::RockDBWrapper edbfake_;
    map<prf_type, prf_type> DictW;
    map<prf_type, FileNode*> DictF;
    map<prf_type, string>DictFake;
    Server(bool useHDD, bool deleteFiles);
    void update(prf_type addr, prf_type val, string fake);
    void updateFile(prf_type addr, FileNode* val);
    pair<vector<prf_type>,vector<string>> search(vector<prf_type> KList);
    vector<FileNode*> searchFile(vector<prf_type> KList);
    virtual ~Server();

};

#endif /* SERVER_H */

