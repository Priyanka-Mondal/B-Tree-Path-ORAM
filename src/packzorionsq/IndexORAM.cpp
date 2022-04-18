#include "IndexORAM.hpp"
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

IndexORAM::IndexORAM(int maxSize, bytes<Key> key)
: key(key), rd(), mt(rd()), dis(0, (pow(2, floor(log2(maxSize / Z)) + 1) - 1) / 2) {
    AES::Setup();
    depth = floor(log2(maxSize / Z));
    leaves = (pow(2, floor(log2(maxSize/Z))+1)-1)/2;
    cout <<"depth of index tree:"<<depth<<endl;
    cout <<"total index leaves:"<< leaves<<endl;
    bucketCount = pow(2, depth + 1) - 1;
    blockSize = sizeof (Node); // B
    size_t blockCount = Z * (pow(2, depth + 1) - 1);
    size_t storeIblockSize = IV + AES::GetCiphertextLength(Z * (blockSize));
    size_t storeIblockCount = blockCount;
    clen_size = AES::GetCiphertextLength((blockSize) * Z);
    plaintext_size = (blockSize) * Z;
    store = new RAMStore(storeIblockCount, storeIblockSize);
    cout << "Ibuckets:"<<bucketCount<<" blockCount:"<<blockCount<<endl;
    for (size_t i = 0; i < bucketCount; i++) {
        Ibucket bucket;
        for (int z = 0; z < Z; z++) {
            bucket[z].id = 0;
            bucket[z].data.resize(blockSize, 0);
        }
        WriteIbucket(i, bucket);
    }
}

IndexORAM::~IndexORAM() {
    AES::Cleanup();
    delete store;
}


int IndexORAM::GetNodeOnPath(int leaf, int curDepth) {
    leaf += bucketCount / 2;
    for (int d = depth - 1; d >= curDepth; d--) {
        leaf = (leaf + 1) / 2 - 1;
    }

    return leaf;
}

// Write bucket to a single block

block IndexORAM::SerialiseIbucket(Ibucket bucket) {
    block buffer;

    for (int z = 0; z < Z; z++) {
        Iblock b = bucket[z];
        buffer.insert(buffer.end(), b.data.begin(), b.data.end());
    //delete b;
    }

    assert(buffer.size() == Z * (blockSize));
    
    return buffer;
}

Ibucket IndexORAM::DeserialiseIbucket(block buffer) {
    assert(buffer.size() == Z * (blockSize));

    Ibucket bucket;

    for (int z = 0; z < Z; z++) {
        Iblock &block = bucket[z];

        block.data.assign(buffer.begin(), buffer.begin() + blockSize);
        Node* node = convertIblockToNode(block.data);
        block.id = node->key;
        delete node;
        buffer.erase(buffer.begin(), buffer.begin() + blockSize);
    }

    return bucket;
}

Ibucket IndexORAM::ReadIbucket(int index) {
    block ciphertext = store->Read(index);
    block buffer = AES::Decrypt(key, ciphertext, clen_size);
    Ibucket bucket = DeserialiseIbucket(buffer);
    ciphertext.clear();
    buffer.clear();
    return bucket;
}

void IndexORAM::WriteIbucket(int index, Ibucket bucket) {
    block b = SerialiseIbucket(bucket);
    block ciphertext = AES::Encrypt(key, b, clen_size, plaintext_size);
    store->Write(index, ciphertext);
    ciphertext.clear();
    b.clear();
}

// Fetches blocks along a path, adding them to the cache

void IndexORAM::FetchPath(int leaf) {
	if(leaf >= leaves)
	{
        	throw runtime_error("leaf OUT OF RANGE");
	}
    for (size_t d = 0; d <= depth; d++) {
        int node = GetNodeOnPath(leaf, d);

        if (find(readviewmap.begin(), readviewmap.end(), node) != readviewmap.end()) {
            continue;
        } else {
            readviewmap.push_back(node);
        }

        Ibucket bucket = ReadIbucket(node);
	searchi_bytes = searchi_bytes+clen_size;

        for (int z = 0; z < Z; z++) {
            Iblock &block = bucket[z];

            if (block.id != 0) { // It isn't a dummy block   
                Node* n = convertIblockToNode(block.data);
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

std::vector<Bid> IndexORAM::GetIntersectingIblocks(int x, int curDepth) {
    std::vector<Bid> validIblocks;

    int node = GetNodeOnPath(x, curDepth);
    for (auto b : cache) {
        Bid bid = b.first;
        if (b.second != NULL && GetNodeOnPath(b.second->pos, curDepth) == node) {
            validIblocks.push_back(bid);
            if (validIblocks.size() >= Z) {
                return validIblocks;
            }
        }
    }
    return validIblocks;
}


void IndexORAM::WritePath(int leaf, int d) 
{
    int node = GetNodeOnPath(leaf, d);
    if (find(writeviewmap.begin(), writeviewmap.end(), node) == writeviewmap.end()) 
    {
        auto validIblocks = GetIntersectingIblocks(leaf, d);
        Ibucket bucket;
        for (int z = 0; z < std::min((int) validIblocks.size(), Z); z++) 
	{
            Iblock &block = bucket[z];
            block.id = validIblocks[z];
	    Bid temp = block.id;
            Node* curnode = cache[block.id];
            block.data = convertNodeToIblock(curnode);
	    if(curnode->key != block.id)
	    {
		    block.id = 0; // curnode->key;
		    block.data.resize(blockSize, 0);
	    }
            delete curnode;
            cache.erase(temp);
        }
        for (int z = validIblocks.size(); z < Z; z++) 
	{
            Iblock &block = bucket[z];
            block.id = 0;
            block.data.resize(blockSize, 0);
        }
        writeviewmap.push_back(node);
        WriteIbucket(node, bucket);
    }
}

// Gets the data of a block in the cache

Node* IndexORAM::ReadData(Bid bid) {
    if (cache.find(bid) == cache.end()) {
        return NULL;
    }
    return cache[bid];
}

// Updates the data of a block in the cache

void IndexORAM::WriteData(Bid bid, Node* node) 
{
    if (store->GetEmptySize() > 0) {
        cache[bid] = node;
        store->ReduceEmptyNumbers();
    } else {
        throw runtime_error("There is no more space in IndexORAM-WriteData");
    }
}

void IndexORAM::DeleteData(Bid bid, Node* node) 
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

void IndexORAM::Access(Bid bid, Node*& node, int lastLeaf, int newLeaf) 
{
    FetchPath(lastLeaf);
    node = ReadData(bid);
    if (node != NULL) 
    {
        node->pos = newLeaf;
        /*if (cache.count(bid) != 0) 
	{
            cache.erase(bid);
        }*/
        cache[bid] = node;
        if (find(leafList.begin(), leafList.end(), lastLeaf) == leafList.end()) {
            leafList.push_back(lastLeaf);
        }
    }
}

void IndexORAM::Access(Bid bid, Node*& node) 
{
    //if (!batchWrite) {
        FetchPath(node->pos);
   // }
    WriteData(bid, node);
    if (find(leafList.begin(), leafList.end(), node->pos) == leafList.end()) {
        leafList.push_back(node->pos);
    }
}

Node* IndexORAM::ReadNode(Bid bid) {
    if (bid == 0) {
        throw runtime_error("Node id is not set ReadNode");
    }
    if (cache.count(bid) == 0) {
        throw runtime_error("Node not found in the cache ReadNode");
    } else {
        Node* node = cache[bid];
        return node;
    }
}

Node* IndexORAM::ReadNode(Bid bid, int lastLeaf, int newLeaf) 
{
    if (bid == 0) 
    {
        return NULL;
    }
    /*
    if(cache.count(bid)>0)
    {
	Node* node = cache[bid];
	node->pos = newLeaf;
	cache[bid]=node;
        modified.insert(bid);
	return node;
    }*/
    if(cache.count(bid)==0)//||find(leafList.begin(),leafList.end(),lastLeaf)==leafList.end())
    {
        Node* node;
        Access(bid, node, lastLeaf, newLeaf);
        //if (node != NULL) 
	//{
         //   modified.insert(bid);
        //}
	//else 
	if(node == NULL)
	{
		cout <<"Node is NULL : "<< bid << endl ;
		cout <<"free node:" << store->GetEmptySize() << endl;
	}
        return node;
    } 
    else 
    {
        //modified.insert(bid);
        Node* node = cache[bid];
        node->pos = newLeaf;
	cache[bid] = node;
        return node;
    }
}

int IndexORAM::WriteNode(Bid bid, Node* node) {
    
    if (bid == 0) 
    {
	cout <<bid<<endl;
        throw runtime_error("Node id is not set WriteNode");
    }
    if (cache.count(bid) == 0) 
    {
        modified.insert(bid);
        Access(bid, node);
        return node->pos;
    } else 
    {
        modified.insert(bid);
        return node->pos;
    }
}


int IndexORAM::DeleteNode(Bid bid, Node* node) {
    if (bid == 0) 
    {
       throw runtime_error("Nodef id is not set in DeleteNode");
    }
    if (cache.count(bid) != 0) 
    {
	    cache.erase(bid);
	    cache[bid] = node;
    }
    DeleteData(bid, node);
        return node->pos;
}

Node* IndexORAM::convertIblockToNode(block b) {
    Node* node = new Node();
    std::array<byte_t, sizeof (Node) > arr;
    std::copy(b.begin(), b.begin() + sizeof (Node), arr.begin());
    from_bytes(arr, *node);
    return node;
}

void IndexORAM::convertIblockToNode(Node*& node, block b) {
    //Node* node = new Node();
    std::array<byte_t, sizeof (Node) > arr;
    std::copy(b.begin(), b.begin() + sizeof (Node), arr.begin());
    from_bytes(arr, *node);
    //return node;
}

block IndexORAM::convertNodeToIblock(Node* node) {
    std::array<byte_t, sizeof (Node) > data = to_bytes(*node);
    block b(data.begin(), data.end());
    return b;
}

void IndexORAM::WriteCache() 
{
        for (unsigned int i = 0; i < leafList.size(); i++) 
	{
    		for (int d = depth; d >= 0; d--) 
    		{
            		WritePath(leafList[i], d);
        	}
    }

    leafList.clear();
    modified.clear();
}

void IndexORAM::finalize() 
{
    for (int d = depth; d >= 0; d--) 
    {
        for (unsigned int i = 0; i < leafList.size(); i++) 
	{
            WritePath(leafList[i], d);
        }
    }
    leafList.clear();
    modified.clear();
}
void IndexORAM::finalizeI() 
{
    vector<Node*> nodes;
    nodes.reserve(cache.size());
    for (auto t : cache) 
    {
	    nodes.push_back(t.second);
    }
    setupInsert(nodes);
    leafList.clear();
    modified.clear();
}
/*
void IndexORAM::finalizefile() 
{
    for (unsigned int i = maxHeight; i >= 1; i--) {
        for (auto t : cache) {
            if (t.second != NULL && t.second->height == i) {
                Node* tmp = t.second;
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

    for (int d = depth; d >= 0; d--) 
    {
        for (unsigned int i = 0; i < leafList.size(); i++) 
	{
            WritePath(leafList[i], d);
        }
    }
    leafList.clear();
    modified.clear();
}
*/
void IndexORAM::start(bool batchWrite) {
    this->batchWrite = batchWrite;
    writeviewmap.clear();
    readviewmap.clear();
}

void IndexORAM::Print() {
    for (unsigned int i = 0; i < bucketCount; i++) {
        block ciphertext = store->Read(i);
        block buffer = AES::Decrypt(key, ciphertext, clen_size);
        Ibucket bucket = DeserialiseIbucket(buffer);
        Node* node = convertIblockToNode(bucket[0].data);
        cout << node->key << " ";
        delete node;
    }
}

int IndexORAM::RandomPath() 
{
    int val = dis(mt);
    return val;
}

void IndexORAM::setupInsert(vector<Node*> nodes) {
    sort(nodes.begin(), nodes.end(), [ ](const Node* lhs, const Node * rhs) {
        return lhs->pos < rhs->pos;
    });
    int curPos = 0;
    if (nodes.size() > 0) {
        curPos = nodes[0]->pos;
    }
    map<int, Ibucket> buckets;
    map<int, int> bucketsCnt;
    int cnt = 0;
    unsigned int i = 0;
    bool cannotInsert = false;
    while (i < nodes.size()) {
        cnt++;
        //if (cnt % 1000 == 0) {
         //   cout << "i:" << i << "/" << nodes.size() << endl;
        //}
        for (int d = depth; d >= 0 && i < nodes.size() && curPos == nodes[i]->pos; d--) {
            int nodeIndex = GetNodeOnPath(curPos, d);
            Ibucket bucket;
            if (bucketsCnt.count(nodeIndex) == 0) {
                bucketsCnt[nodeIndex] = 0;
                for (int z = 0; z < Z; z++) {
                    if (i < nodes.size() && nodes[i]->pos == curPos) {
                        Iblock &curIblock = bucket[z];
                        curIblock.id = nodes[i]->key;
                        curIblock.data = convertNodeToIblock(nodes[i]);
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
                            Iblock &curIblock = bucket[z];
                            curIblock.id = nodes[i]->key;
                            curIblock.data = convertNodeToIblock(nodes[i]);
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
            WriteIbucket(buk.first, buk.second);
        } else {
            for (long unsigned int z = bucketsCnt[buk.first]; z < Z; z++) {
                Iblock &curIblock = buk.second[z];
                curIblock.id = 0;
                curIblock.data.resize(blockSize, 0);
            }
            WriteIbucket(buk.first, buk.second);
        }
    }

    for (; i < nodes.size(); i++) {
        cache[nodes[i]->key] = nodes[i];
    }
}
