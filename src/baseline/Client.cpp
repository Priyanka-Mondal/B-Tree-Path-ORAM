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
string delimiters("|+#(){}[]0123456789*?&@=,:!\"><; _-./  \n");
std::set<std::string> stopwords = {"_","a","b","c","d","e","f","g","h","i","j","k","l","m","n","o","p","q","r","s","t","u","v","w","x","y","z","\n","\0", " ", "-","?","from","to", "in","on","so","how""me","you","this","that","ok","no","yes","him","her","they","them","not","none","an","under","below","behind","he","she","their","has","our","would","am","may","know","all","can","any","me","or","as","your","it","we","please","at","if","will","are","by","with","be","com","have","is","of","for","and","the","date","cc","up","but","do","what","which","been","where","could","who","would","did","put","done","too","get","got","yet","co","if"};
vector<string> getUniquedWords(vector<string> kws)
{
    vector<string> kw;
    map<string, int> mp;
    string word;
    for(auto word : kws)
    {
	    if(word.size()<=12 && (stopwords.find(word)==stopwords.end()))
	    {
    		    if ((!mp.count(word)) && (word.size()<=12))
    		    {
    		        mp.insert(make_pair(word, 1));
    		    }
    		    else 
    		        mp[word]++;
	    }
    }
    for (map<string, int> :: iterator p = mp.begin();
         p != mp.end(); p++)
    {
        if (p->second >= 1)
            kw.push_back(p->first) ;
    }
    mp.clear();
return kw;
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
    server->update(addr,head);
}
int Client::getfreq(string kw, int fileid)
{
	int res = 0;
	vector<prf_type> addrs;
	for(int i = 1; i<= fileid; i++)
	{
    		prf_type file; 
   		string id = to_string(i);
    		memset(file.data(), 0, AES_KEY_SIZE); 
   		copy(id.begin(), id.end(), file.data());
   		prf_type addr;
   		getAESRandomValue(file.data(), 0, 1, 1, addr.data());
		addrs.emplace_back(addr);
	}
	vector<FileNode*> heads = server->search(addrs);
	for(auto it = heads.begin(); it != heads.end(); it++)
	{
        	FileNode* head = *it;
		string cont = getfile(head);
		vector<string> kws1, kws;
	        boost::split(kws1, cont, boost::is_any_of(delimiters));
		kws =  getUniquedWords(kws1);
		int flag = 0;
		for(auto it = kws.begin();it!=kws.end();it++)
		{
			if(*it == kw && flag == 0)
			{
			    res++; // 
			    flag = 1;
			}
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


void Client::getAESRandomValue(unsigned char* keyword, int op, int srcCnt, int fileCnt, unsigned char* result) {
    if (deleteFiles) {
        *(int*) (&keyword[AES_KEY_SIZE - 9]) = srcCnt;
    }
    keyword[AES_KEY_SIZE - 5] = op & 0xFF;
    *(int*) (&keyword[AES_KEY_SIZE - 4]) = fileCnt;
    sse::crypto::Prg::derive((unsigned char*) keyword, 0, AES_KEY_SIZE, result);
}

