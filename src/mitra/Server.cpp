#include "Server.h"
#include <iostream>
#include <sstream>
#include <cryptopp/rng.h>
#include <cryptopp/sha.h>
#include <cryptopp/hex.h>
#include <cryptopp/modes.h>
#include <cryptopp/aes.h>
#include <vector>
#include "utils/Utilities.h"

Server::Server(bool useHDD, bool deleteFiles) : edb_("MitraEDB"), edbfake_("FakeEDB") {
    this->useRocksDB = useHDD;
    this->deleteFiles = deleteFiles;
    this->useRocksDB = false;
}

Server::~Server() {
}

void Server::updateFile(prf_type addr, FileNode* val) {
//    if (useRocksDB) {
//        edb_.put(addr, val);
//    } else {
        DictF[addr] = val;
//    }
}
void Server::update(prf_type addr, prf_type val, string fake) {
//    if (useRocksDB) {
//        edb_.put(addr, val);
//	edbfake_.put(addr,fake);
//    } else {
        DictW[addr] = val;
	DictFake[addr] = fake;
//    }
}
vector<FileNode*> Server::searchFile(vector<prf_type> KList) {
    vector<FileNode*> result;
    result.reserve(KList.size());
    file_type notfound;
    memset(notfound.data(), 0, FILE_SIZE);
    for (unsigned int i = 0; i < KList.size(); i++) {
        FileNode* val;
//        if (useRocksDB) {
//		//cout <<"KList[i]"<< KList[i]<<endl;
//            bool found = edb_.get(KList[i], val);
//            if (found) {
//                result.push_back(val);
//                if (deleteFiles) {
//                    edb_.remove(KList[i]);
//                }
//            }
//	    else
//	    {
//		cout <<"will send fake file"<< endl;
//	    }
//        } else {
            val = DictF[KList[i]];
            if (val != NULL) {
                result.emplace_back(val);
            }
//        }
    }
    return result;
}

pair<vector<prf_type>,vector<string>> Server::search(vector<prf_type> KList) {
    vector<prf_type> result;
    vector<string> resfake;
    result.reserve(KList.size());
    resfake.reserve(KList.size());
    prf_type notfound;
    memset(notfound.data(), 0, AES_KEY_SIZE);
    //cout <<"KList size at server:"<< KList.size()<<endl;
    for (unsigned int i = 0; i < KList.size(); i++) {
        prf_type val;
        string fake;
        if (useRocksDB) {
		//cout <<"KList[i]"<< KList[i]<<endl;
            bool found = edb_.get(KList[i], val);
		edbfake_.get(KList[i],fake);
            if (found) {
                result.push_back(val);
		resfake.push_back(fake);
                //if (deleteFiles) {
                    edb_.remove(KList[i]);
		    edbfake_.remove(KList[i]);
                //}
            }
// else ??
        } else {
            val = DictW[KList[i]];
	    fake = DictFake[KList[i]];
            if (val != notfound) {
                result.emplace_back(val);
		resfake.emplace_back(fake);
            }
	    DictW.erase(KList[i]);
	    DictFake.erase(KList[i]);
        }
    }
    return make_pair(result,resfake);
}


