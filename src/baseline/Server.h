#ifndef SERVER_H
#define SERVER_H
#include "Types.hpp"
typedef uint64_t index_type;

using namespace std;

class Server {
private:

    bool deleteFiles;

public:
    map<prf_type, prf_type > DictF;

    Server(bool deleteFiles);
    void update(prf_type addr, prf_type val);
    vector<prf_type> search(vector<prf_type> KList);
    virtual ~Server();

};

#endif /* SERVER_H */

