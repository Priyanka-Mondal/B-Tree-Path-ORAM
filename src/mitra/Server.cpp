#include "Server.h"
#include <iostream>
#include <sstream>
/*
#include <cryptopp/rng.h>
#include <cryptopp/sha.h>
#include <cryptopp/hex.h>
#include <cryptopp/modes.h>
#include <cryptopp/aes.h>
*/
#include <vector>
#include "utils/Utilities.h"

Server::Server(bool useHDD, bool deleteFiles) : edb_("DBmitra") 
{
    this->deleteFiles = deleteFiles;
    this->useRocksDB = true;
}

Server::~Server() { }

void Server::update(prf_type addr, prf_type val) 
{
        edb_.put(addr, val);
}

vector<prf_type> Server::search(vector<prf_type> KList) 
{
    vector<prf_type> result;
    for (unsigned int i = 0; i < KList.size(); i++) 
    {
        prf_type val;
        bool found = edb_.get(KList[i], val);
        if (found) 
	{
		result.push_back(val);
                if (deleteFiles) 
                    edb_.remove(KList[i]);
        }
    }
    return result;
}


