#include "Types.hpp"

class Node {
public:

    Node() {}
    ~Node(){}

    Bid key;
    std::array< byte_t, BLOCK> value;
    int pos;
};

struct Block {
    Bid id;
    block data;
};

using Bucket = std::array<Block, Z>;

struct BidHasher {

    std::size_t operator()(const Bid &key) const {
        std::hash<byte_t> hasher;
        size_t result = 0; // I would still seed this.
        for (size_t i = 0; i < ID_SIZE; ++i) {
            result = (result << 1) ^ hasher(key.id[i]); // ??
        }
        return result;
    }
};

