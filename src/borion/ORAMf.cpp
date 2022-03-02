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
    cout << "Bucket and block count in ORAM:" << bucketCount<<"|"<<Z<<"="<<bucketCount*Z<<endl;
    store = new RAMStore(storeBlockCount, storeBlockSize);
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
    /*for(auto c: cache)
    {
	    Bid k = c.first;
	    cout <<k<< endl;
    }*/
}

// Gets a list of blocks on the cache which can be placed at a specific point

std::vector<Bid> ORAMf::GetIntersectingBlocks(int x, int curDepth) {
    std::vector<Bid> validBlocks;

    int node = GetNodefOnPath(x, curDepth);
    for (auto b : cache) {
        Bid bid = b.first;
        if (b.second != NULL && GetNodefOnPath(b.second->pos, curDepth) == node) {
            validBlocks.push_back(bid);
            if (validBlocks.size() >= Z) {
                return validBlocks;
            }
        }
    }
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
		    block.id = 0; // curnode->key;
		    block.data.resize(blockSize, 0);
	    }
            delete curnode;
            cache.erase(temp);
        }
        // Fill any empty spaces with dummy blocks
        for (int z = validBlocks.size(); z < Z; z++) {
            Blockf &block = bucket[z];
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
    if (store->GetEmptySize() > 0) {
        cache[bid] = node;
        store->ReduceEmptyNumbers();
    } else {
        throw runtime_error("There is no more space in ORAMf");
    }
}

void ORAMf::DeleteData(Bid bid, Nodef* node) 
{
	if(bid != node->key)
	{
        	cache[bid]=node; 
        	store->IncreaseEmptyNumbers();
		int ret = store->GetEmptySize();
		cout <<bid<<"empty nodes in ORAMf:"<< ret <<endl;
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
       if (find(leafList.begin(), leafList.end(), lastLeaf) == leafList.end()) 
       {
            leafList.push_back(lastLeaf);
       }
    }
}

void ORAMf::Access(Bid bid, Nodef*& node) {
    if (!batchWrite) {
        FetchPath(node->pos);
    }
    WriteData(bid, node);
    if (find(leafList.begin(), leafList.end(), node->pos) == leafList.end()) {
        leafList.push_back(node->pos);
    }
}


Nodef* ORAMf::setupReadNf(Bid bid,int leaf)
{
	    cout <<bid<<"in setupReadNf leaf is:"<<leaf<<endl;
    if (bid == 0) {
	    cout <<"Heyyyy setupReadNf is 0"<<endl;
        return NULL;
    }
    Nodef* n;
    for (size_t d = 0; d <= depth; d++) 
    {
        int node = GetNodefOnPath(leaf, d);
        Bucketf bucket = ReadBucket(node);
        for (int z = 0; z < Z; z++) 
	{
            Blockf &block = bucket[z];
            if (block.id == bid) 
	    {    
		cout<<"at setupReadNf block.id = bid:"<< block.id<<endl; 
                n = convertBlockToNodef(block.data);
		return n;
            }
        }

     }
    cout <<"found "<<bid<<" in cache"<<endl;
    n=cache[bid];
    return n;
}

Nodef* ORAMf::ReadNodef(Bid bid) {
    if (bid == 0) {
        throw runtime_error("Nodef id is not set");
    }
    if (cache.count(bid) == 0) {
        throw runtime_error("Nodef not found in the cache");
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
	else 
	{
		cout <<"nodef is NULL : "<< bid << endl ;
		cout <<"free nodefs:" << store->GetEmptySize() << endl;
	}
        return node;
    } else {
        modified.insert(bid);
        Nodef* node = cache[bid];
        node->pos = newLeaf;
        return node;
    }
}

void ORAMf::setupWriteBucket(Bid bid, Nodef* n, Bid rootKey, int& rootPos)
{
    int flag = 0;
    for (size_t d = 0; d <= depth; d++) 
    {
        int node = GetNodefOnPath(n->pos, d);
        Bucketf bucket = ReadBucket(node);
        Bucketf newbucket;
        for (int z = 0; z < Z; z++) 
	{
            Blockf &newblock = newbucket[z];
	    Blockf &block = bucket[z];
	    int pos ;
            if (flag==0 && (block.id == bid || block.id == 0)) 
	    {    
            	Nodef* curnode = n;
		newblock.id = bid;
		cout <<n->key<<"CURNODE key IS:--------"<< block.id<<endl;
                newblock.data = convertNodefToBlock(curnode);
		flag = 1;
		store->ReduceEmptyNumbers();
		pos = curnode->pos;
		//delete curnode;
            }
	    else if(block.id == 0)
	    {
            	newblock.id = 0;
            	newblock.data.resize(blockSize, 0);
	    }
	    else
	    {
                Nodef* curnode = convertBlockToNodef(block.data);
		newblock.id = curnode->key;
		cout <<"full blocks setupWriting:"<<block.id<<endl;
		newblock.data = convertNodefToBlock(curnode);
		pos = curnode->pos;
		//delete curnode;
	    }
	    if(rootKey == newblock.id)
	    {
		    rootPos = pos;
		    cout <<"At ROOT :"<< rootPos<< endl;
	    }
		
        }

        WriteBucket(node, newbucket);
	if(flag == 1)
		break;
     }
    //if(fla==0)
    //	    call this function with different leaf
}

int ORAMf::setupWriteNf(Bid bid, Nodef* node, Bid rootKey, int& rootPos) {
    if (bid == 0) {
        throw runtime_error("Nodef id is not set in WriteNode");
    }
    else
    {
	    setupWriteBucket(bid,node,rootKey,rootPos);
    }
    cout <<"returning rootPOs ------"<< node->key << endl;
    return rootPos;
}
int ORAMf::WriteNodef(Bid bid, Nodef* node) {
    if (bid == 0) {
        throw runtime_error("Nodef id is not set in WriteNode");
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

int ORAMf::DeleteNode(Bid bid, Nodef* node) {
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
    DeleteData(bid, node);
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
            }
        }
    }
    if (cache[rootKey] != NULL)
        rootPos = cache[rootKey]->pos;

    for (int d = depth; d >= 0; d--) {
        for (unsigned int i = 0; i < leafList.size(); i++) {
            WritePath(leafList[i], d);
        }
    }

    leafList.clear();
    modified.clear();
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
        Nodef* node = convertBlockToNodef(bucket[0].data);
        cout << node->key << " ";
        delete node;
    }
}

int ORAMf::RandomPath() {
    int val = dis(mt);
    return val;
}
