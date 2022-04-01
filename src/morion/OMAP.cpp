#include "OMAP.h"
using namespace std;

OMAP::OMAP(int maxSize, bytes<Key> key) {
    treeHandler = new AVLTree(maxSize, key);
    rootKey = 0;
}

OMAP::~OMAP() {

}


string OMAP::setupfind(Bid key) {
    if (rootKey == 0) {
        return "";
    }
    treeHandler->startOperation();
    Node* node = new Node();
    node->key = rootKey;
    node->pos = rootPos;
    //cout <<"root is:"<< rootKey<<endl;
    auto resNode = treeHandler->setupsearch(node, key);
    string res = "";
    if (resNode != NULL) {
        res.assign(resNode->value.begin(), resNode->value.end());
        res = res.c_str();
    }
    return res;
}
string OMAP::find(Bid key) {
    if (rootKey == 0) {
        return "";
    }
    treeHandler->startOperation();
    Node* node = new Node();
    node->key = rootKey;
    node->pos = rootPos;
    auto resNode = treeHandler->search(node, key);
    //string res1 = "";
    string res = "";
    if (resNode != NULL) {
        res.assign(resNode->value.begin(), resNode->value.end());
        res = res.c_str();
    }
    treeHandler->finishOperation(true, rootKey, rootPos);
    return res;
}


Bid OMAP::remove(Bid delKey)
{
    treeHandler->startOperation();
    if (rootKey == 0) {
        rootKey = treeHandler->removeMain(0, rootPos, delKey);
    } else {
        rootKey = treeHandler->removeMain(rootKey, rootPos, delKey);
    }
    treeHandler->finishOperation(false, rootKey, rootPos);
    
}

void OMAP::setupInsert(map<Bid, string> pairs) {
    treeHandler->setupInsert(rootKey, rootPos, pairs);
}

void OMAP::setupinsert(Bid key, string value)
{

    if (rootKey == 0) {
        rootKey = treeHandler->setupinsert(0, rootPos, key, value);
//	cout <<rootPos<<"root at OMAP is :"<< rootKey<<endl;
    } else {
        rootKey = treeHandler->setupinsert(rootKey, rootPos, key, value);
//	cout <<rootPos<<"root at OMAP is :"<< rootKey<<endl;
    }
}
void OMAP::insert(Bid key, string value) {
    treeHandler->startOperation();
    if (rootKey == 0) {
	    cout <<"root is 0"<< endl;
        rootKey = treeHandler->insert(0, rootPos, key, value);
    } else {
	    cout <<"root is :"<<rootKey<< endl;
        rootKey = treeHandler->insert(rootKey, rootPos, key, value);
    }
    treeHandler->finishOperation(false, rootKey, rootPos);
}

void OMAP::printTree() {
    treeHandler->startOperation();
    Node* node = new Node();
    node->key = rootKey;
    node->pos = rootPos;
    treeHandler->printTree(node, 0);
    delete node;
    treeHandler->finishOperation(true, rootKey, rootPos);
}

/**
 * This function is used for batch insert which is used at the end of setup phase.
 */
void OMAP::batchInsert(map<Bid, string> pairs) {
    //treeHandler->startOperation(true);
    for (auto pair : pairs) 
    {
    	treeHandler->startOperation();
        if (rootKey == 0) 
	{
            rootKey = treeHandler->insert(0, rootPos, pair.first, pair.second);
        } else 
	{
            rootKey = treeHandler->insert(rootKey, rootPos, pair.first, pair.second);
        }
        treeHandler->finishOperation(false, rootKey, rootPos);
    }
    //treeHandler->finishOperation(false, rootKey, rootPos);
}

/**
 * This function is used for batch search which is used in the real search procedure
 */
vector<string> OMAP::batchSearch(vector<Bid> keys) {
    vector<string> result;
    treeHandler->startOperation(false);
    Node* node = new Node();
    node->key = rootKey;
    node->pos = rootPos;
    vector<Node*> resNodes;
    treeHandler->batchSearch(node, keys, &resNodes);

    for (Node* n : resNodes) {
        string res;
	string res1="";
        if (n != NULL) {
            res1.assign(n->value.begin(), n->value.end());
	    res = res1.c_str();
            result.push_back(res);
        } else {
            result.push_back(""); 
        }
    }
    treeHandler->finishOperation(true, rootKey, rootPos);
    return result;
}
