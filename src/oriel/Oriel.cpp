#include "Oriel.h"
#include<string>
#include<math.h>
#include<queue>
#include <unistd.h>
#include<tuple>
#include<ctime>
#include <string.h>

int inserted = 0;
int uniquekw = 0;

Oriel::Oriel(bool usehdd, int maxSize,int filesize, bool local) 
{
    this->useHDD = usehdd;
    this->local = local;
    bytes<Key> key1{0};
    bytes<Key> key2{1};
    I = new OMAPf(maxSize, key1);
    //del = new OMAPf(maxSize,key2);
    ac = new OMAPf(filesize, key2); 
    clen_size = AES::GetCiphertextLength(BLOCK);
}

int stoI(string del_cnt)
{
        int updc;
        stringstream convstoi(del_cnt);
        convstoi >> updc;
        return updc;
}

string delimiters("|+#(){}[]0123456789*?&@=,:!\"><; _-./ \n\t");

std::set<std::string> neg = {"_","a","b","c","d","e","f","g","h","i","j","k","l","m","n","o","p","q","r","s","t","u","v","w","x","y","z","\n","\0", " ", "-","?","from","to", "in","on","so","how""me","you","this","that","ok","no","yes","him","her","they","them","not","none","an","under","below","behind","he","she","their","has","our","would","am","may","know","all","can","any","me","or","as","your","it","we","please","at","if","will","are","by","with","be","com","have","is","of","for","and","the","date","cc","up","but","do","what","which","been","where","could","who","would","did","put","done","too","get","got","yet","co","if"};
vector<string> getUniquedWords(vector<string> kws)
{
    vector<string> kw;
    map<string, int> mp;
    string word;
    for(auto word : kws)
    {
	    if(word.size()<=12 && (neg.find(word)==neg.end()))
	    {
    		    if ((!mp.count(word)) && (word.size()<=12))
    		        mp.insert(make_pair(word, 1));
    		    else 
    		        mp[word]++;
	    }
    }
    for (map<string, int> :: iterator p = mp.begin(); p != mp.end(); p++)
    {
        if (p->second >= 1)
            kw.push_back(p->first) ;
    }
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


Oriel::~Oriel() 
{
    delete I;
    //delete del;
    delete ac;
}

void Oriel::insertWrap(string cont, int ind, bool batch)
{
      vector<string> kws1, kws;
      boost::split(kws1, cont, boost::is_any_of(delimiters));
      kws =  getUniquedWords(kws1);
      for (auto it = kws.begin(); it != kws.end(); it++)
      {
	      if(neg.find(*it)!=neg.end())
	      {
		 kws.erase(it--);
	      }
      }
      	insertWrapper(kws, cont, ind, batch);
}


void Oriel::insertWrapper(vector<string> kws, string content,int ind,bool batch)
{ 
     if(batch)
     {
     	for(auto kw: kws) 
	    setupInsertkw(kw,ind);
     }
     else
     {
	 for(auto kw: kws)
   	    insertkw(kw,ind); 
     } 
     cout << "inserted keywords (total:" <<kws.size() <<")for id:"<<ind<< endl;
     insertFile(ind,content,batch); 
     //inserted=inserted+kws.size();
     //cout << endl<<"--TOTAL keywords inserted so far: "<<inserted<<endl;
     //cout <<"--TOTAL unique keywords inserted so far: "<<uniquekw<<endl;
}

void Oriel::insertkw(string keyword, int ind) 
{
    Bid mapKey(keyword);
    int updc = (I->find(mapKey));
    if (updc == 0) 
	 uniquekw++;  
    updc++;
    I->insert(mapKey,updc);
    Bid updKey = createBid(keyword, ind);
    //del->insert(updKey, updc);
    Bid key = createBid(keyword, updc);
    I->insert(key, ind);
}
    
void Oriel::setupinsertkw(string keyword, int ind) 
{
    Bid mapKey(keyword);
    int updc = (I->setupfind(mapKey));
    if (updc == 0) 
	 uniquekw++;  
    updc++;
    I->setupinsert(mapKey,updc);
    Bid updKey = createBid(keyword, ind);
    //del->setupinsert(updKey, updc);
    Bid key = createBid(keyword, updc);
    I->setupinsert(key, ind);
}
void Oriel::setupInsertkw(string keyword, int ind) 
{
    Bid mapKey(keyword);
    int updc = 0;
    if (Imap.count(mapKey)>0)
    	updc = Imap[mapKey];
    //if (updc == 0) 
//	 uniquekw++;  
    updc++;
    Imap[mapKey]=updc;
    Bid key = createBid(keyword, updc);
    Imap[key]= ind;
}

void Oriel::insertFile(int ind, string content,bool batch)
{
    prf_type file; 
    memset(file.data(), 0, AES_KEY_SIZE); 
    string id = to_string(ind);
    copy(id.begin(), id.end(), file.data());
    Bid mapKey(id);
    if(batch && !local)
	acmap[mapKey]=1;
    else if(!local)
    	ac->insert(mapKey,1);
    else 
	localAC[ind]=1;
    prf_type addr;
    getAESRandomValue(file.data(), 0, 1, 1, addr.data());
    int sz = content.size();
    sz = (sz / BLOCK ) * BLOCK;
    if (sz == 0)
	    sz = BLOCK;
    FileNode *head = NULL;
    int len = 0;
    while(len < sz)
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
void Oriel::endSetup()
{
	I->setupInsert(Imap);
	if(!local)
	   ac->setupInsert(acmap);
}

vector <string> Oriel::simplebatchSearch(string keyword)
{
    vector<string> files;
    Bid mapKey(keyword);
    int updc = I->find(mapKey); 
    //cout <<"UPDC:"<< updc<< endl;
    if (updc == 0)
        return files;
    vector<Bid> bids;
    bids.reserve(updc);
    for(int pos=1;pos<= updc; pos++)
    {
    	mapKey =createBid(keyword,pos);
	bids.push_back(mapKey);
    }
    vector<int> ids = I->batchSearch(bids); // add y^m - updc fake accesses
    for(int ind : ids)
    {
	string id = to_string(ind);
	int accsCnt = localAC[ind];
        prf_type file;
        memset(file.data(), 0, AES_KEY_SIZE);
        copy(id.begin(), id.end(), file.data());
        prf_type addr;
        getAESRandomValue(file.data(), 0, accsCnt, accsCnt, addr.data());
        FileNode* head = DictF[addr];
	FileNode* newhead = NULL;
	fblock plaintext;
        while(head!=NULL)
        {
    	    fblock ciphertext;
            ciphertext.insert(ciphertext.end(),(head->data).begin(),(head->data).end());
	    plaintext  = AES::Decrypt(key, ciphertext, clen_size);
	    string temp;
	    temp.assign(plaintext.begin(),plaintext.end());
	    files.push_back(temp);
	    FileNode* temphead;
    	    temphead = head->next;
	    delete head;
	    head = temphead;
	    append(&newhead,ciphertext);
        }
	localAC[ind]=++accsCnt;
	DictF.erase(addr);
        getAESRandomValue(file.data(), 0, accsCnt, accsCnt, addr.data());
        DictF[addr]=newhead;
    }
    return files;
}
vector <string> Oriel::search(string keyword)
{
    vector<string> files;
    Bid mapKey(keyword);
    int updc = I->find(mapKey); 
    //cout <<"UPDC:"<< updc<< endl;
    if (updc == 0)
        return files;
    int pos = 1;
    vector<Bid> bids;
    while(pos <= updc)
    {
    	mapKey =createBid(keyword,pos);
	pos++;
	bids.push_back(mapKey);
    }
    vector<int> ids = I->batchSearch(bids); // add y^m - updc fake accesses
    for(int ind : ids)
    {
        //int ind = I->find(mapKey);
	string id = to_string(ind);
	Bid acKey(id);
	int accsCnt;
	if(!local)
            accsCnt = ac->find(acKey);
	else 
            accsCnt = localAC[ind];
        prf_type file;
        memset(file.data(), 0, AES_KEY_SIZE);
        copy(id.begin(), id.end(), file.data());
        Bid mapKey(id);
        prf_type addr;
        getAESRandomValue(file.data(), 0, accsCnt, accsCnt, addr.data());
        FileNode* head = DictF[addr];
	FileNode* newhead = NULL;
        while(head!=NULL)
        {
    	    fblock ciphertext;
            ciphertext.insert(ciphertext.end(),(head->data).begin(),(head->data).end());
	    fblock plaintext  = AES::Decrypt(key, ciphertext, clen_size);
	    string temp;
	    temp.assign(plaintext.begin(),plaintext.end());
	    files.push_back(temp);
    	    FileNode* temphead = head->next;
	    delete head;
	    head = temphead;
	    append(&newhead,ciphertext);
        }
	if(!local)
		ac->insert(acKey,(++accsCnt));
	else
		localAC[ind]=++accsCnt;
        prf_type newaddr;
        getAESRandomValue(file.data(), 0, accsCnt, accsCnt, newaddr.data());
	DictF.erase(addr);
        DictF[newaddr]=newhead;
    }
//cout <<"size of files:"<<files.size()<<endl;
    return files;
}

/*
void Oriel::remove(int ind) 
{
	string id = to_string(ind);
	Bid acKey(id);
	int accsCnt = ac->find(acKey);
	if(accsCnt==0)
	{
		cout <<" File does NOT EXIST!"<<endl;
		return;
	}
        ac->remove(acKey);	
        prf_type fileid;
        memset(fileid.data(), 0, AES_KEY_SIZE);
        copy(id.begin(), id.end(), fileid.data());
        prf_type addr;
        getAESRandomValue(fileid.data(), 0, accsCnt, accsCnt, addr.data());
        FileNode* head = DictF[addr];
	string file="";
        while(head!=NULL)
        {
    	    string temp;
            temp.assign((head->data).begin(),(head->data).end());
    	    file.append(temp);
    	    head = head->next;
        }
	vector<string> kws1;
	boost::split(kws1, file, boost::is_any_of(delimiters));
	vector<string> kws = getUniquedWords(kws1);
	removekw(kws, ind);
}

void Oriel::removekw(vector<string> kws, int ind) 
{
    for(auto keyword : kws)
    {
          Bid mapKey = createBid(keyword, ind);
          int del_cnt = del->find(mapKey);
          del->remove(mapKey); 
          if (del_cnt!=0)
          {
                Bid firstKey(keyword);
                int updc = I->find(firstKey);
                if(updc==0)
                	return;
                Bid lastKey = createBid(keyword,updc);
                int lastid = I->find(lastKey);
                I->remove(lastKey);
                updc--;
                if (updc > 0) 
                {
                    I->insert(firstKey,updc);
                    if (updc + 1 != del_cnt) 
                    {
                        Bid lastkey_del = createBid(keyword, lastid); 
                        del->insert(lastkey_del, del_cnt);
                        Bid delKeyI = createBid(keyword, del_cnt);
                        I->insert(delKeyI,lastid);
                    } 
                }
		else
		    I->remove(firstKey);
           }
      }
}
*/

void Oriel::getAESRandomValue(unsigned char* keyword, int op, int srcCnt, int fileCnt, unsigned char* result) {
    keyword[AES_KEY_SIZE - 5] = op & 0xFF;
    *(int*) (&keyword[AES_KEY_SIZE - 4]) = fileCnt;
    sse::crypto::Prg::derive((unsigned char*) keyword, 0, AES_KEY_SIZE, result);
}

Bid Oriel::createBid(string keyword, int number) 
{
    Bid bid(keyword);
    auto arr = to_bytes(number);
    std::copy(arr.begin(), arr.end(), bid.id.end() - 4);
    return bid;
}

void Oriel::print()
{
	I->printTree();
	//del->printTree();
	ac->printTree();
}
