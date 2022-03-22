#include "RAMStore.hpp"
#include <iostream>
#include "ORAM.hpp"
#include "Types.hpp"

using namespace std;

RAMStore::RAMStore(size_t count, size_t size)
: store(count), size(size) , omapfcsc_("OMAPfcsc"){
}

RAMStore::~RAMStore() {
}

block RAMStore::Read(int pos) 
{
    prf_type key;
    memset(key.data(), 0, AES_KEY_SIZE);
    auto arr = to_bytes(pos);
    copy(arr.begin(), arr.end(), key.data());
    block_type val;
    memset(val.data(), 0, BLOCK_SIZE);
    bool found = omapfcsc_.get(key, val);
    block blk;
    if(found)
	    blk.assign(val.begin(), val.end()); 
    
    return blk;
//**********************************************************************    
    //return store[pos];
}

void RAMStore::Write(int pos, block blk) 
{
    prf_type key;
    memset(key.data(), 0, AES_KEY_SIZE);
    auto arr = to_bytes(pos);
    copy(arr.begin(), arr.end(), key.data());
    block_type val;
    std::fill(val.begin(), val.end(), 0);
    std::copy(blk.begin(), blk.end(), val.begin());
    omapfcsc_.put(key,val);
//**********************************************************************    
    //store[pos] = blk;
}

