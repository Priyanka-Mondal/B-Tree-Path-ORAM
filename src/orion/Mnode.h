
class Mnode {
public:

    Mnode() {}
    ~Mnode(){}

    Bid key;
    std::array< byte_t, 64> value;
    int pos;
    Bid nextID;
    int nextPos;
};

struct Oblock {
    Bid id;
    block data;
};

using Bucket = std::array<Oblock, Z>;

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

