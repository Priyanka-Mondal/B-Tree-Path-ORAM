#include "ORAM.hpp"
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

ORAM::ORAM(int maxSize, bytes<Key> key)
: key(key), rd(), mt(rd()), dis(0, (pow(2, floor(log2(maxSize / Z))) - 1) / 2) {
    AES::Setup();
    depth = floor(log2(maxSize / Z));
    bucketCount = pow(2, depth + 1) - 1;
    blockSize = sizeof (Node); // B
    size_t blockCount = Z * (pow(2, depth + 1) - 1);
    size_t storeBlockSize = IV + AES::GetCiphertextLength(Z * (blockSize));
    size_t storeBlockCount = blockCount;
    clen_size = AES::GetCiphertextLength((blockSize) * Z);
    plaintext_size = (blockSize) * Z;
    store = new RAMStore(storeBlockCount, storeBlockSize);
    cout << "Bucket and block count in ORAM:" << bucketCount<<"|"<<Z<<endl;
    for (size_t i = 0; i < bucketCount; i++) {
        Bucket bucket;
        for (int z = 0; z < Z; z++) {
            bucket[z].id = 0;
            bucket[z].data.resize(blockSize, 0);
        }
        WriteBucket(i, bucket);
    }
}

ORAM::~ORAM() {
    AES::Cleanup();
}

// Fetches the array index a bucket
// that lise on a specific path

int ORAM::GetNodeOnPath(int leaf, int curDepth) {
    leaf += bucketCount / 2;
    for (int d = depth - 1; d >= curDepth; d--) {
        leaf = (leaf + 1) / 2 - 1;
    }

    return leaf;
}

// Write bucket to a single block

block ORAM::SerialiseBucket(Bucket bucket) {
    block buffer;

    for (int z = 0; z < Z; z++) {
        Block b = bucket[z];

        // Write block data
        buffer.insert(buffer.end(), b.data.begin(), b.data.end());
    }

    assert(buffer.size() == Z * (blockSize));

    return buffer;
}

Bucket ORAM::DeserialiseBucket(block buffer) {
    assert(buffer.size() == Z * (blockSize));

    Bucket bucket;

    for (int z = 0; z < Z; z++) {
        Block &block = bucket[z];

        block.data.assign(buffer.begin(), buffer.begin() + blockSize);
        Node* node = convertBlockToNode(block.data);
        block.id = node->key;
        delete node;
        buffer.erase(buffer.begin(), buffer.begin() + blockSize);
    }

    return bucket;
}

Bucket ORAM::ReadBucket(int index) {
    block ciphertext = store->Read(index);
    block buffer = AES::Decrypt(key, ciphertext, clen_size);
    Bucket bucket = DeserialiseBucket(buffer);
    return bucket;
}

void ORAM::WriteBucket(int index, Bucket bucket) {
    block b = SerialiseBucket(bucket);
    block ciphertext = AES::Encrypt(key, b, clen_size, plaintext_size);
    store->Write(index, ciphertext);
}

// Fetches blocks along a path, adding them to the cache

void ORAM::FetchPath(int leaf) {
    readCnt++;
    for (size_t d = 0; d <= depth; d++) {
        int node = GetNodeOnPath(leaf, d);

        if (find(readviewmap.begin(), readviewmap.end(), node) != readviewmap.end()) {
            continue;
        } else {
            readviewmap.push_back(node);
        }

        Bucket bucket = ReadBucket(node);

        for (int z = 0; z < Z; z++) {
            Block &block = bucket[z];

            if (block.id != 0) { // It isn't a dummy block   
                Node* n = convertBlockToNode(block.data);
                if (cache.count(block.id) == 0) {
                    cache.insert(make_pair(block.id, n)); //
                } else {
                    delete n;
                }
            }
        }
    }
}

// Gets a list of blocks on the cache which can be placed at a specific point

std::vector<Bid> ORAM::GetIntersectingBlocks(int x, int curDepth) {
    std::vector<Bid> validBlocks;

    int node = GetNodeOnPath(x, curDepth);
    for (auto b : cache) {
        Bid bid = b.first;
        if (b.second != NULL && GetNodeOnPath(b.second->pos, curDepth) == node) {
            validBlocks.push_back(bid);
            if (validBlocks.size() >= Z) {
                return validBlocks;
            }
        }
    }
    return validBlocks;
}

// Greedily writes blocks from the cache to the tree, pushing blocks as deep into the tree as possible

void ORAM::WritePath(int leaf, int d) {
    // Find blocks that can be on this bucket
    int node = GetNodeOnPath(leaf, d);
    if (find(writeviewmap.begin(), writeviewmap.end(), node) == writeviewmap.end()) {

        auto validBlocks = GetIntersectingBlocks(leaf, d);
        // Write blocks to tree
        Bucket bucket;
        for (int z = 0; z < std::min((int) validBlocks.size(), Z); z++) {
            Block &block = bucket[z];
            block.id = validBlocks[z];
	    Bid temp = block.id;
            Node* curnode = cache[block.id];
            block.data = convertNodeToBlock(curnode);
	    if(curnode->key != block.id)
	    {
	          cout <<"curnode!=block.id"<<curnode->key<<block.id<<endl;
		    block.id = 0; // curnode->key;
		    block.data.resize(blockSize, 0);
        	   //std::fill(curnode->value.begin(), curnode->value.end(), 0);
            	    //block.data = convertNodefToBlock(curnode);
	    }
            delete curnode;
            cache.erase(temp);
        }
        // Fill any empty spaces with dummy blocks
        for (int z = validBlocks.size(); z < Z; z++) {
            Block &block = bucket[z];
            block.id = 0;
            block.data.resize(blockSize, 0);
        }

        // Write bucket to tree
        writeviewmap.push_back(node);
        WriteBucket(node, bucket);
    }
}

// Gets the data of a block in the cache

Node* ORAM::ReadData(Bid bid) {
    if (cache.find(bid) == cache.end()) {
        return NULL;
    }
    return cache[bid];
}

// Updates the data of a block in the cache

void ORAM::WriteData(Bid bid, Node* node) 
{
	if(bid == node->key)
	{
	    if (store->GetEmptySize() > 0) 
	    {
	        cache[bid] = node;
	        store->ReduceEmptyNumbers();
	cout << "EmptyNode Write ORAM src:"<< store->GetEmptySize()<<"/"<< bucketCount*Z<<endl;
	    } 
	    else 
	    {
		    cout << "can not write anymore - WriteData src!"<<endl;
	            throw runtime_error("There is no more space in ORAM src");
            }
	}
	else if(bid != node->key)
	{
        	cache[bid]=node; 
        	store->IncreaseEmptyNumbers();
	cout <<"EmptyNode delete ORAM src:"<< store->GetEmptySize()<<"/"<< bucketCount*Z<<endl;
	}
}


// Fetches a block, allowing you to read and write in a block

void ORAM::Access(Bid bid, Node*& node, int lastLeaf, int newLeaf) {
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

void ORAM::Access(Bid bid, Node*& node) {
    if (!batchWrite) {
        FetchPath(node->pos);
    }
    WriteData(bid, node);
    if (find(leafList.begin(), leafList.end(), node->pos) == leafList.end()) {
        leafList.push_back(node->pos);
    }
}

Node* ORAM::ReadNode(Bid bid) {
    if (bid == 0) {
        throw runtime_error("Node id is not set in ReadNode src");
    }
    if (cache.count(bid) == 0) {
        throw runtime_error("Node not found in the cache src");
    } else {
        Node* node = cache[bid];
        return node;
    }
}

Node* ORAM::ReadNode(Bid bid, int lastLeaf, int newLeaf) {
    if (bid == 0) {
        return NULL;
    }
    if (cache.count(bid) == 0 || find(leafList.begin(), leafList.end(), lastLeaf) == leafList.end()) {
        Node* node;
        Access(bid, node, lastLeaf, newLeaf);
        if (node != NULL) {
            modified.insert(bid);
        }
        return node;
    } else {
        modified.insert(bid);
        Node* node = cache[bid];
        node->pos = newLeaf;
        return node;
    }
}

int ORAM::WriteNode(Bid bid, Node* node) {
    if (bid == 0) {
        throw runtime_error("Node id is not set in WriteNode");
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


int ORAM::DeleteNode(Bid bid, Node* node) {
    cout <<"In deleteNode:"<<bid<<endl;
    if (bid == 0) 
    {
       throw runtime_error("Nodef id is not set in DeleteNode");
    }
    if (cache.count(bid) != 0) 
    {
	    cache.erase(bid);
	    cache[bid] = node;
    }
        //modified.insert(bid);
	//if(modified.count(bid))
	//	modified.erase(bid);
    WriteData(bid, node);
        return node->pos;
}

Node* ORAM::convertBlockToNode(block b) {
    Node* node = new Node();
    std::array<byte_t, sizeof (Node) > arr;
    std::copy(b.begin(), b.begin() + sizeof (Node), arr.begin());
    from_bytes(arr, *node);
    return node;
}

block ORAM::convertNodeToBlock(Node* node) {
    std::array<byte_t, sizeof (Node) > data = to_bytes(*node);
    block b(data.begin(), data.end());
    return b;
}

void ORAM::finilize(bool find, Bid& rootKey, int& rootPos) {
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

    //updating the binary tree positions
    for (unsigned int i = 0; i <= depth + 2; i++) {
        for (auto t : cache) {
            if (t.second != NULL && t.second->height == i) {
                Node* tmp = t.second;
                if (modified.count(tmp->key)) {
                    tmp->pos = RandomPath();
                }
                if (tmp->leftID != 0 && cache.count(tmp->leftID) > 0) {
                    tmp->leftPos = cache[tmp->leftID]->pos;
                }
                if (tmp->rightID != 0 && cache.count(tmp->rightID) > 0) {
                    tmp->rightPos = cache[tmp->rightID]->pos;
                }
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
            WritePath(leafList[i], d);
        }
    }

    leafList.clear();
    modified.clear();

    //Print();
}

void ORAM::start(bool batchWrite) {
    this->batchWrite = batchWrite;
    writeviewmap.clear();
    readviewmap.clear();
    readCnt = 0;
}

void ORAM::Print() {
    for (unsigned int i = 0; i < bucketCount; i++) {
        block ciphertext = store->Read(i);
        block buffer = AES::Decrypt(key, ciphertext, clen_size);
        Bucket bucket = DeserialiseBucket(buffer);
	Node* node;
	for(int j=0; j<Z; j++)
	{
        	node = convertBlockToNode(bucket[j].data);
        	cout << node->key << "]  ";
	}cout <<endl;
        delete node;
    }
        for (auto p : cache)
	{
		cout << "| " <<p.second->key << "] |";
	}cout << endl ;
}

int ORAM::RandomPath() {
    int val = dis(mt);
    return val;
}
