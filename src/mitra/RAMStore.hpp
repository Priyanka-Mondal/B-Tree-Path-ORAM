#pragma once
#include <map>
#include <array>
#include <vector>
#include "utils/rocksdb_wrapper.hpp"

using namespace std;

using byte_t = uint8_t;
using block = std::vector<byte_t>;

class RAMStore {
    std::vector<block> store;
    size_t size;

public:
    RAMStore(size_t num, size_t size);
    ~RAMStore();
    sse::sophos::RockDBWrapper omapfcsc_;
    block Read(int pos);
    void Write(int pos, block b);

};
