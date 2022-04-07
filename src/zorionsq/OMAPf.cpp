#include "OMAPf.h"
using namespace std;

OMAPf::OMAPf(int maxSize, bytes<Key> key) {
    treeHandler = new AVLTreef(maxSize, key);
    rootKey = 0;
}

OMAPf::~OMAPf() {
delete treeHandler;
}

int stoint(string del_cnt)
{
        int updc;
        stringstream convstoi(del_cnt);
        convstoi >> updc;
        return updc;
}
/*
int OMAPf::setupfind(Bid key) {
    if (rootKey == 0) {
        return 0;
    }
//    treeHandler->startOperation();
    Nodef* node = new Nodef();
    node->key = rootKey;
    node->pos = rootPos;
    Nodef* resNode = treeHandler->setupsearch(node, key);
    int res;
    if (resNode != NULL) 
    {
	string res2;
        res2.assign(resNode->value.begin(), resNode->value.end());
        res2 = res2.c_str();
	res = stoint(res2);
    }
    //else if(resNode == NULL)
//	    cout <<"Null recieved in OMAPf for"<< key<< endl;
delete node;	    
delete resNode;
    return res;
}*/

int OMAPf::setupfind(Bid key) {
    if (rootKey == 0) {
        return 0;
    }
    string resNode = treeHandler->setupsimplesearch(rootKey,rootPos,key);
    int res=0;
    if (resNode != "") 
    {
	//string res2;
        //res2.assign(resNode.begin(), resNode.end());
        //res2 = res2.c_str();
	res = stoint(resNode);
    }
    //else if(resNode == NULL)
//	    cout <<"Null recieved in OMAPf for"<< key<< endl;
return res;
}
void OMAPf::setupInsert(map<Bid, int> pairs) {
    treeHandler->setupInsert(rootKey, rootPos, pairs);
}
int OMAPf::find(Bid key) {
    if (rootKey == 0) {
        return 0;
    }
    treeHandler->startOperation();
    Nodef* node = new Nodef();
    node->key = rootKey;
    node->pos = rootPos;
    Nodef* resNode = treeHandler->search(node, key);
    int res;
    if (resNode != NULL) 
    {
	string res2;
        res2.assign(resNode->value.begin(), resNode->value.end());
        res2 = res2.c_str();
	res = stoint(res2);
    }
    treeHandler->finishOperation(true, rootKey, rootPos);
delete node;	    
//delete resNode;
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

void OMAPf::setupinsert(Bid key, int value)
{
    //treeHandler->startOperation();
    if (rootKey == 0) {
        rootKey = treeHandler->setupinsert(0, rootPos, key, value);
//	cout <<rootPos<<"root at OMAP fis :"<< rootKey<<endl;
    } else {
        rootKey = treeHandler->setupinsert(rootKey, rootPos, key, value);
	//cout <<rootPos<<":root:"<< rootKey<<endl;
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
    treeHandler->printTree(node, 10);
    delete node;
    treeHandler->finishOperation(true, rootKey, rootPos);
    cout << endl << endl;
}

void OMAPf::batchInsert(map<Bid, int> pairs) 
{
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
        if (n != NULL) 
	{
	    string res2;
            res2.assign(n->value.begin(), n->value.end());
	    res = stoint(res2);
            result.push_back(res);
        } else {
            result.push_back(0); // in oriel it is commented
        }
    }
    treeHandler->finishOperation(true, rootKey, rootPos);
    delete node;
    resNodes.clear();
    return result;
}
