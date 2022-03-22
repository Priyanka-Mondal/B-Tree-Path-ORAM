#ifndef SERVER_H
#define SERVER_H
#include "Types.hpp"
typedef uint64_t index_type;

using namespace std;

class Server {
private:

    bool deleteFiles;
    bool useRocksDB;

public:
    sse::sophos::RockDBWrapper edb_;
    map<prf_type, prf_type > DictW;

    Server(bool useHDD, bool deleteFiles);
    void update(prf_type addr, prf_type val);
    vector<prf_type> search(vector<prf_type> KList);
    virtual ~Server();

};

#endif /* SERVER_H */

