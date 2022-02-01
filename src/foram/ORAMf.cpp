#include "ORAMf.hpp"
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

ORAMf::ORAMf(int maxSize, bytes<Key> key)
: key(key), rd(), mt(rd()), dis(0, (pow(2, floor(log2(maxSize / Z))) - 1) / 2) {
    AES::Setup();
    depth = floor(log2(maxSize / Z));
    bucketCount = pow(2, depth + 1) - 1;
    blockSize = sizeof (Nodef); // B
    size_t blockCount = Z * (pow(2, depth + 1) - 1);
    size_t storeBlockSize = IV + AES::GetCiphertextLength(Z * (blockSize));
    size_t storeBlockCount = blockCount;
    clen_size = AES::GetCiphertextLength((blockSize) * Z);
    plaintext_size = (blockSize) * Z;
    store = new RAMStore(storeBlockCount, storeBlockSize);
    //cout << "Bucket and block count in ORAMf:" << bucketCount<<"|"<<Z<<endl;
    for (size_t i = 0; i < bucketCount; i++) {
        Bucketf bucket;
        for (int z = 0; z < Z; z++) {
            bucket[z].id = 0;
            bucket[z].data.resize(blockSize, 0);
        }
        WriteBucket(i, bucket);
    }
}

ORAMf::~ORAMf() {
    AES::Cleanup();
}

// Fetches the array index a bucket
// that lise on a specific path

int ORAMf::GetNodefOnPath(int leaf, int curDepth) {
    leaf += bucketCount / 2;
    for (int d = depth - 1; d >= curDepth; d--) {
        leaf = (leaf + 1) / 2 - 1;
    }

    return leaf;
}

// Write bucket to a single block

block ORAMf::SerialiseBucket(Bucketf bucket) {
    block buffer;

    for (int z = 0; z < Z; z++) {
        Blockf b = bucket[z];

        // Write block data
        buffer.insert(buffer.end(), b.data.begin(), b.data.end());
    }

    assert(buffer.size() == Z * (blockSize));

    return buffer;
}

Bucketf ORAMf::DeserialiseBucket(block buffer) {
    assert(buffer.size() == Z * (blockSize));

    Bucketf bucket;

    for (int z = 0; z < Z; z++) {
        Blockf &block = bucket[z];

        block.data.assign(buffer.begin(), buffer.begin() + blockSize);
        Nodef* node = convertBlockToNodef(block.data);
        block.id = node->key;
        delete node;
        buffer.erase(buffer.begin(), buffer.begin() + blockSize);
    }

    return bucket;
}

Bucketf ORAMf::ReadBucket(int index) {
    block ciphertext = store->Read(index);
    block buffer = AES::Decrypt(key, ciphertext, clen_size);
    Bucketf bucket = DeserialiseBucket(buffer);
    return bucket;
}

void ORAMf::WriteBucket(int index, Bucketf bucket) {
    block b = SerialiseBucket(bucket);
    block ciphertext = AES::Encrypt(key, b, clen_size, plaintext_size);
    store->Write(index, ciphertext);
}

// Fetches blocks along a path, adding them to the cache

void ORAMf::FetchPath(int leaf) {
    readCnt++;
    for (size_t d = 0; d <= depth; d++) {
        int node = GetNodefOnPath(leaf, d);

        if (find(readviewmap.begin(), readviewmap.end(), node) != readviewmap.end()) {
            continue;
        } else {
            readviewmap.push_back(node);
        }

        Bucketf bucket = ReadBucket(node);

        for (int z = 0; z < Z; z++) {
            Blockf &block = bucket[z];

            if (block.id != 0) { // It isn't a dummy block   
                Nodef* n = convertBlockToNodef(block.data);
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

std::vector<Bid> ORAMf::GetIntersectingBlocks(int x, int curDepth) {
    std::vector<Bid> validBlocks;

    //cout <<"++++++++++++++++++++++++++++++"<<curDepth<<";"<<x<<endl;
    int node = GetNodefOnPath(x, curDepth);
    for (auto b : cache) {
        Bid bid = b.first;
        if (b.second != NULL && GetNodefOnPath(b.second->pos, curDepth) == node) {
	//cout <<"Bid inserted in validBlocks:"<< bid<< "at dep:"<<curDepth<<endl;
            validBlocks.push_back(bid);
            if (validBlocks.size() >= Z) {
                return validBlocks;
            }
        }
    }
    //cout <<"++++++++++++++++++++++++++++++"<<curDepth<<";;"<<x<<endl;
    return validBlocks;
}

// Greedily writes blocks from the cache to the tree, pushing blocks as deep into the tree as possible

void ORAMf::WritePath(int leaf, int d) {
    // Find blocks that can be on this bucket
    int node = GetNodefOnPath(leaf, d);
    if (find(writeviewmap.begin(), writeviewmap.end(), node) == writeviewmap.end()) {
        auto validBlocks = GetIntersectingBlocks(leaf, d);
        // Write blocks to tree
        Bucketf bucket;
        for (int z = 0; z < std::min((int) validBlocks.size(), Z); z++) {
            Blockf &block = bucket[z];
            block.id = validBlocks[z];
	    Bid temp = block.id;
            Nodef* curnode = cache[block.id];
            block.data = convertNodefToBlock(curnode);
	    if(curnode->key != block.id)
	    {
	          cout <<"curnode!=block.idf "<<curnode->key<<block.id<<endl;
		    block.id = 0; // curnode->key;
		    block.data.resize(blockSize, 0);
	    }
            delete curnode;
            cache.erase(temp);
        }
        // Fill any empty spaces with dummy blocks
        for (int z = validBlocks.size(); z < Z; z++) {
            Blockf &block = bucket[z];
	    //cout <<"i am extra"<<endl;
            block.id = 0;
            block.data.resize(blockSize, 0);
        }

        // Write bucket to tree
        writeviewmap.push_back(node);
        WriteBucket(node, bucket);
    }
}

// Gets the data of a block in the cache

Nodef* ORAMf::ReadData(Bid bid) {
    if (cache.find(bid) == cache.end()) {
        return NULL;
    }
    return cache[bid];
}

// Updates the data of a block in the cache

void ORAMf::WriteData(Bid bid, Nodef* node) {
	if(bid == node->key)
	{
	    if (store->GetEmptySize() > 0) 
	    {
	        cache[bid] = node;
	        store->ReduceEmptyNumbers();
	cout << bid<<"EmptyNode write ORAMf upd:"<< store->GetEmptySize() <<"(of "<< bucketCount*Z<<")"<<endl;
	    } 
	    else 
	    {
		    cout << "can not write anymore - WriteDataf upd!"<<endl;
	            throw runtime_error("There is no more space in ORAMf upd");
            }
	}
	else if(bid != node->key)
	{
        	cache[bid]=node; 
        	store->IncreaseEmptyNumbers();
	cout <<bid<<"EmptyNode delete ORAMf upd:"<< store->GetEmptySize()<<"(of "<<bucketCount*Z<<")"<<endl;
	}
}

// Fetches a block, allowing you to read and write in a block

void ORAMf::Access(Bid bid, Nodef*& node, int lastLeaf, int newLeaf) {
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

void ORAMf::Access(Bid bid, Nodef*& node) {
    if (!batchWrite) {
        FetchPath(node->pos);
    }
    //cout <<"In Access"<<endl;
    WriteData(bid, node);
    if (find(leafList.begin(), leafList.end(), node->pos) == leafList.end()) {
        leafList.push_back(node->pos);
    }
}

Nodef* ORAMf::ReadNodef(Bid bid) {
    if (bid == 0) {
        throw runtime_error("Nodef id is not set in ReadNode upd");
    }
    if (cache.count(bid) == 0) {
        throw runtime_error("Nodef not found in the cache upd!!");
    } else {
        Nodef* node = cache[bid];
        return node;
    }
}

Nodef* ORAMf::ReadNodef(Bid bid, int lastLeaf, int newLeaf) {
    if (bid == 0) {
        return NULL;
    }
    if (cache.count(bid) == 0 || find(leafList.begin(), leafList.end(), lastLeaf) == leafList.end()) {
        Nodef* node;
        Access(bid, node, lastLeaf, newLeaf);
        if (node != NULL) {
            modified.insert(bid);
        }
        return node;
    } else {
        modified.insert(bid);
        Nodef* node = cache[bid];
        node->pos = newLeaf;
        return node;
    }
}

int ORAMf::WriteNodef(Bid bid, Nodef* node) {
    if (bid == 0) 
    {
       throw runtime_error("Nodef id is not set in WriteNode");
    }
    if (cache.count(bid) == 0) 
    {
        modified.insert(bid);
        Access(bid, node);
        return node->pos;
    } 
    else 
    {
        modified.insert(bid);
        return node->pos;
    }
}

int ORAMf::DeleteNodef(Bid bid, Nodef* node) {
//    cout <<"In deleteNodef:"<<bid<<endl;
    if (bid == 0) 
    {
       throw runtime_error("Nodef id is not set in DeleteNodef");
    }
    if (cache.count(bid) != 0) 
    {
	    cache.erase(bid);
	    cache[bid] = node;
    }
        //deleted.insert(bid);
        //modified.insert(bid);
	//if(modified.count(bid))
	//	modified.erase(bid);
    WriteData(bid, node);
        return node->pos;
}

Nodef* ORAMf::convertBlockToNodef(block b) {
    Nodef* node = new Nodef();
    std::array<byte_t, sizeof (Nodef) > arr;
    std::copy(b.begin(), b.begin() + sizeof (Nodef), arr.begin());
    from_bytes(arr, *node);
    return node;
}

block ORAMf::convertNodefToBlock(Nodef* node) {
    std::array<byte_t, sizeof (Nodef) > data = to_bytes(*node);
    block b(data.begin(), data.end());
    return b;
}

void ORAMf::finilize(bool find, Bid& rootKey, int& rootPos) {
	//cout <<"Printing ORAM--------------------------"<<endl;
    //Print();
    //fake read for padding     
    if (!batchWrite) {
        if (find) {
            for (unsigned int i = readCnt; i < depth * 1.45; i++) {
                int rnd = RandomPath();
                if (std::find(leafList.begin(), leafList.end(), rnd) == leafList.end()) {
                    leafList.push_back(rnd);
                }
                FetchPath(rnd);
            }
        } else {
            for (int i = readCnt; i < 4.35 * depth; i++) {
                int rnd = RandomPath();
                if (std::find(leafList.begin(), leafList.end(), rnd) == leafList.end()) {
                    leafList.push_back(rnd);
                }
                FetchPath(rnd);
            }
        }
    }
    //cout <<"In Finilize"<<endl;

    //updating the binary tree positions
    for (unsigned int i = 0; i <= depth + 2; i++) {
        for (auto t : cache) {
		//cout <<"depth:"<<i<<"cache:"<<&(t.first)<<"|"<<t.second->key<<endl;
            if (t.second != NULL && t.second->height == i) {
                Nodef* tmp = t.second;
                if (modified.count(tmp->key)) {
                    tmp->pos = RandomPath();
                }
                if (tmp->leftID != 0 && cache.count(tmp->leftID) > 0) {
                    tmp->leftPos = cache[tmp->leftID]->pos;
                }
                if (tmp->rightID != 0 && cache.count(tmp->rightID) > 0) {
                    tmp->rightPos = cache[tmp->rightID]->pos;
                }
		/*if(deleted.count(tmp->key))
		{
			cout <<"deleting node:"<<tmp->key<<endl;
			tmp->pos = RandomPath();
			tmp->leftID = 0;
			tmp->rightID = 0;
		}*/
            }
        }
    }
    if (cache[rootKey] != NULL)
        rootPos = cache[rootKey]->pos;

    int cnt = 0;
    for (int d = depth; d >= 0; d--) {
        for (unsigned int i = 0; i < leafList.size(); i++) {
            cnt++;
            if (cnt % 1000 == 0 && batchWrite) {
                cout << "OMAP:" << cnt << "/" << (depth+1) * leafList.size() << " inserted" << endl;
            }
	    //cout <<"CALLING WRITE PATH:"<<leafList[i]<<"|"<<d<<endl;
            WritePath(leafList[i], d);
        }
    }

    leafList.clear();
    modified.clear();
    //deleted.clear();
    //Print();
    //cout <<"Everything clear ORAM Printed-------------------------"<<endl;
    //cout <<"Printed ORAM--------------------------"<<endl;
}

void ORAMf::start(bool batchWrite) {
    this->batchWrite = batchWrite;
    writeviewmap.clear();
    readviewmap.clear();
    readCnt = 0;
}

void ORAMf::Print() {
    for (unsigned int i = 0; i < bucketCount; i++) {
        block ciphertext = store->Read(i);
        block buffer = AES::Decrypt(key, ciphertext, clen_size);
        Bucketf bucket = DeserialiseBucket(buffer);
	Nodef* node;
	for(int j=0; j<Z; j++)
	{
        	node = convertBlockToNodef(bucket[j].data);
        	cout << node->key << " ";
	}cout <<endl;
        delete node;
    }
    for (auto p : cache)
	{
		cout << p.second->key << "|";
	}cout << endl ;
}

int ORAMf::RandomPath() {
    int val = dis(mt);
    return val;
}
