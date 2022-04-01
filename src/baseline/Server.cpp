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

Server::Server(bool deleteFiles) 
{
    this->deleteFiles = deleteFiles;
}

Server::~Server() { }

void Server::update(prf_type addr, FileNode* val) 
{
        DictF[addr]= val;
}

vector<FileNode*> Server::search(vector<prf_type> KList) 
{
    vector<FileNode*> result;
    for (unsigned int i = 0; i < KList.size(); i++) 
		result.emplace_back(DictF[KList[i]]);
    return result;
}


