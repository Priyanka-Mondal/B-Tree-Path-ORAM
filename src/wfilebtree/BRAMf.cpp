#include "BRAMf.hpp"
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

BRAMf::BRAMf(int maxSize, bytes<Key> key)
: key(key), rd(), mt(rd()), dis(0, (pow(2, floor(log2(maxSize / Z)) + 1) - 1) / 2) 
{
    AES::Setup();
    depth = floor(log2(maxSize / Z));
    cout <<"depth of tree:"<<depth<<endl;
    pad = floor((log2((maxSize+1)/(2*Z)))/(log2(T)));
    cout <<"worst case height of BTree:"<<pad<<endl;
    bucketCount = pow(2, depth + 1) - 1;
    blockSize = sizeof (BTreeNodef); //??
    size_t blockCount = Z * (pow(2, depth + 1) - 1);
    size_t storeBlockSize = IV + AES::GetCiphertextLength(Z * (blockSize));
    size_t storeBlockCount = blockCount;
    clen_size = AES::GetCiphertextLength((blockSize) * Z);
    plaintext_size = (blockSize) * Z;
    store = new RAMStore(storeBlockCount, storeBlockSize);
    cout << "Buckets:"<<bucketCount<<" block count in BRAMf:"<<blockCount<<endl;
    for (size_t i = 0; i < bucketCount; i++) 
    {
        Bucket bucket;
        for (int z = 0; z < Z; z++) 
	{
            bucket[z].id = 0;
            bucket[z].data.resize(blockSize, 0);
        }
        WriteBucket(i, bucket);
    }
}

BRAMf::~BRAMf() {
    AES::Cleanup();
    delete store;
}

int BRAMf::GetBTreeNodefOnPath(int leaf, int curDepth) {
    leaf += bucketCount / 2;
    for (int d = depth - 1; d >= curDepth; d--) {
        leaf = (leaf + 1) / 2 - 1;
    }
    return leaf;
}

block BRAMf::SerialiseBucket(Bucket bucket) 
{
    block buffer;
    for (int z = 0; z < Z; z++) 
    {
        Block b = bucket[z];
        buffer.insert(buffer.end(), b.data.begin(), b.data.end());
    }
    assert(buffer.size() == Z * (blockSize));
    return buffer;
}

Bucket BRAMf::DeserialiseBucket(block buffer) 
{
    assert(buffer.size() == Z * (blockSize));
    Bucket bucket;
    for (int z = 0; z < Z; z++) 
    {
        Block &block = bucket[z];
        block.data.assign(buffer.begin(), buffer.begin() + blockSize);
        //BTreeNodef* node = convertBlockToBTreeNodef(block.data);
        //block.id = node->bid;
        //delete node; //why cant I delete this
	int bID;
	getbid(block.data,bID);
	block.id = bID;
        buffer.erase(buffer.begin(), buffer.begin() + blockSize);
    }
    return bucket;
}

Bucket BRAMf::ReadBucket(int index) {
    block ciphertext = store->Read(index);
    block buffer = AES::Decrypt(key, ciphertext, clen_size);
    Bucket bucket = DeserialiseBucket(buffer);
    return bucket;
}

void BRAMf::WriteBucket(int index, Bucket bucket) 
{
    block b = SerialiseBucket(bucket);
    block ciphertext = AES::Encrypt(key, b, clen_size, plaintext_size);
    store->Write(index, ciphertext);
}


void BRAMf::FetchPath(int leaf) 
{
    readCnt++;
    for (size_t d = 0; d <= depth; d++) 
    {
        int node = GetBTreeNodefOnPath(leaf, d);
        if (find(readviewmap.begin(), readviewmap.end(), node) != readviewmap.end()) 
	{
            continue;
        } 
	else 
	{
            readviewmap.push_back(node);
        }

        Bucket bucket = ReadBucket(node); //<<-- here
	searchf_bytes = searchf_bytes + clen_size;
	cout <<"searchf:"<<searchf_bytes<<endl;
        for (int z = 0; z < Z; z++) {
            Block &block = bucket[z];

            if (block.id != 0) { // 0 is root right ? or maybe makeit 1
                BTreeNodef* n = convertBlockToBTreeNodef(block.data);
                if (cache.count(block.id) == 0) 
		{
                    cache.insert(make_pair(block.id, n));
                } 
		else 
		{
		//	cout <<"DELETING n----------------------"<<endl;
                    delete n;
                }
            }
        }
    }
}


std::vector<int> BRAMf::GetIntersectingBlocks(int x, int curDepth) {
    std::vector<int> validBlocks;
    int node = GetBTreeNodefOnPath(x, curDepth);
    for (auto b : cache) {
        int bid = b.first;
        if (b.second != NULL && GetBTreeNodefOnPath(b.second->pos, curDepth) == node) 
	{
            validBlocks.push_back(bid);
            if (validBlocks.size() >= Z) 
	    {
                return validBlocks;
            }
        }
    }
    return validBlocks;
}


void BRAMf::WritePath(int leaf, int d) 
{
    int node = GetBTreeNodefOnPath(leaf, d);
    if (find(writeviewmap.begin(), writeviewmap.end(), node) == writeviewmap.end()) 
    {
        auto validBlocks = GetIntersectingBlocks(leaf, d);
        Bucket bucket;
        for (int z = 0; z < std::min((int) validBlocks.size(), Z); z++) 
	{
            Block &block = bucket[z];
            block.id = validBlocks[z];
	    //int temp = block.id;
            BTreeNodef* curnode = cache[block.id];
            block.data = convertBTreeNodefToBlock(curnode);
	    /*if(curnode->bid != block.id)
	    {
		    block.id = 0; // curnode->key;
		    block.data.resize(blockSize, 0);
	    }*/
            delete curnode;
            cache.erase(block.id);
        }
        for (int z = validBlocks.size(); z < Z; z++) 
	{
            Block &block = bucket[z];
            block.id = 0;
            block.data.resize(blockSize, 0);
        }
        writeviewmap.push_back(node);
        WriteBucket(node, bucket);
    }
}

// Gets the data of a block in the cache

BTreeNodef* BRAMf::ReadData(int bid) {
    if (cache.find(bid) == cache.end()) {
        return NULL;
    }
    return cache[bid];
}

// Updates the data of a block in the cache

void BRAMf::WriteData(int bid, BTreeNodef* node) 
{
    if (store->GetEmptySize() > 0) 
    {
        cache[bid] = node;
	if(node->knum==2*T-1) // still not correct
        	store->ReduceEmptyNumbers();
	//cout <<"FREE:"<<store->GetEmptySize()<<endl;
    } 
    else 
    {
        throw runtime_error("There is no more space in BRAMf-WriteData");
    }
}


// Fetches a block, allowing you to read and write in a block

void BRAMf::Access(int bid, BTreeNodef*& node, int lastLeaf, int newLeaf) {
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

void BRAMf::Access(int bid, BTreeNodef*& node) 
{
    FetchPath(node->pos);
    WriteData(bid, node);
    if (find(leafList.begin(), leafList.end(), node->pos) == leafList.end()) 
    {
        leafList.push_back(node->pos);
    }
}

BTreeNodef* BRAMf::ReadBTreeNodef(int bid) {
    if (bid == 0) {
        throw runtime_error("BTreeNodef id is not set ReadBTreeNodef");
    }
    if (cache.count(bid) == 0) {
        throw runtime_error("BTreeNodef not found in the cache ReadBTreeNodef");
    } else {
        BTreeNodef* node = cache[bid];
        return node;
    }
}

BTreeNodef* BRAMf::ReadBTreeNodef(int bid, int lastLeaf) {
    if (bid == 0) 
    {
        throw runtime_error("BTreeNodef id is not set ReadBTreeNodef");
        return NULL;
    }
    if (cache.count(bid) == 0) 
    {
        BTreeNodef* node;
        Access(bid, node, lastLeaf, lastLeaf);
        if (node != NULL) {
            modified.insert(bid);
        }
	else 
	{
		cout <<"BTreeNodef is NULL : "<< bid << endl ;
		cout <<"free node:" << store->GetEmptySize() << endl;
        throw runtime_error("BTreeNodef id is not set ReadBTreeNodef");
	}
        return node;
    } else {
        modified.insert(bid);
        BTreeNodef* node = cache[bid];
        node->pos = lastLeaf;
        return node;
    }
}

int BRAMf::WriteBTreeNodef(int bid, BTreeNodef* node) 
{
    if (bid == 0) 
    {
        throw runtime_error("Node id is not set WriteBTreeNodef");
    }
    if (cache.count(bid) == 0) 
    {
        modified.insert(bid);
        Access(bid, node);
        return node->pos;
    } 
    else 
    {
	//cout <<"going to write but in cache: "<< bid<<" knum:"<<node->knum<<endl;
        modified.insert(bid);
        return node->pos;
    }
}

BTreeNodef* BRAMf::convertBlockToBTreeNodef(block b) 
{
    BTreeNodef* node = new BTreeNodef();
    std::array<byte_t, sizeof (BTreeNodef) > arr;
    std::copy(b.begin(), b.begin() + sizeof (BTreeNodef), arr.begin());
    from_bytes(arr, *node);
    return node;
}

void BRAMf::getbid(block b, int &bID) 
{
    BTreeNodef* node = new BTreeNodef();
    std::array<byte_t, sizeof (BTreeNodef) > arr;
    std::copy(b.begin(), b.begin() + sizeof (BTreeNodef), arr.begin());
    from_bytes(arr, *node);
    bID = node->bid;
    //delete node;
}

block BRAMf::convertBTreeNodefToBlock(BTreeNodef* node) 
{
    std::array<byte_t, sizeof (BTreeNodef) > data = to_bytes(*node);
    block b(data.begin(), data.end());
    return b;
}

void BRAMf::finalize(int& brootKey, int& brootPos) 
{
    if (!batchWrite) 
    {
            for (int i = readCnt; i < pad; i++)
	    {
                int rnd = RandomPath();
                if (std::find(leafList.begin(), leafList.end(), rnd) == leafList.end()) 
		{
                    leafList.push_back(rnd);
                }
                FetchPath(rnd);
        }
    }
	
        int maxHeight = 1;
        for (auto t : cache) {
            if (t.second != NULL && t.second->height > maxHeight) {
                maxHeight = t.second->height;
            }
        }
    for (unsigned int i = 1; i <= maxHeight; i++) 
    {
        for (auto t : cache) 
	{
            if (t.second != NULL && t.second->height == i) 
	    {
                BTreeNodef* tmp = t.second;
                if (modified.count(tmp->bid)) 
		{
                    tmp->pos = RandomPath();
                }
		for(int k = 0;k<=tmp->knum;k++)
		{
			if (tmp->cbids[k] != 0 && cache.count(tmp->cbids[k]) > 0) 
			{
		    		tmp->cpos[k] = cache[tmp->cbids[k]]->pos;
                	}
		}
            }
        }
    }
    if (cache.count(brootKey) != 0)
        brootPos = cache[brootKey]->pos;

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

void BRAMf::finalizedel(int& brootKey, int& brootPos) 
{
	
    if (cache.count(brootKey) != 0)
        brootPos = cache[brootKey]->pos;

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
void BRAMf::start(bool batchWrite) 
{
    this->batchWrite = batchWrite;
    writeviewmap.clear();
    readviewmap.clear();
    readCnt = 0;
    //searchf_bytes=0;
}

void BRAMf::Print() 
{
    for (unsigned int i = 0; i < bucketCount; i++) 
    {
        block ciphertext = store->Read(i);
        block buffer = AES::Decrypt(key, ciphertext, clen_size);
        Bucket bucket = DeserialiseBucket(buffer);
        BTreeNodef* node = convertBlockToBTreeNodef(bucket[0].data);
        cout << node->bid << " ";
        delete node;
    }
}

int BRAMf::RandomPath() 
{
    int val = dis(mt);
    return val;
}

/*
void BRAMf::setupInsert(vector<BTreeNodef*> nodes) {
    sort(nodes.begin(), nodes.end(), [ ](const BTreeNodef* lhs, const BTreeNodef * rhs) {
        return lhs->pos < rhs->pos;
    });
    int curPos = 0;
    if (nodes.size() > 0) {
        curPos = nodes[0]->pos;
    }
    map<int, Bucket> buckets;
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
            int nodeIndex = GetBTreeNodefOnPath(curPos, d);
            Bucket bucket;
            if (bucketsCnt.count(nodeIndex) == 0) {
                bucketsCnt[nodeIndex] = 0;
                for (int z = 0; z < Z; z++) {
                    if (i < nodes.size() && nodes[i]->pos == curPos) {
                        Block &curBlock = bucket[z];
                        curBlock.id = nodes[i]->key;
                        curBlock.data = convertBTreeNodefToBlock(nodes[i]);
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
                            Block &curBlock = bucket[z];
                            curBlock.id = nodes[i]->key;
                            curBlock.data = convertBTreeNodefToBlock(nodes[i]);
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
            WriteBucket(buk.first, buk.second);
        } else {
            for (long unsigned int z = bucketsCnt[buk.first]; z < Z; z++) {
                Block &curBlock = buk.second[z];
                curBlock.id = 0;
                curBlock.data.resize(blockSize, 0);
            }
            WriteBucket(buk.first, buk.second);
        }
    }

    for (; i < nodes.size(); i++) {
        cache[nodes[i]->key] = nodes[i];
    }
}
*/
