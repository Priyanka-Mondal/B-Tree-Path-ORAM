
class Fnode {
public:

    Fnode() {}
    ~Fnode(){}

    Fbid key;
    std::array< byte_t, 64> value;
    int pos;
};

struct Fblock {
    Fbid id;
    block data;
};

using Fbucket = std::array<Fblock, Z>;

struct FbidHasher {

    std::size_t operator()(const Fbid &key) const {
        std::hash<byte_t> hasher;
        size_t result = 0; // I would still seed this.
        for (size_t i = 0; i < ID_SIZE; ++i) {
            result = (result << 1) ^ hasher(key.id[i]); // ??
        }
        return result;
    }
};

