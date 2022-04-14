#include "Types.hpp"

class Node {
public:

    Node() {}
    ~Node(){}

    Bid key;
    std::array< byte_t, NODE_SIZE> value;
    int pos;
};

struct Iblock {
    Bid id;
    block data;
};

using Ibucket = std::array<Iblock, Z>;

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

