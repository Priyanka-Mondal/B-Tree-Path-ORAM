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
	int pos; 
        bool isleaf;
	//int knum;
	int height;
	array<Bid, D-1>keys;
	array<int, D-1>values;
        array<int,D> cbids;
	array<int,D> cpos;

	int findKey(Bid k);

friend class BTree;
};
#endif
