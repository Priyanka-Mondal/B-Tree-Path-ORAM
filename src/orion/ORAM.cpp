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
: key(key), rd(), mt(rd()), dis(0, (pow(2, floor(log2(maxSize / Z)) + 1) - 1) / 2) {
    AES::Setup();
    depth = floor(log2(maxSize / Z));
    cout <<"depth of tree:"<<depth<<endl;
    bucketCount = pow(2, depth + 1) - 1;
    blockSize = sizeof (Node); // B
    size_t blockCount = Z * (pow(2, depth + 1) - 1);
    size_t storeBlockSize = IV + AES::GetCiphertextLength(Z * (blockSize));
    size_t storeBlockCount = blockCount;
    clen_size = AES::GetCiphertextLength((blockSize) * Z);
    plaintext_size = (blockSize) * Z;
    store = new RAMStore(storeBlockCount, storeBlockSize);
    cout << "Buckets:"<<bucketCount<<" block count in ORAM:"<<blockCount<<endl;
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
    delete store;
}


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
        buffer.insert(buffer.end(), b.data.begin(), b.data.end());
    //delete b;
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
    ciphertext.clear();
    buffer.clear();
    return bucket;
}

void ORAM::WriteBucket(int index, Bucket bucket) {
    block b = SerialiseBucket(bucket);
    block ciphertext = AES::Encrypt(key, b, clen_size, plaintext_size);
    store->Write(index, ciphertext);
    ciphertext.clear();
    b.clear();
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
                    cache.insert(make_pair(block.id, n));
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


void ORAM::WritePath(int leaf, int d) 
{
    int node = GetNodeOnPath(leaf, d);
    if (find(writeviewmap.begin(), writeviewmap.end(), node) == writeviewmap.end()) 
    {
        auto validBlocks = GetIntersectingBlocks(leaf, d);
        Bucket bucket;
        for (int z = 0; z < std::min((int) validBlocks.size(), Z); z++) 
	{
            Block &block = bucket[z];
            block.id = validBlocks[z];
	    Bid temp = block.id;
            Node* curnode = cache[block.id];
            block.data = convertNodeToBlock(curnode);
	    if(curnode->key != block.id)
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
    if (store->GetEmptySize() > 0) {
        cache[bid] = node;
        store->ReduceEmptyNumbers();
	cout <<"FREE:"<<store->GetEmptySize()<<endl;
    } else {
        throw runtime_error("There is no more space in ORAM-WriteData");
    }
}

void ORAM::DeleteData(Bid bid, Node* node) 
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
    //if (!batchWrite) {
        FetchPath(node->pos);
   // }
    WriteData(bid, node);
    if (find(leafList.begin(), leafList.end(), node->pos) == leafList.end()) {
        leafList.push_back(node->pos);
    }
}

Node* ORAM::setupReadN(Bid bid,int leaf)
{
    if (bid == 0) {
        return NULL;
    }
    for (size_t d = depth; d >= 0; d--) 
    {
        int node = GetNodeOnPath(leaf, d);
        Bucket bucket = ReadBucket(node);
        for (int z = 0; z < Z; z++) 
	{
            Block &block = bucket[z];
            if (block.id == bid) 
	    {    
                Node* n = new Node();
    std::array<byte_t, sizeof (Node) > arr;
    std::copy(block.data.begin(), block.data.begin() + sizeof (Node), arr.begin());
    from_bytes(arr, *n);
		return n;
            }
        }

     }
    if(cache.count(bid)>0)
    {
    	cout <<"found "<<bid<<" in cache ORAM"<<endl;
    	return cache[bid];
    }
    else
	    cout<<bid<<"NOT FOUND at ALL in setupRead"<<endl;
    return NULL;
}
void ORAM::setupReadN(Node*& n, Bid bid,int leaf)
{
    if (bid == 0) {
        return ;
    }
    for (size_t d = depth; d >= 0; d--) 
    {
        int node = GetNodeOnPath(leaf, d);
        Bucket bucket = ReadBucket(node);
        for (int z = 0; z < Z; z++) 
	{
            Block &block = bucket[z];
            if (block.id == bid) 
	    {    
                //Node* n = new Node();
	       	//convertBlockToNode(n,block.data);
		//return n;

    std::array<byte_t, sizeof (Node) > arr;
    std::copy(block.data.begin(), block.data.begin() + sizeof (Node), arr.begin());
    from_bytes(arr, *n);
    return;
            }
        }

     }
    if(cache.count(bid)>0)
    {
    	cout <<"found "<<bid<<" in cache ORAM"<<endl;
    	n = cache[bid];
    }
    else
	    cout<<bid<<"NOT FOUND at ALL in setupRead"<<endl;
    //return NULL;
}
Node* ORAM::ReadNode(Bid bid) {
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
	else 
	{
		cout <<"Node is NULL : "<< bid << endl ;
		cout <<"free node:" << store->GetEmptySize() << endl;
	}
        return node;
    } else {
        modified.insert(bid);
        Node* node = cache[bid];
        node->pos = newLeaf;
        return node;
    }
}

void ORAM::setupWriteBucket(Bid bid, Node* n, Bid rootKey, int& rootPos)
{
	int oramsz = store->GetEmptySize();
	 if (oramsz>0) {
    int flag = 0;
    for (size_t d = depth; d >= 0; d--) 
    {
        int node = GetNodeOnPath(n->pos, d);
        Bucket bucket = ReadBucket(node);
        for (int z = 0; z < Z; z++) 
	{
	    Block &block = bucket[z];
	    int pos ;
            if (flag==0 &&  block.id == 0) 
	    {    
            	//Node* curnode = n;
		block.id = bid;
                //block.data = convertNodeToBlock(curnode);
                block.data = convertNodeToBlock(n);
		flag = 1;
		store->ReduceEmptyNumbers();
		//pos = curnode->pos;
		pos = n->pos;
		 //cout<<pos<<endl;//" Empty Nodes in ORAM:"<<oramsz<<endl;
		//delete curnode;
            }
	    else if (flag==0 && block.id == bid ) 
	    {    
            	//Node* curnode = n;
		block.id = bid;
                //block.data = convertNodeToBlock(curnode);
                block.data = convertNodeToBlock(n);
		flag = 1;
		//store->ReduceEmptyNumbers();
		//pos = curnode->pos;
		pos = n->pos;
		//delete curnode;
            }
	    else if(block.id == 0)
	    {
            	block.id = 0;
            	block.data.resize(blockSize, 0);
	    }
	    else
	    {
                Node* curnode = convertBlockToNode(block.data);
		//Bid kk = curnode->key;
		block.id = curnode->key;//kk
		//cout <<"full blocks setupWriting:"<<block.id<<endl;
		//block.data = convertNodeToBlock(block.data);
		block.data = convertNodeToBlock(curnode);
		pos = curnode->pos;
		delete curnode;
	    }
	    if(rootKey == block.id)
	    {
		    rootPos = pos;
		    //cout <<"At ROOT :"<< rootPos<< endl;
	    }
		
        }

        WriteBucket(node, bucket);
	if(flag == 1)
		break;
     }
    if(flag == 0)
    {
	    cache[bid] = n;
	    cout <<"Writing in CACHE!!!!!!"<<endl;
    }
 }
    else
	 throw runtime_error("No more spaCe in ORAM");
}


int ORAM::setupWriteN(Bid bid, Node* node, Bid rootKey, int& rootPos) 
{
    if (bid == 0) 
        throw runtime_error("Node id is not set in WriteNode");
    else
	setupWriteBucket(bid,node,rootKey,rootPos);
    return rootPos;
}
int ORAM::WriteNode(Bid bid, Node* node) {
    
    if (bid == 0) 
    {
	cout <<bid<<endl;
        throw runtime_error("Node id is not set WriteNode");
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

Node* ORAM::convertBlockToNode(block b) {
    Node* node = new Node();
    std::array<byte_t, sizeof (Node) > arr;
    std::copy(b.begin(), b.begin() + sizeof (Node), arr.begin());
    from_bytes(arr, *node);
    return node;
}

void ORAM::convertBlockToNode(Node*& node, block b) {
    //Node* node = new Node();
    std::array<byte_t, sizeof (Node) > arr;
    std::copy(b.begin(), b.begin() + sizeof (Node), arr.begin());
    from_bytes(arr, *node);
    //return node;
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
            for (int i = readCnt; i < 1.45 * depth; i++) {//4.35
                int rnd = RandomPath();
                if (std::find(leafList.begin(), leafList.end(), rnd) == leafList.end()) {
                    leafList.push_back(rnd);
                }
                FetchPath(rnd);
            }
        }
    }
        int maxHeight = 1;
        for (auto t : cache) {
            if (t.second != NULL && t.second->height > maxHeight) {
                maxHeight = t.second->height;
            }
        }
    //for (unsigned int i = 0; i <= depth + 2; i++) {
    for (unsigned int i = 1; i <= maxHeight; i++) {
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
        Node* node = convertBlockToNode(bucket[0].data);
        cout << node->key << " ";
        delete node;
    }
}

int ORAM::RandomPath() {
    int val = dis(mt);
    return val;
}
void ORAM::setupInsert(vector<Node*> nodes) {
    sort(nodes.begin(), nodes.end(), [ ](const Node* lhs, const Node * rhs) {
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
            int nodeIndex = GetNodeOnPath(curPos, d);
            Bucket bucket;
            if (bucketsCnt.count(nodeIndex) == 0) {
                bucketsCnt[nodeIndex] = 0;
                for (int z = 0; z < Z; z++) {
                    if (i < nodes.size() && nodes[i]->pos == curPos) {
                        Block &curBlock = bucket[z];
                        curBlock.id = nodes[i]->key;
                        curBlock.data = convertNodeToBlock(nodes[i]);
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
                            curBlock.data = convertNodeToBlock(nodes[i]);
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
