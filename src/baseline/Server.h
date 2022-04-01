#ifndef SERVER_H
#define SERVER_H
#include "Types.hpp"
typedef uint64_t index_type;

using namespace std;

class FileNode
{
 public:
 FileNode(){ };
    ~FileNode() { };

   fblock data;
     FileNode *next;
};
class Server {
private:

    bool deleteFiles;

public:
    map<prf_type, FileNode*> DictF;

    Server(bool deleteFiles);
    void update(prf_type addr, FileNode* val);
    vector<FileNode*> search(vector<prf_type> KList);
    virtual ~Server();

};

#endif /* SERVER_H */

