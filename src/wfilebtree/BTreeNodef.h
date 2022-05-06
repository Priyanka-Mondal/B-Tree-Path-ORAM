#pragma once
#ifndef BTREENODEF_H
#define BTREENODEF_H

#include <string>
#include <iostream>
#include <bits/stdc++.h>
#include "Bid.h"

using namespace std;

class BTreeNodef 
{
public:
        BTreeNodef() { }
       ~BTreeNodef() { }

        int bid; 
	int pos; 
        //bool isleaf;
	//int knum;
	//int height;
	array<Bid, D-1>keys;
	array<array<byte_t,BLOCK>, D-1>values;
        array<int,D> cbids;
	array<int,D> cpos;

	int findKey(Bid k);

friend class BTreef;
};
#endif
