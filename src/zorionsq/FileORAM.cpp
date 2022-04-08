#include "FileORAM.hpp"
#include "../utils/Utilities.h"
#include <algorithm>
#include <iomanip>
#include <fstream>
#include <random>
#include <cmath>
#include <cassert>
#include <cstring>
#include <map>
#include <stdexcept>

FileORAM::FileORAM(int maxSize, bytes<Key> key)
: key(key), rd(), mt(rd()), dis(0, (pow(2, floor(log2(maxSize / Z)) + 1) - 1) / 2) {
    AES::Setup();
    depth = floor(log2(maxSize / Z));
    cout <<"depth of tree:"<<depth<<endl;
    bucketCount = pow(2, depth + 1) - 1;
    blockSize = sizeof (Fnode); // B
    size_t blockCount = Z * (pow(2, depth + 1) - 1);
    size_t storeFblockSize = IV + AES::GetCiphertextLength(Z * (blockSize));
    size_t storeFblockCount = blockCount;
    clen_size = AES::GetCiphertextLength((blockSize) * Z);
    plaintext_size = (blockSize) * Z;
    store = new RAMStore(storeFblockCount, storeFblockSize);
    cout << "Fbuckets:"<<bucketCount<<" blockCount:"<<blockCount<<endl;
    for (size_t i = 0; i < bucketCount; i++) {
        Fbucket bucket;
        for (int z = 0; z < Z; z++) {
            bucket[z].id = 0;
            bucket[z].data.resize(blockSize, 0);
        }
        WriteFbucket(i, bucket);
    }
}

FileORAM::~FileORAM() {
    AES::Cleanup();
    delete store;
}


int FileORAM::GetFnodeOnPath(int leaf, int curDepth) {
    leaf += bucketCount / 2;
    for (int d = depth - 1; d >= curDepth; d--) {
        leaf = (leaf + 1) / 2 - 1;
    }

    return leaf;
}

// Write bucket to a single block

block FileORAM::SerialiseFbucket(Fbucket bucket) {
    block buffer;

    for (int z = 0; z < Z; z++) {
        Fblock b = bucket[z];
        buffer.insert(buffer.end(), b.data.begin(), b.data.end());
    //delete b;
    }

    assert(buffer.size() == Z * (blockSize));
    
    return buffer;
}

Fbucket FileORAM::DeserialiseFbucket(block buffer) {
    assert(buffer.size() == Z * (blockSize));

    Fbucket bucket;

    for (int z = 0; z < Z; z++) {
        Fblock &block = bucket[z];

        block.data.assign(buffer.begin(), buffer.begin() + blockSize);
        Fnode* node = convertFblockToFnode(block.data);
        block.id = node->key;
        delete node;
        buffer.erase(buffer.begin(), buffer.begin() + blockSize);
    }

    return bucket;
}

Fbucket FileORAM::ReadFbucket(int index) {
    block ciphertext = store->Read(index);
    block buffer = AES::Decrypt(key, ciphertext, clen_size);
    Fbucket bucket = DeserialiseFbucket(buffer);
    ciphertext.clear();
    buffer.clear();
    return bucket;
}

void FileORAM::WriteFbucket(int index, Fbucket bucket) {
    block b = SerialiseFbucket(bucket);
    block ciphertext = AES::Encrypt(key, b, clen_size, plaintext_size);
    store->Write(index, ciphertext);
    ciphertext.clear();
    b.clear();
}

// Fetches blocks along a path, adding them to the cache

void FileORAM::FetchPath(int leaf) {
    readCnt++;
    for (size_t d = 0; d <= depth; d++) {
        int node = GetFnodeOnPath(leaf, d);

        if (find(readviewmap.begin(), readviewmap.end(), node) != readviewmap.end()) {
            continue;
        } else {
            readviewmap.push_back(node);
        }

        Fbucket bucket = ReadFbucket(node);

        for (int z = 0; z < Z; z++) {
            Fblock &block = bucket[z];

            if (block.id != 0) { // It isn't a dummy block   
                Fnode* n = convertFblockToFnode(block.data);
                if (cache.count(block.id) == 0) {
                    cache.insert(make_pair(block.id, n));
                } else {
                    delete n;
                }
            }
        }
    }
}

// Gets a list of blocks on the cache which can be placed at a specific point

std::vector<Fbid> FileORAM::GetIntersectingFblocks(int x, int curDepth) {
    std::vector<Fbid> validFblocks;

    int node = GetFnodeOnPath(x, curDepth);
    for (auto b : cache) {
        Fbid bid = b.first;
        if (b.second != NULL && GetFnodeOnPath(b.second->pos, curDepth) == node) {
            validFblocks.push_back(bid);
            if (validFblocks.size() >= Z) {
                return validFblocks;
            }
        }
    }
    return validFblocks;
}


void FileORAM::WritePath(int leaf, int d) 
{
    int node = GetFnodeOnPath(leaf, d);
    if (find(writeviewmap.begin(), writeviewmap.end(), node) == writeviewmap.end()) 
    {
        auto validFblocks = GetIntersectingFblocks(leaf, d);
        Fbucket bucket;
        for (int z = 0; z < std::min((int) validFblocks.size(), Z); z++) 
	{
            Fblock &block = bucket[z];
            block.id = validFblocks[z];
	    Fbid temp = block.id;
            Fnode* curnode = cache[block.id];
            block.data = convertFnodeToFblock(curnode);
	    if(curnode->key != block.id)
	    {
		    block.id = 0; // curnode->key;
		    block.data.resize(blockSize, 0);
	    }
            delete curnode;
            cache.erase(temp);
        }
        for (int z = validFblocks.size(); z < Z; z++) 
	{
            Fblock &block = bucket[z];
            block.id = 0;
            block.data.resize(blockSize, 0);
        }
        writeviewmap.push_back(node);
        WriteFbucket(node, bucket);
    }
}

// Gets the data of a block in the cache

Fnode* FileORAM::ReadData(Fbid bid) {
    if (cache.find(bid) == cache.end()) {
        return NULL;
    }
    return cache[bid];
}

// Updates the data of a block in the cache

void FileORAM::WriteData(Fbid bid, Fnode* node) 
{
    if (store->GetEmptySize() > 0) {
        cache[bid] = node;
        store->ReduceEmptyNumbers();
    } else {
        throw runtime_error("There is no more space in FileORAM-WriteData");
    }
}

void FileORAM::DeleteData(Fbid bid, Fnode* node) 
{
	if(bid != node->key)
	{
        	cache[bid]=node; 
        	store->IncreaseEmptyNumbers();
		int ret = store->GetEmptySize();
		cout <<bid<<"empty nodes:"<< ret <<endl;
	}
}

// Fetches a block, allowing you to read and write in a block

void FileORAM::Access(Fbid bid, Fnode*& node, int lastLeaf, int newLeaf) {
    FetchPath(lastLeaf);
    node = ReadData(bid);
    if (node != NULL) {
        node->pos = newLeaf;
        if (cache.count(bid) != 0) {
            cache.erase(bid);
        }
        cache[bid] = node;
        if (find(leafList.begin(), leafList.end(), lastLeaf) == leafList.end()) {
            leafList.push_back(lastLeaf);
        }
    }
}

void FileORAM::Access(Fbid bid, Fnode*& node) {
    //if (!batchWrite) {
        FetchPath(node->pos);
   // }
    WriteData(bid, node);
    if (find(leafList.begin(), leafList.end(), node->pos) == leafList.end()) {
        leafList.push_back(node->pos);
    }
}

Fnode* FileORAM::ReadFnode(Fbid bid) {
    if (bid == 0) {
        throw runtime_error("Fnode id is not set ReadFnode");
    }
    if (cache.count(bid) == 0) {
        throw runtime_error("Fnode not found in the cache ReadFnode");
    } else {
        Fnode* node = cache[bid];
        return node;
    }
}

Fnode* FileORAM::ReadFnode(Fbid bid, int lastLeaf, int newLeaf) {
    if (bid == 0) {
        return NULL;
    }
    if (cache.count(bid) == 0 || find(leafList.begin(), leafList.end(), lastLeaf) == leafList.end()) {
        Fnode* node;
        Access(bid, node, lastLeaf, newLeaf);
        if (node != NULL) {
            modified.insert(bid);
        }
	else 
	{
		cout <<"Fnode is NULL : "<< bid << endl ;
		cout <<"free node:" << store->GetEmptySize() << endl;
	}
        return node;
    } else {
        modified.insert(bid);
        Fnode* node = cache[bid];
        node->pos = newLeaf;
        return node;
    }
}

int FileORAM::WriteFnode(Fbid bid, Fnode* node) {
    
    if (bid == 0) 
    {
	cout <<bid<<endl;
        throw runtime_error("Fnode id is not set WriteFnode");
    }
    if (cache.count(bid) == 0) {
        modified.insert(bid);
        Access(bid, node);
        return node->pos;
    } else {
        modified.insert(bid);
        return node->pos;
    }
}


int FileORAM::DeleteFnode(Fbid bid, Fnode* node) {
    if (bid == 0) 
    {
       throw runtime_error("Fnodef id is not set in DeleteFnode");
    }
    if (cache.count(bid) != 0) 
    {
	    cache.erase(bid);
	    cache[bid] = node;
    }
    DeleteData(bid, node);
        return node->pos;
}

Fnode* FileORAM::convertFblockToFnode(block b) {
    Fnode* node = new Fnode();
    std::array<byte_t, sizeof (Fnode) > arr;
    std::copy(b.begin(), b.begin() + sizeof (Fnode), arr.begin());
    from_bytes(arr, *node);
    return node;
}

void FileORAM::convertFblockToFnode(Fnode*& node, block b) {
    //Fnode* node = new Fnode();
    std::array<byte_t, sizeof (Fnode) > arr;
    std::copy(b.begin(), b.begin() + sizeof (Fnode), arr.begin());
    from_bytes(arr, *node);
    //return node;
}

block FileORAM::convertFnodeToFblock(Fnode* node) {
    std::array<byte_t, sizeof (Fnode) > data = to_bytes(*node);
    block b(data.begin(), data.end());
    return b;
}

void FileORAM::finalize() {
        int maxHeight = 1;
        for (auto t : cache) {
            if (t.second != NULL && t.second->height > maxHeight) {
                maxHeight = t.second->height;
            }
        }
    //for (unsigned int i = 0; i <= depth + 2; i++) {
    for (unsigned int i = maxHeight; i >= 1; i--) {
        for (auto t : cache) {
            if (t.second != NULL && t.second->height == i) {
                Fnode* tmp = t.second;
                if (modified.count(tmp->key)) {
                    tmp->pos = RandomPath();
		    if (i==1)
			localBCNT[t.first] = make_pair(localBCNT[t.first].first,tmp->pos);
                }
                if (tmp->nextID != 0 && cache.count(tmp->nextID) > 0) {
                    tmp->nextPos = cache[tmp->nextID]->pos;
                }
            }
        }
    }
    //if (cache[rootKey] != NULL)
      //  rootPos = cache[rootKey]->pos;

    for (int d = depth; d >= 0; d--) {
        for (unsigned int i = 0; i < leafList.size(); i++) {
            WritePath(leafList[i], d);
        }
    }

    leafList.clear();
    modified.clear();
}

void FileORAM::start(bool batchWrite) {
    this->batchWrite = batchWrite;
    writeviewmap.clear();
    readviewmap.clear();
    readCnt = 0;
}

void FileORAM::Print() {
    for (unsigned int i = 0; i < bucketCount; i++) {
        block ciphertext = store->Read(i);
        block buffer = AES::Decrypt(key, ciphertext, clen_size);
        Fbucket bucket = DeserialiseFbucket(buffer);
        Fnode* node = convertFblockToFnode(bucket[0].data);
        cout << node->key << " ";
        delete node;
    }
}

int FileORAM::RandomPath() {
    int val = dis(mt);
    return val;
}
void FileORAM::setupInsert(vector<Fnode*> nodes) {
    sort(nodes.begin(), nodes.end(), [ ](const Fnode* lhs, const Fnode * rhs) {
        return lhs->pos < rhs->pos;
    });
    int curPos = 0;
    if (nodes.size() > 0) {
        curPos = nodes[0]->pos;
    }
    map<int, Fbucket> buckets;
    map<int, int> bucketsCnt;
    int cnt = 0;
    unsigned int i = 0;
    bool cannotInsert = false;
    while (i < nodes.size()) {
        cnt++;
        if (cnt % 1000 == 0) {
            cout << "i:" << i << "/" << nodes.size() << endl;
        }
        for (int d = depth; d >= 0 && i < nodes.size() && curPos == nodes[i]->pos; d--) {
            int nodeIndex = GetFnodeOnPath(curPos, d);
            Fbucket bucket;
            if (bucketsCnt.count(nodeIndex) == 0) {
                bucketsCnt[nodeIndex] = 0;
                for (int z = 0; z < Z; z++) {
                    if (i < nodes.size() && nodes[i]->pos == curPos) {
                        Fblock &curFblock = bucket[z];
                        curFblock.id = nodes[i]->key;
                        curFblock.data = convertFnodeToFblock(nodes[i]);
                        delete nodes[i];
                        bucketsCnt[nodeIndex]++;
                        i++;
                    }
                }
                buckets[nodeIndex] = bucket;
            } else {
                if (bucketsCnt[nodeIndex] < Z) {
                    bucket = buckets[nodeIndex];
                    for (int z = bucketsCnt[nodeIndex]; z < Z; z++) {
                        if (i < nodes.size() && nodes[i]->pos == curPos) {
                            Fblock &curFblock = bucket[z];
                            curFblock.id = nodes[i]->key;
                            curFblock.data = convertFnodeToFblock(nodes[i]);
                            delete nodes[i];
                            bucketsCnt[nodeIndex]++;
                            i++;
                        }
                    }
                    buckets[nodeIndex] = bucket;
                } else {
                    cannotInsert = true;
                }
            }

        }

        if (i < nodes.size()) {
            if (cannotInsert) {
                cache[nodes[i]->key] = nodes[i];
                i++;
                cannotInsert = false;
            }
            if (i < nodes.size()) {
                curPos = nodes[i]->pos;
            }
        }
    }


    for (auto buk : buckets) {
        if (bucketsCnt[buk.first] == Z) {
            WriteFbucket(buk.first, buk.second);
        } else {
            for (long unsigned int z = bucketsCnt[buk.first]; z < Z; z++) {
                Fblock &curFblock = buk.second[z];
                curFblock.id = 0;
                curFblock.data.resize(blockSize, 0);
            }
            WriteFbucket(buk.first, buk.second);
        }
    }

    for (; i < nodes.size(); i++) {
        cache[nodes[i]->key] = nodes[i];
    }
}
