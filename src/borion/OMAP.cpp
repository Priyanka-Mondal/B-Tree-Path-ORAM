#include "OMAP.h"
using namespace std;

OMAP::OMAP(int maxSize, bytes<Key> key) {
    treeHandler = new AVLTree(maxSize, key);
    rootKey = 0;
}

OMAP::~OMAP() {

}


pair<string,string> OMAP::find(Bid key) {
    if (rootKey == 0) {
        return (make_pair("",""));
    }
    treeHandler->startOperation();
    Node* node = new Node();
    node->key = rootKey;
    node->pos = rootPos;
    auto resNode = treeHandler->search(node, key);
    //string res1 = "";
    pair <string,string> res = make_pair("","");
    if (resNode != NULL) {
        res.first.assign(resNode->value.first.begin(), resNode->value.first.end());
        res.first = res.first.c_str();
        res.second.assign(resNode->value.second.begin(), resNode->value.second.end());
        res.second = res.second.c_str();
    }
    treeHandler->finishOperation(true, rootKey, rootPos);
    return res;
}


Bid OMAP::remove(Bid delKey)
{
	cout <<"in omap remove"<<endl;
	return 0;
	/*
    treeHandler->startOperation();
    if (rootKey == 0) {
        rootKey = treeHandler->remove(0, rootPos, delKey);
    } else {
        rootKey = treeHandler->remove(rootKey, rootPos, delKey);
    }
    treeHandler->finishOperation(false, rootKey, rootPos);
    */
}


void OMAP::insert(Bid key, pair<string,string> value) {
//	cout << "in OMAP insert 6[" << value.second << "]"<< endl;
    treeHandler->startOperation();
    if (rootKey == 0) {
//	cout << "in OMAP inserted as root 7.1[" << value.second << "]"<< endl;
        rootKey = treeHandler->insert(0, rootPos, key, value);
    } else {
  //     cout << "in OMAP inserted as NONroot 7.2[" << value.second << "]"<< endl;
        rootKey = treeHandler->insert(rootKey, rootPos, key, value);
    }
    treeHandler->finishOperation(false, rootKey, rootPos);
    //cout << "in OMAP inserted fake access 8[" << value.second << "]"<< endl;
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
void OMAP::batchInsert(map<Bid, pair<string,string>> pairs) {
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

/**
 * This function is used for batch search which is used in the real search procedure
 */
vector<pair<string,string>> OMAP::batchSearch(vector<Bid> keys) {
    vector<pair<string,string>> result;
    treeHandler->startOperation(false);
    Node* node = new Node();
    node->key = rootKey;
    node->pos = rootPos;

    vector<Node*> resNodes;
    treeHandler->batchSearch(node, keys, &resNodes);
    for (Node* n : resNodes) {
        pair<string,string> res;
	string res1="";
	string res2="";
        if (n != NULL) {
            res1.assign(n->value.first.begin(), n->value.first.end());
	    res.first = res1.c_str();
            res2.assign(n->value.second.begin(), n->value.second.end());
	    res.second = res2.c_str();
            result.push_back(res);
        } else {
		cout << "Pushing empty" << endl;
            result.push_back(make_pair("","")); // not sure if first is -1
        }
    }
    treeHandler->finishOperation(true, rootKey, rootPos);
    return result;
}
