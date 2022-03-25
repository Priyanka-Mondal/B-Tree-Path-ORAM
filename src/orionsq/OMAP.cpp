#include "OMAP.h"
using namespace std;

OMAP::OMAP(int maxSize, bytes<Key> key) {
    treeHandler = new AVLTree(maxSize, key);
    rootKey = 0;
}

OMAP::~OMAP() {
delete treeHandler;
}
/*
string OMAP::setupfind(Bid key) {
    if (rootKey == 0) {
        return "";
    }
//    treeHandler->startOperation();
    Node* node = new Node();
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
delete node;
//delete resNode;
return res;

}*/
string OMAP::setupfind(Bid key) {
    if (rootKey == 0) {
        return 0;
    }
    string resNode = treeHandler->setupsimplesearch(rootKey,rootPos,key);
    int res;
return resNode;
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
    string res = "";
    if (resNode != NULL) {
        res.assign(resNode->value.begin(), resNode->value.end());
        res = res.c_str();
    }
    treeHandler->finishOperation(true, rootKey, rootPos);
delete node;
delete resNode;
return res;
}

void OMAP::setupinsert(Bid key, string value)
{
    if (rootKey == 0) {
        rootKey = treeHandler->setupinsert(0, rootPos, key, value);
    } else {
        rootKey = treeHandler->setupinsert(rootKey, rootPos, key, value);
    }
}
void OMAP::insert(Bid key, string value) {
    treeHandler->startOperation();
    if (rootKey == 0) {
        rootKey = treeHandler->insert(0, rootPos, key, value);
    } else {
        rootKey = treeHandler->insert(rootKey, rootPos, key, value);
    }
    treeHandler->finishOperation(false, rootKey, rootPos);
}


void OMAP::remove(Bid delKey)
{
    treeHandler->startOperation();
    if (rootKey == 0) {
        rootKey = treeHandler->removeMain(0, rootPos, delKey);
    } else {
        rootKey = treeHandler->removeMain(rootKey, rootPos, delKey);
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
        if (n != NULL) {
            res.assign(n->value.begin(), n->value.end());
            result.push_back(res);
        } else {
            result.push_back("");
        }
    }
    treeHandler->finishOperation(true, rootKey, rootPos);
    delete node;
    resNodes.clear();
    return result;
}
