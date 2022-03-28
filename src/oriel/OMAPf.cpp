#include "OMAPf.h"
using namespace std;

OMAPf::OMAPf(int maxSize, bytes<Key> key) {
    treeHandler = new AVLTreef(maxSize, key);
    rootKey = 0;
}

OMAPf::~OMAPf() {

}

int stoint(string del_cnt)
{
        int updc;
        stringstream convstoi(del_cnt);
        convstoi >> updc;
        return updc;
}
int OMAPf::find(Bid key) {
    if (rootKey == 0) {
        return 0;
    }
    treeHandler->startOperation();
    Nodef* node = new Nodef();
    node->key = rootKey;
    node->pos = rootPos;
    auto resNode = treeHandler->search(node, key);
    int res;
    if (resNode != NULL) 
    {
	string res2="";
        res2.assign(resNode->value.begin(), resNode->value.begin()+4);
        res2 = res2.c_str();
	res = stoint(res2);
    }
    treeHandler->finishOperation(true, rootKey, rootPos);
    return res;
}


int OMAPf::setupfind(Bid key) {
    if (rootKey == 0) {
        return 0;
    }
//    treeHandler->startOperation();
    Nodef* node = new Nodef();
    node->key = rootKey;
    node->pos = rootPos;
    auto resNode = treeHandler->setupsearch(node, key);
    int res ; 
    if (resNode != NULL) 
    {
        string res2;
        res2.assign(resNode->value.begin(), resNode->value.end());
        res2 = res2.c_str();
	res = stoint(res2);
    }
    //else if(resNode == NULL)
//	    cout <<"Null recieved in OMAPf for"<< key<< endl;
    return res;
}

void OMAPf::setupInsert(map<Bid, int> pairs) {
    treeHandler->setupInsert(rootKey, rootPos, pairs);
}
void OMAPf::remove(Bid delKey)
{
    treeHandler->startOperation();
    if (rootKey == 0) 
    {
        rootKey = treeHandler->removeMain(0, rootPos, delKey);
    } 
    else 
    {
        rootKey = treeHandler->removeMain(rootKey, rootPos, delKey);
    }
    treeHandler->finishOperation(false, rootKey, rootPos);
}


void OMAPf::setupinsert(Bid key, int value)
{
    //treeHandler->startOperation();
    if (rootKey == 0) 
    {
        rootKey = treeHandler->setupinsert(0, rootPos, key, value);
	//cout <<rootPos<<"root at OMAP is :"<< rootKey<<endl;
    } 
    else 
    {
        rootKey = treeHandler->setupinsert(rootKey, rootPos, key, value);
	//cout <<rootPos<<"root at OMAP is :"<< rootKey<<endl;
    }
}


void OMAPf::insert(Bid key, int value) {
    treeHandler->startOperation();
    if (rootKey == 0) {
        rootKey = treeHandler->insert(0, rootPos, key, value);
    } else {
        rootKey = treeHandler->insert(rootKey, rootPos, key, value);
    }
    treeHandler->finishOperation(false, rootKey, rootPos);
}

void OMAPf::printTree() {
    treeHandler->startOperation();
    Nodef* node = new Nodef();
    node->key = rootKey;
    node->pos = rootPos;
    treeHandler->printTree(node, 0);
    delete node;
    treeHandler->finishOperation(true, rootKey, rootPos);
}

/**
 * This function is used for batch insert which is used at the end of setup phase.
void OMAPf::batchInsert(map<Bid, string> pairs) {
    treeHandler->startOperation(true);
    for (auto pair : pairs) {
	    Bid key = pair.first;
        if (rootKey == 0) {
            rootKey = treeHandler->insert(0, rootPos, pair.first, pair.second);
        } else {
            rootKey = treeHandler->insert(rootKey, rootPos, pair.first, pair.second);
        }
    }
    treeHandler->finishOperation(false, rootKey, rootPos);
}

 */
/**
 * This function is used for batch search which is used in the real search procedure
 */
vector<int> OMAPf::batchSearch(vector<Bid> keys) {
    vector<int> result;
    treeHandler->startOperation(false);
    Nodef* node = new Nodef();
    node->key = rootKey;
    node->pos = rootPos;

    vector<Nodef*> resNodes;
    treeHandler->batchSearch(node, keys, &resNodes);
    for (Nodef* n : resNodes) {
        int res;
        if (n != NULL) {
		string res2;
            res2.assign(n->value.begin(), n->value.end());
	    res = stoint(res2);
            result.push_back(res);
       // } else {
         //   result.push_back(0);
        }
    }
    treeHandler->finishOperation(true, rootKey, rootPos);
    return result;
}
