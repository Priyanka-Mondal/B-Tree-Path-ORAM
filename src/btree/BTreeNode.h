#pragma once
#ifndef BTREENODE_H
#define BTREENODE_H

#include <random>
#include <string>
#include <iostream>
#include <bits/stdc++.h>

using namespace std;

class BTreeNode 
{
public:
        BTreeNode();
        BTreeNode(bool leaf, int nextBid, int pos);
       ~BTreeNode() { }

        int bid;
	int pos; // position of this node
	array<string, D-1>keys;
        array<int,D> cbids; // An array of ints
	array<int,D> cpos;
        int n;   // Current number of keys
        bool leaf; // Is true when node is leaf. Otherwise false
	int height;
	int t;


	//int nextbid = 0;
        //int nextBid();
        int RandomPath();
        std::random_device rd;
        std::mt19937 mt;
        std::uniform_int_distribution<int> dis;
        int BRAMleaves = 0;


        void insertNonFull(string kw,BTreeNode *node);
        void splitChild(int i, BTreeNode *y, BTreeNode *&z);
friend class BTree;
};
#endif
/*void BTreeNode::insertNonFull(string k, BTreeNode* node)
{
	cout <<"insertNonFull"<<endl;
	int i = n-1;
	cout <<"i:"<<i<<endl;
	if (leaf == true)
	{
		while (i >= 0 && keys[i] > k)
		{
			keys[i+1] = keys[i];
			i--;
		}
		keys[i+1] = k;
		n = n+1;
	}
	else // If this node is not leaf
	{
		while (i >= 0 && keys[i] > k)
			i--;
		BTreeNode *ci = bram->ReadBTreeNode(cbids[i+1],cpos[i+1],cpos[i+1]);
		if (ci->n == 2*t-1)
		{
			BTreeNode *z = new BTreeNode(true, nextBid(),0);
			splitChild(i+1, ci, z);
			if (keys[i+1] < k)
				i++;
		}
		ci->insertNonFull(k);
	}
}*/
