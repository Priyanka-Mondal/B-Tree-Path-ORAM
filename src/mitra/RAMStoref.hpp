#pragma once
#include <map>
#include <array>
#include <vector>
//#include "utils/rocksdb_wrapper.hpp"

using namespace std;

using byte_t = uint8_t;
using block = std::vector<byte_t>;

class RAMStoref {
    std::vector<block> store;
    size_t size;

public:
    RAMStoref(size_t num, size_t size);
    ~RAMStoref();
    //sse::sophos::RockDBWrapper omapac_;
    block Read(int pos);
    void Write(int pos, block b);

};
