#include <openssl/conf.h>
#include <openssl/evp.h>
#include <openssl/err.h>
#include <string.h>
#include <sse/crypto/prg.hpp>
#include "Client.h"

using namespace std;
using namespace boost::algorithm;

Client::Client(Server* server, bool deleteFiles, int keyworsSize, int fileSize) 
{
    this->server = server;
    this->deleteFiles = deleteFiles;
    bytes<Key> key{0};
    clen_size = AES::GetCiphertextLength(BLOCK);
}
Client::Client(bool deleteFiles, int keyworsSize, int fileSize) 
{
    this->deleteFiles = deleteFiles;
    bytes<Key> key{0};
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


void Client::insertFile(int ind, string content)
{
    prf_type file; 
    memset(file.data(), 0, AES_KEY_SIZE); 
    string id = to_string(ind);
    copy(id.begin(), id.end(), file.data());
    prf_type addr;
    getAESRandomValue(file.data(), 0, 1, 1, addr.data());

    int sz = content.size();
    sz = sz < BLOCK? BLOCK:nextPowerOf2(sz);
    if(content.size()<sz) 
	  content.insert(content.size(),sz-content.size(),'#');
    FileNode *head = NULL;
    int len = 0;
    while(len < content.size())
    {
	    string part = content.substr(len,BLOCK);
	    fblock plaintext;
	    plaintext.insert(plaintext.end(),part.begin(), part.end());
    	    block ciphertext = AES::Encrypt(key, plaintext, clen_size, BLOCK);
	    append(&head,ciphertext);
	    len = len + BLOCK;
    }

    DictF[addr]=head;
}
int Client::getfreq(string kw, int fileid)
{
	int res = 0;
	for(int i = 1; i<= fileid; i++)
	{
    		prf_type file; 
   		string id = to_string(i);
    		memset(file.data(), 0, AES_KEY_SIZE); 
   		copy(id.begin(), id.end(), file.data());
   		prf_type addr;
   		getAESRandomValue(file.data(), 0, 1, 1, addr.data());
        	FileNode* head = DictF[addr];
		string cont = getfile(head);
		//cout << cont<< endl;
		if(cont.find(kw)!= std::string::npos)
		{
			res++;
			//cout <<"Found in:"<< i<< endl;
		}
		
	}
	return res;
}
string Client::getfile(FileNode* head) 
{
	string filedata;
        while(head!=NULL)
        {
    	    fblock ciphertext;
            ciphertext.insert(ciphertext.end(),(head->data).begin(),(head->data).end());
	    fblock plaintext  = AES::Decrypt(key, ciphertext, clen_size);
	    string temp;
	    temp.assign(plaintext.begin(),plaintext.end());
    	    filedata.append(temp);
	    head= head->next;
        }
	// decrypt
    return filedata;
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
