#include "BRAM.hpp"
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

BRAM::BRAM(int maxSize, bytes<Key> key)
: key(key), rd(), mt(rd()), dis(0, (pow(2, floor(log2(maxSize / Z)) + 1) - 1) / 2) 
{
    AES::Setup();
    depth = floor(log2(maxSize / Z));
    cout <<"depth of tree:"<<depth<<endl;
    pad = floor((log2((maxSize+1)/(2*Z)))/(log2(T)));
    cout <<"worst case height of BTree:"<<pad<<endl;
    bucketCount = pow(2, depth + 1) - 1;
    blockSize = sizeof (BTreeNode); //??
    size_t blockCount = Z * (pow(2, depth + 1) - 1);
    size_t storeBlockbSize = IV + AES::GetCiphertextLength(Z * (blockSize));
    size_t storeBlockbCount = blockCount;
    clen_size = AES::GetCiphertextLength((blockSize) * Z);
    plaintext_size = (blockSize) * Z;
    store = new RAMStore(storeBlockbCount, storeBlockbSize);
    cout << "Bucketbs:"<<bucketCount<<" block count in BRAM:"<<blockCount<<endl;
    for (size_t i = 0; i < bucketCount; i++) 
    {
        Bucketb bucket;
        for (int z = 0; z < Z; z++) 
	{
            bucket[z].id = 0;
            bucket[z].data.resize(blockSize, 0);
        }
        WriteBucketb(i, bucket);
    }
}

BRAM::~BRAM() {
    AES::Cleanup();
    delete store;
}
int BRAM::get_knumbn(BTreeNode* bn)
{
	int knum = 0;
	int i = 0;
	while(i<D-1 && bn->values[i]!=0)
	{
		i++;
		knum++;
	}
	return knum;
}

int BRAM::GetBTreeNodeOnPath(int leaf, int curDepth) {
    leaf += bucketCount / 2;
    for (int d = depth - 1; d >= curDepth; d--) {
        leaf = (leaf + 1) / 2 - 1;
    }
    return leaf;
}

block BRAM::SerialiseBucketb(Bucketb bucket) 
{
    block buffer;
    for (int z = 0; z < Z; z++) 
    {
        Blockb b = bucket[z];
        buffer.insert(buffer.end(), b.data.begin(), b.data.end());
    }
    assert(buffer.size() == Z * (blockSize));
    return buffer;
}

Bucketb BRAM::DeserialiseBucketb(block buffer) 
{
    assert(buffer.size() == Z * (blockSize));
    Bucketb bucket;
    for (int z = 0; z < Z; z++) 
    {
        Blockb &block = bucket[z];
        block.data.assign(buffer.begin(), buffer.begin() + blockSize);
        //BTreeNode* node = convertBlockbToBTreeNode(block.data);
        //block.id = node->bid;
        //delete node; //why cant I delete this
	int bID;
	getbid(block.data,bID);
	block.id = bID;
        buffer.erase(buffer.begin(), buffer.begin() + blockSize);
    }
    return bucket;
}

Bucketb BRAM::ReadBucketb(int index) {
    block ciphertext = store->Read(index);
    block buffer = AES::Decrypt(key, ciphertext, clen_size);
    Bucketb bucket = DeserialiseBucketb(buffer);
    return bucket;
}

void BRAM::WriteBucketb(int index, Bucketb bucket) 
{
    block b = SerialiseBucketb(bucket);
    block ciphertext = AES::Encrypt(key, b, clen_size, plaintext_size);
    store->Write(index, ciphertext);
}


void BRAM::FetchPath(int leaf) 
{
    readCnt++;
    for (size_t d = 0; d <= depth; d++) 
    {
        int node = GetBTreeNodeOnPath(leaf, d);
        if (find(readviewmap.begin(), readviewmap.end(), node) != readviewmap.end()) 
	{
            continue;
        } 
	else 
	{
            readviewmap.push_back(node);
        }

        Bucketb bucket = ReadBucketb(node); //<<-- here
        for (int z = 0; z < Z; z++) 
	{
            Blockb &block = bucket[z];
            if (block.id != 0) 
	    { 
                BTreeNode* n = convertBlockbToBTreeNode(block.data);
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


std::vector<int> BRAM::GetIntersectingBlockbs(int x, int curDepth) {
    std::vector<int> validBlockbs;
    int node = GetBTreeNodeOnPath(x, curDepth);
    for (auto b : cache) {
        int bid = b.first;
        if (b.second != NULL && GetBTreeNodeOnPath(b.second->pos, curDepth) == node) 
	{
            validBlockbs.push_back(bid);
            if (validBlockbs.size() >= Z) 
	    {
                return validBlockbs;
            }
        }
    }
    return validBlockbs;
}


void BRAM::WritePath(int leaf, int d) 
{
    int node = GetBTreeNodeOnPath(leaf, d);
    if (find(writeviewmap.begin(), writeviewmap.end(), node) == writeviewmap.end()) 
    {
        auto validBlockbs = GetIntersectingBlockbs(leaf, d);
        Bucketb bucket;
        for (int z = 0; z < std::min((int) validBlockbs.size(), Z); z++) 
	{
            Blockb &block = bucket[z];
            block.id = validBlockbs[z];
	    int temp = block.id;
            BTreeNode* curnode = cache[block.id];
            block.data = convertBTreeNodeToBlockb(curnode);
	    if(curnode->bid != block.id)
	    {
		    block.id = 0; // curnode->key;
		    block.data.resize(blockSize, 0);
	    }
            delete curnode;
            cache.erase(temp);
        }
        for (int z = validBlockbs.size(); z < Z; z++) 
	{
            Blockb &block = bucket[z];
            block.id = 0;
            block.data.resize(blockSize, 0);
        }
        writeviewmap.push_back(node);
        WriteBucketb(node, bucket);
    }
}

// Gets the data of a block in the cache

BTreeNode* BRAM::ReadData(int bid) {
    if (cache.find(bid) == cache.end()) {
        return NULL;
    }
    return cache[bid];
}

// Updates the data of a block in the cache

void BRAM::WriteData(int bid, BTreeNode* node) 
{
    if (store->GetEmptySize() > 0) 
    {
        cache[bid] = node;
	if(get_knumbn(node)==2*T-1) // still not correct
        	store->ReduceEmptyNumbers();
	else if(bid != node->bid)
		store->IncreaseEmptyNumbers(); // will be deleted in WritePath()
    } 
    else 
    {
        throw runtime_error("There is no more space in BRAM-WriteData");
    }
}


// Fetches a block, allowing you to read and write in a block

void BRAM::Access(int bid, BTreeNode*& node, int lastLeaf) 
{
    FetchPath(lastLeaf);
    node = ReadData(bid);
    if (node != NULL) 
    {
        //node->pos = newLeaf;
        if (cache.count(bid) != 0) 
	{
            cache.erase(bid);
        }
        cache[bid] = node;
        if (find(leafList.begin(), leafList.end(), lastLeaf) == leafList.end()) 
	{
            leafList.push_back(lastLeaf);
        }
    }
}

void BRAM::Access(int bid, BTreeNode*& node) 
{
    FetchPath(node->pos);
    WriteData(bid, node);
    if (find(leafList.begin(), leafList.end(), node->pos) == leafList.end()) 
    {
        leafList.push_back(node->pos);
    }
}

BTreeNode* BRAM::ReadBTreeNode(int bid) {
    if (bid == 0) {
        throw runtime_error("BTreeNode id is not set ReadBTreeNode");
    }
    if (cache.count(bid) == 0) {
        throw runtime_error("BTreeNode not found in the cache ReadBTreeNode");
    } else {
        BTreeNode* node = cache[bid];
        return node;
    }
}

BTreeNode* BRAM::ReadBTreeNode(int bid, int lastLeaf) {
    if (bid == 0) 
    {
        throw runtime_error("BTreeNode id is not set ReadBTreeNode");
        return NULL;
    }
    if (cache.count(bid) == 0) 
    {
        BTreeNode* node;
        Access(bid, node, lastLeaf);
        if (node != NULL) {
            modified.insert(bid);
        }
	else 
	{
		cout <<"BTreeNode is NULL : "<< bid << endl ;
		cout <<"free node:" << store->GetEmptySize() << endl;
        	throw runtime_error("BTreeNode id is not set ReadBTreeNode");
	}
        return node;
    } else {
        modified.insert(bid);
        BTreeNode* node = cache[bid];
        //node->pos = lastLeaf;
        return node;
    }
}

int BRAM::WriteBTreeNode(int bid, BTreeNode* node) 
{
    if (bid == 0) 
    {
        throw runtime_error("Node id is not set WriteBTreeNode");
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

BTreeNode* BRAM::convertBlockbToBTreeNode(block b) 
{
    BTreeNode* node = new BTreeNode();
    std::array<byte_t, sizeof (BTreeNode) > arr;
    std::copy(b.begin(), b.begin() + sizeof (BTreeNode), arr.begin());
    from_bytes(arr, *node);
    return node;
}

void BRAM::getbid(block b, int &bID) 
{
    BTreeNode* node = new BTreeNode();
    std::array<byte_t, sizeof (BTreeNode) > arr;
    std::copy(b.begin(), b.begin() + sizeof (BTreeNode), arr.begin());
    from_bytes(arr, *node);
    bID = node->bid;
    //delete node;
}

block BRAM::convertBTreeNodeToBlockb(BTreeNode* node) 
{
    std::array<byte_t, sizeof (BTreeNode) > data = to_bytes(*node);
    block b(data.begin(), data.end());
    return b;
}

void BRAM::finalize(int& brootKey, int& brootPos) 
{
/*    if (!batchWrite) 
    {
            for (int i = readCnt; i <= pad; i++)
	    {
                int rnd = RandomPath();
                if (std::find(leafList.begin(), leafList.end(), rnd) == leafList.end()) 
		{
                    leafList.push_back(rnd);
                }
                FetchPath(rnd);
        }
    }
*/	
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
                BTreeNode* tmp = t.second;
                if (modified.count(tmp->bid)) 
		{
                    tmp->pos = RandomPath();
                }
		for(int k = 0;k<=get_knumbn(tmp);k++)
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

void BRAM::finalizedel(int& brootKey, int& brootPos) 
{
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
                BTreeNode* tmp = t.second;
                if (modified.count(tmp->bid)) 
		{
                    tmp->pos = RandomPath();
                }
		for(int k = 0;k<=get_knumbn(tmp);k++)
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
void BRAM::start(bool batchWrite) 
{
    this->batchWrite = batchWrite;
    writeviewmap.clear();
    readviewmap.clear();
    readCnt = 0;
}

void BRAM::Print() 
{
    for (unsigned int i = 0; i < bucketCount; i++) 
    {
        block ciphertext = store->Read(i);
        block buffer = AES::Decrypt(key, ciphertext, clen_size);
        Bucketb bucket = DeserialiseBucketb(buffer);
        BTreeNode* node = convertBlockbToBTreeNode(bucket[0].data);
        cout << node->bid << " ";
        delete node;
    }
}

int BRAM::RandomPath() 
{
    int val = dis(mt);
    return val;
}

/*
void BRAM::setupInsert(vector<BTreeNode*> nodes) {
    sort(nodes.begin(), nodes.end(), [ ](const BTreeNode* lhs, const BTreeNode * rhs) {
        return lhs->pos < rhs->pos;
    });
    int curPos = 0;
    if (nodes.size() > 0) {
        curPos = nodes[0]->pos;
    }
    map<int, Bucketb> buckets;
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
            int nodeIndex = GetBTreeNodeOnPath(curPos, d);
            Bucketb bucket;
            if (bucketsCnt.count(nodeIndex) == 0) {
                bucketsCnt[nodeIndex] = 0;
                for (int z = 0; z < Z; z++) {
                    if (i < nodes.size() && nodes[i]->pos == curPos) {
                        Blockb &curBlockb = bucket[z];
                        curBlockb.id = nodes[i]->key;
                        curBlockb.data = convertBTreeNodeToBlockb(nodes[i]);
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
                            Blockb &curBlockb = bucket[z];
                            curBlockb.id = nodes[i]->key;
                            curBlockb.data = convertBTreeNodeToBlockb(nodes[i]);
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
            WriteBucketb(buk.first, buk.second);
        } else {
            for (long unsigned int z = bucketsCnt[buk.first]; z < Z; z++) {
                Blockb &curBlockb = buk.second[z];
                curBlockb.id = 0;
                curBlockb.data.resize(blockSize, 0);
            }
            WriteBucketb(buk.first, buk.second);
        }
    }

    for (; i < nodes.size(); i++) {
        cache[nodes[i]->key] = nodes[i];
    }
}
*/
