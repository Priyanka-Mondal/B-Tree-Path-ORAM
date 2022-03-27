#include "Client.h"
#include <openssl/conf.h>
#include <openssl/evp.h>
#include <openssl/err.h>
#include <string.h>
#include <sse/crypto/prg.hpp>

using namespace std;
using namespace boost::algorithm;

Client::Client(Server* server, bool deleteFiles, int keyworsSize, int fileSize) 
{
    this->server = server;
    this->deleteFiles = deleteFiles;
    bytes<Key> key{0};
    omap = new OMAP(keyworsSize, key);
    ac = new OMAPf(fileSize,key);
}
Client::Client(bool deleteFiles, int keyworsSize, int fileSize) 
{
    this->deleteFiles = deleteFiles;
    bytes<Key> key{0};
    omap = new OMAP(keyworsSize, key);
    ac = new OMAPf(fileSize, key);
}

Client::~Client() { }

int to_int(string del_cnt)
{
        int updc;
        stringstream convstoi(del_cnt);
        convstoi >> updc;
        return updc;
}
void append(FileNode** head_ref, fblock new_data)
{
	FileNode* new_node = new FileNode();
	FileNode *last = *head_ref;
	new_node->data = new_data;
	new_node->next = NULL;
	if(*head_ref == NULL)
	{
		*head_ref = new_node;
		return;
	}
	while(last->next != NULL)
		last = last->next;

	last->next = new_node;
	return;
}

int nextPowerOf2(int n)
{
         int count = 0;
	         
         if (n && !(n & (n - 1)))
            return n;
         
	 while( n != 0)
         {
                n >>= 1;
                count += 1;
         }
         return 1 << count;
}

void Client::insert(vector<string> keywords, int ind, bool setup) 
{
    for(string keyword:keywords)
    {
        omap->treeHandler->oram->totalRead = 0;
        omap->treeHandler->oram->totalWrite = 0;
        totalUpdateCommSize = 0;
        prf_type k_w;
        memset(k_w.data(), 0, AES_KEY_SIZE);
        copy(keyword.begin(), keyword.end(), k_w.data());
        int fileCnt = 0, srcCnt = 0;
        Bid mapKey = getBid(keyword);
        string fileCntStr = setup ? setupOMAP[mapKey] : omap->incrementFileCnt(mapKey);
        if (fileCntStr != "") 
        {
            auto parts = Utilities::splitData(fileCntStr, "-");
            fileCnt = stoi(parts[0]);
            srcCnt = stoi(parts[1]);
        }
        fileCnt++;
        if (setup)
            setupOMAP[mapKey] = to_string(fileCnt) + "-" + to_string(srcCnt);
        prf_type addr, rnd;
        getAESRandomValue(k_w.data(), 0, srcCnt, fileCnt, addr.data());
        getAESRandomValue(k_w.data(), 1, srcCnt, fileCnt, rnd.data());
        prf_type val = bitwiseXOR(ind, 0, rnd);
        totalUpdateCommSize=(sizeof(prf_type)*2)+(omap->treeHandler->oram->totalRead+omap->treeHandler->oram->totalWrite)*(sizeof(prf_type)+sizeof(int));
        server->update(addr, val);
    }
}

void Client::insertFile(int ind, string content, bool setup)
{
    prf_type file; 
    memset(file.data(), 0, AES_KEY_SIZE); 
    string id = to_string(ind);
    copy(id.begin(), id.end(), file.data());
    Bid mapKey = getBid(id);
    if(setup)
	    setupAC[mapKey] = to_string(1);
    else
    {
    	    ac->insert(mapKey,to_string(1));
    }
	    accCnt[ind]=1;
    prf_type addr;
    getAESRandomValue(file.data(), 0, 1, 1, addr.data());
    int sz = content.size();
    sz = sz < sizeof(fblock)? sizeof(fblock):nextPowerOf2(sz);
    if(content.size()<sz) 
	  content.insert(content.size(),sz-content.size(),'#');
    FileNode *head = NULL;
    int len = 0;
    while(len < content.size())
    {
	    string part = content.substr(len,sizeof(fblock));
	    fblock val;
	    copy(part.begin(), part.end(), val.data());
	    append(&head,val);
	    len = len+ sizeof(fblock);
    }
    DictF[addr]=head;
}

map<int,string> Client::search(string keyword) 
{
    omap->treeHandler->oram->totalRead = 0;
    omap->treeHandler->oram->totalWrite = 0;
    totalSearchCommSize = 0;
    vector<int> finalRes;
    map<int,string> files;
    vector<prf_type> KList;
    prf_type k_w;
    memset(k_w.data(), 0, AES_KEY_SIZE);
    copy(keyword.begin(), keyword.end(), k_w.data());
    Bid mapKey = getBid(keyword);
    int fileCnt = 0, srcCnt = 0;
    string fileCntStr = omap->find(mapKey);
    if (fileCntStr != "") 
    {
        auto parts = Utilities::splitData(fileCntStr, "-");
        fileCnt = stoi(parts[0]);
        srcCnt = stoi(parts[1]);
    } 
    else 
        return files;
    KList.reserve(fileCnt);
    finalRes.reserve(fileCnt);
    for (int i = 1; i <= fileCnt; i++) 
    {
        prf_type rnd;
        getAESRandomValue(k_w.data(), 0, srcCnt, i, rnd.data());
        KList.emplace_back(rnd);
    }
    vector<prf_type> encIndexes = server->search(KList);
    cout <<"sizeof finalRes:"<< encIndexes.size()<<endl; 
    map<int, int> remove;
    int cnt = 1;
    
    for (auto i = encIndexes.begin(); i != encIndexes.end(); i++) 
    {
        prf_type tmp;
        getAESRandomValue(k_w.data(), 1, srcCnt, cnt, tmp.data());
        prf_type decodedString = *i;
        prf_type plaintextBytes = bitwiseXOR(tmp, decodedString);
        int plaintext = (*((int*) &plaintextBytes[0]));
        remove[plaintext] += (2 * plaintextBytes[4] - 1);
        cnt++;
    }
    fileCnt = 0;
    srcCnt++;
    for (auto const& cur : remove) 
    {
        if (cur.second < 0) 
	{
                finalRes.emplace_back(cur.first);
	cout <<keyword<<" id:"<<cur.first<<endl;
                fileCnt++;
                prf_type addr, rnd;
                getAESRandomValue(k_w.data(), 0, srcCnt, fileCnt, addr.data());
                getAESRandomValue(k_w.data(), 1, srcCnt, fileCnt, rnd.data());
                prf_type val = bitwiseXOR(cur.first, OP::INS, rnd);
                server->update(addr, val);
        }
    }
    totalSearchCommSize += (fileCnt * 2 * sizeof (prf_type));
    omap->insert(mapKey, to_string(fileCnt) + "-" + to_string(srcCnt));
    totalSearchCommSize += sizeof (prf_type) * KList.size() + encIndexes.size() * sizeof (prf_type) + (omap->treeHandler->oram->totalRead + omap->treeHandler->oram->totalWrite)*(sizeof (prf_type) + sizeof (int));
    
    for(int i =0;i< finalRes.size();i++)
    {
	int ind = finalRes[i];
	string id = to_string(ind);
	Bid acKey = getBid(id);
	string acnt = ac->find(acKey);
        int accsCnt;// = accCnt[ind];//to_int(ac->find(acKey));
	if(acnt=="")
		continue; // not possible
	else 
		accsCnt = to_int(acnt);
        prf_type file;
        memset(file.data(), 0, AES_KEY_SIZE);
        copy(id.begin(), id.end(), file.data());
        Bid mapKey = getBid(id);
        prf_type addr;
        getAESRandomValue(file.data(), 0, accsCnt, accsCnt, addr.data());
        FileNode* head = DictF[addr];
	FileNode* newhead = NULL;
        while(head!=NULL)
        {
    	    string temp;
            temp.assign((head->data).begin(),(head->data).end());
    	    if(files.find(ind) == files.end())
    	    	files[ind]= temp;
    	    else
    	    	(files[ind]).append(temp);
    	    FileNode* temphead = head->next;
	    delete head;
	    head = temphead;
	    fblock val;
	    copy(temp.begin(), temp.end(), val.data());
	    append(&newhead,val);
        }
	accsCnt++;
	ac->insert(acKey,to_string(accsCnt));
	accCnt[ind] = accsCnt;
        prf_type newaddr;
        getAESRandomValue(file.data(), 0, accsCnt, accsCnt, newaddr.data());
	DictF.erase(addr);
	if(newhead != NULL)
        	DictF[newaddr]=newhead;
    }
    return files;
}

prf_type Client::bitwiseXOR(int input1, int op, prf_type input2) {
    prf_type result;
    result[3] = input2[3] ^ ((input1 >> 24) & 0xFF);
    result[2] = input2[2] ^ ((input1 >> 16) & 0xFF);
    result[1] = input2[1] ^ ((input1 >> 8) & 0xFF);
    result[0] = input2[0] ^ (input1 & 0xFF);
    result[4] = input2[4] ^ (op & 0xFF);
    for (int i = 5; i < AES_KEY_SIZE; i++) {
        result[i] = (rand() % 255) ^ input2[i];
    }
    return result;
}

prf_type Client::bitwiseXOR(prf_type input1, prf_type input2) {
    prf_type result;
    for (unsigned int i = 0; i < input2.size(); i++) {
        result[i] = input1.at(i) ^ input2[i];
    }
    return result;
}

void Client::getAESRandomValue(unsigned char* keyword, int op, int srcCnt, int fileCnt, unsigned char* result) {
    if (deleteFiles) {
        *(int*) (&keyword[AES_KEY_SIZE - 9]) = srcCnt;
    }
    keyword[AES_KEY_SIZE - 5] = op & 0xFF;
    *(int*) (&keyword[AES_KEY_SIZE - 4]) = fileCnt;
    sse::crypto::Prg::derive((unsigned char*) keyword, 0, AES_KEY_SIZE, result);
}

double Client::getTotalSearchCommSize() const {
    return totalSearchCommSize;
}

double Client::getTotalUpdateCommSize() const {
    return totalUpdateCommSize;
}

Bid Client::getBid(string input) {
    std::array< uint8_t, ID_SIZE> value;
    std::fill(value.begin(), value.end(), 0);
    std::copy(input.begin(), input.end(), value.begin());
    Bid res(value);
    return res;
}
void Client::endSetup() 
{
        omap->setupInsert(setupOMAP);
	ac->setupInsert(setupAC);
}
