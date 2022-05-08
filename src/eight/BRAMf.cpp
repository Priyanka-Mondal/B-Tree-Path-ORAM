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
#include <memory>

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

int BRAMf::knumbn(BTreeNodef* bn)
{
	int knum = 0;
	int i = 0;
	while(i<D-1 && bn->keys[i]!=Bid(0))
	{
		i++;
		knum++;
	}
	return knum;
}
bool BRAMf::isleafbn(BTreeNodef* bn)
{
	if(bn->cpos[0]==-1)
		return true;
	else return false;
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

BTreeNodef* common;
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
        common = convertBlockToBTreeNodef(block.data);
        block.id = common->bid;
	free(common);
        buffer.erase(buffer.begin(), buffer.begin() + blockSize);
        //delete common; //why cant I delete this
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
	rtt++;
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
	    int temp = block.id;
            BTreeNodef* curnode = cache[temp];
            block.data = convertBTreeNodefToBlock(curnode);
	    if(curnode->bid != block.id)
	    {
		    block.id = 0; // curnode->key;
		    block.data.resize(blockSize, 0);
	    }
            delete curnode;
            cache.erase(temp);
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


BTreeNodef* BRAMf::ReadData(int bid) 
{
    if (cache.find(bid) == cache.end()) {
        return NULL;
    }
    return cache[bid];
}


void BRAMf::WriteData(int bid, BTreeNodef* node) 
{
    if (store->GetEmptySize() > 0) 
    {
        cache[bid] = node;
	if(knumbn(node)==2*T-1) // still not correct
        	store->ReduceEmptyNumbers();
	//cout <<"FREE:"<<store->GetEmptySize()<<endl;
    } 
    else 
    {
        throw runtime_error("There is no more space in BRAMf-WriteData");
    }
}



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

BTreeNodef* BRAMf::Access(int bid, int lastLeaf) 
{
    FetchPath(lastLeaf);
    BTreeNodef *node = ReadData(bid);
    if (node != NULL) 
    {
        cache[bid] = node;
        if (find(leafList.begin(), leafList.end(), lastLeaf) == leafList.end()) 
	{
            leafList.push_back(lastLeaf);
        }
    }
    return node;
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

BTreeNodef* BRAMf::ReadBTreeNodef(int bid, int lastLeaf, int mh) 
{
    if (bid == 0) 
    {
        throw runtime_error("BTreeNodeFILE id is not set ReadBTreeNodef");
        return NULL;
    }
    if (cache.count(bid) == 0) 
    {
        BTreeNodef* node = Access(bid, lastLeaf);
        if (node != NULL) 
	{
		hstash[bid]=mh;
                modified.insert(bid);
        }
	else 
	{
		cout <<"BTreeNodeFILE is NULL : "<< bid << endl ;
		cout <<"free node:" << store->GetEmptySize() << endl;
        	throw runtime_error("BTreeNodeFILE id is not set ReadBTreeNodef");
	}
        return node;
    } 
    else 
    {
	hstash[bid]=mh;
        modified.insert(bid);
        BTreeNodef* node = cache[bid];
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
    BTreeNodef* node = (BTreeNodef*)malloc(sizeof(BTreeNodef));
    std::array<byte_t, sizeof (BTreeNodef) > arr;
    std::copy(b.begin(), b.begin() + sizeof (BTreeNodef), arr.begin());
    from_bytes(arr, *node);
    return node;
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
            for (int i = readCnt; i <= pad; i++)
	    {
                int rnd = RandomPath();
                if (std::find(leafList.begin(), leafList.end(), rnd) == leafList.end()) 
                    leafList.push_back(rnd);
                FetchPath(rnd);
        }
    }
    
    for (unsigned int i = 0; i <= maxheight; i++) 
    {
	for(auto t: hstash)
	{
		if(modified.count(t.first) && t.second == i)
		{
                    BTreeNodef* tmp = cache[t.first];
		    if(tmp != NULL)
		    {
			//assert(t.second == tmp->height);
                    	tmp->pos = RandomPath();
			if(!isleafbn(tmp))
			{
			    for(int k = 0;k<=knumbn(tmp);k++)
			    {
			        if (tmp->cbids[k] != 0 && cache.count(tmp->cbids[k]) > 0) 
			        {
		    		    tmp->cpos[k] = cache[tmp->cbids[k]]->pos;
                	        }
			    }
		         }
                    }
		    //cache[t.first]=tmp;
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
/*
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
*/

void BRAMf::start(bool batchWrite) 
{
    this->batchWrite = batchWrite;
    hstash.clear();
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

void BRAMf::setupInsert(vector<BTreeNodef*> nodes) 
{
	auto el = nodes.begin();
	int leaf;
	LEAF: if(el != nodes.end())
		leaf = (*el)->pos;
	if(el != nodes.end())
	{
		for (size_t d = depth; d >= 0; d--) 
		{
			if(leaf == (*el)->pos)
			{
		        	int node = GetBTreeNodefOnPath(leaf, d);
		       		Bucket bucket = ReadBucket(node);
				int z = 0;
				while(el != nodes.end() && z<Z && leaf == (*el)->pos)
		        	{
					assert(leaf == (*el)->pos);
					Block &block = bucket[z];
					block.id = (*el)->bid;
					block.data = convertBTreeNodefToBlock(*el);
					delete *el;
					z++;
					el++;
				}
				//if(z>0)
				//{
				      WriteBucket(node,bucket);
				//}
				if(el != nodes.end() && leaf != (*el)->pos)
					goto LEAF;
				else if(el == nodes.end())
					goto DONE;
			}
			else if(el != nodes.end())
				goto LEAF;
			else goto DONE;
		}
	}
DONE: cout <<"done FILE setup"<<endl;
}
