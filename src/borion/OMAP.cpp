#include "OMAP.h"
using namespace std;

OMAP::OMAP(int maxSize, bytes<Key> key) {
    treeHandler = new AVLTree(maxSize, key);
    rootKey = 0;
}

OMAP::~OMAP() {

}

pair<int,string> OMAP::find(Bid key) {
    if (rootKey == 0) {
        return (make_pair(-1,""));
    }
    treeHandler->startOperation();
    Node* node = new Node();
    node->key = rootKey;
    node->pos = rootPos;
    auto resNode = treeHandler->search(node, key);
    //string res1 = "";
    pair <int,string> res;
    if (resNode != NULL) {
	res.first=resNode->value.first;
        res.second.assign(resNode->value.second.begin(), resNode->value.second.end());
        res.second = res.second.c_str();
    }
    treeHandler->finishOperation(true, rootKey, rootPos);
    return res;
}

void OMAP::insert(Bid key, pair<int,string> value) {
    treeHandler->startOperation();
    if (rootKey == 0) {
        rootKey = treeHandler->insert(0, rootPos, key, value);
    } else {
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
void OMAP::batchInsert(map<Bid, pair<int,string>> pairs) {
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
vector<pair<int,string>> OMAP::batchSearch(vector<Bid> keys) {
    vector<pair<int,string>> result;
    treeHandler->startOperation(false);
    Node* node = new Node();
    node->key = rootKey;
    node->pos = rootPos;

    vector<Node*> resNodes;
    treeHandler->batchSearch(node, keys, &resNodes);
    for (Node* n : resNodes) {
        pair<int,string> res;
	string res1="";
        if (n != NULL) {
            res.first=n->value.first;
            res1.assign(n->value.second.begin(), n->value.second.end());
	    res.second = res1.c_str();
            result.push_back(res);
        } else {
            result.push_back(make_pair(-1,"")); // not sure if first ia -1
        }
    }
    treeHandler->finishOperation(true, rootKey, rootPos);
    return result;
}
