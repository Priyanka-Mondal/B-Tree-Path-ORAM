#include "OMAPf.h"
using namespace std;

OMAPf::OMAPf(int maxSize, bytes<Key> key) {
    treeHandler = new AVLTreef(maxSize, key);
    rootKey = 0;
}

OMAPf::~OMAPf() {

}

string OMAPf::setupfind(Bid key) {
    if (rootKey == 0) {
        return "";
    }
//    treeHandler->startOperation();
    Nodef* node = new Nodef();
    node->key = rootKey;
    node->pos = rootPos;
    auto resNode = treeHandler->setupsearch(node, key);
    string res = "";
    if (resNode != NULL) {
        res.assign(resNode->value.begin(), resNode->value.end());
        res = res.c_str();
    }
    //else if(resNode == NULL)
//	    cout <<"Null recieved in OMAPf for"<< key<< endl;
    return res;
}
string OMAPf::find(Bid key) {
    if (rootKey == 0) {
        return "";
    }
    treeHandler->startOperation();
    Nodef* node = new Nodef();
    node->key = rootKey;
    node->pos = rootPos;
    auto resNode = treeHandler->search(node, key);
    string res = "";
    if (resNode != NULL) {
        res.assign(resNode->value.begin(), resNode->value.end());
        res = res.c_str();
    }
    treeHandler->finishOperation(true, rootKey, rootPos);
    return res;
}


void OMAPf::remove(Bid delKey) 
{
    treeHandler->startOperation();
    if (rootKey == 0) {
        rootKey = treeHandler->removeMain(0, rootPos, delKey);
    } else {
//	    cout <<"(begin)OMAPf: Root is"<<rootKey<<endl;
        rootKey = treeHandler->removeMain(rootKey, rootPos, delKey);
//	cout <<"(end)OMAPf: Root is:"<< rootKey<<endl;
    }
    treeHandler->finishOperation(false, rootKey, rootPos);
}

void OMAPf::setupinsert(Bid key, string value)
{
    //treeHandler->startOperation();
    if (rootKey == 0) {
        rootKey = treeHandler->setupinsert(0, rootPos, key, value);
	//cout <<rootPos<<"root at OMAP is :"<< rootKey<<endl;
    } else {
        rootKey = treeHandler->setupinsert(rootKey, rootPos, key, value);
	//cout <<rootPos<<"root at OMAP is :"<< rootKey<<endl;
    }
}
void OMAPf::insert(Bid key, string value) {
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
    treeHandler->printTree(node, 10);
    delete node;
    treeHandler->finishOperation(true, rootKey, rootPos);
    cout << endl << endl;
}

/**
 * This function is used for batch insert which is used at the end of setup phase.
 */
void OMAPf::batchInsert(map<Bid, string> pairs) {
    treeHandler->startOperation(true);
    int cnt = 0;
    for (auto pair : pairs) {
        cnt++;
        if (cnt % 1000 == 0) {
            cout << cnt << " items inserted in AVL of " << pairs.size() << endl;
        }
        if (rootKey == 0) {
            rootKey = treeHandler->insert(0, rootPos, pair.first, pair.second);
        } else {
            rootKey = treeHandler->insert(rootKey, rootPos, pair.first, pair.second);
        }
    }
    treeHandler->finishOperation(false, rootKey, rootPos);
}

/**
 * This function is used for batch search which is used in the real search procedure
 */
vector<string> OMAPf::batchSearch(vector<Bid> keys) {
    vector<string> result;
    treeHandler->startOperation(false);
    Nodef* node = new Nodef();
    node->key = rootKey;
    node->pos = rootPos;

    vector<Nodef*> resNodes;
    treeHandler->batchSearch(node, keys, &resNodes);
    for (Nodef* n : resNodes) {
        string res;
        if (n != NULL) {
            res.assign(n->value.begin(), n->value.end());
            result.push_back(res);
        } else {
            result.push_back("");
        }
    }
    treeHandler->finishOperation(true, rootKey, rootPos);
    return result;
}
