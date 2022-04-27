#pragma once
#ifndef BTREENODE_H
#define BTREENODE_H

#include <string>
#include <iostream>
#include <bits/stdc++.h>
#include "Bid.h"

using namespace std;

class BTreeNode 
{
public:
        BTreeNode() { }
       ~BTreeNode() { }

        int bid; 
	int pos; // position of this node
        int knum;   // Current number of keys
        bool isleaf; // Is true when node is leaf. Otherwise false
	int height;
	array<Bid, D-1>keys;
	array<int, D-1>values;
        array<int,D> cbids;
	array<int,D> cpos;

	int findKey(Bid k);
//	void deletion(int k);
	void removeFromLeaf(int idx);

friend class BTree;
};
#endif
