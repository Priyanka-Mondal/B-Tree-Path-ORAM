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

Oriel::Oriel(bool usehdd, int kwSize,int filesize, bool local) 
{
    this->useHDD = usehdd;
    this->local = local;
    bytes<Key> key1{0};
    bytes<Key> key2{1};
    I = new OMAPf(kwSize*100, key1);
    del = new OMAPf(kwSize*100,key2);
    fcnt = new OMAPf(kwSize, key1);
    ac = new OMAPf(filesize, key2); 
    clen_size = AES::GetCiphertextLength(FILEBLOCK);
}

string toS(int id)
{
	string s = to_string(id);
	string front ="";
	if (id < 10)
		front = "00000";
	else if(id < 100)
		front = "0000";
	else if(id < 1000)
		front = "000";
	else if(id < 10000)
		front = "00";
	else if(id < 100000)
		front = "0";
	s=front.append(s);
	return s;
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

int get_block_num(int updc, int com)
{
       float bnum = ceil(updc/com); // com file-ids each block
       int block_num ;
       if(updc%com == 0)
	      block_num = bnum;
       else 
	      block_num = bnum+1;

	return block_num;
}

int get_position(int updc, int com)
{
       int pos_in_block = updc%com; // 1,2,3,4, ..., 16
       if (pos_in_block == 0) 
	       pos_in_block = com;

       return pos_in_block;
}
Oriel::~Oriel() 
{
    delete I;
    delete fcnt;
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
}

void Oriel::setupInsertkw(string keyword, int ind) 
{
    Bid mapKey(keyword);
    int updc =0;
    if(fcntmap.count(mapKey)>0)
    	updc = stoI(fcntmap[mapKey]);
    updc++;
    fcntmap[mapKey]=to_string(updc);
    Bid updKey = createBid(keyword, ind);
    updtmap[updKey]= to_string(updc);//pad

    int pos_in_block = get_position(updc,COM);
    int block_num = get_block_num(updc,COM);
    Bid key = createBid(keyword, block_num);
    if(pos_in_block == 1)
    {
           string id = toS(ind);
           id.insert(FID_SIZE, BLOCK-FID_SIZE, '#'); // padding happpens
	   Imap[key]=id;
    }
    else if (pos_in_block >=2 && pos_in_block <=COM)
    {
          string oldblock = Imap[key];//(srch->setupfind(key)).second;
          oldblock.replace((pos_in_block-1)*FID_SIZE,FID_SIZE,toS(ind));
	  Imap[key]=oldblock;
    }
}
    
void Oriel::insertFile(int ind, string content,bool batch)
{
    prf_type file; 
    memset(file.data(), 0, AES_KEY_SIZE); 
    string id = to_string(ind);
    copy(id.begin(), id.end(), file.data());
    Bid mapKey(id);
    if(batch && !local)
	acmap[mapKey]=to_string(1);
    else if(!local)
    	ac->insert(mapKey,to_string(1));
    else 
	localAC[ind]=1;
    prf_type addr;
    getAESRandomValue(file.data(), 0, 1, 1, addr.data());
    int sz = content.size();
    if(!(sz%FILEBLOCK == 0))
    {
    	sz = (sz / FILEBLOCK + 1) * FILEBLOCK;
    }
    FileNode *head = NULL;
    int len = 0;
    while(len < sz)
    {
	    string part = content.substr(len,FILEBLOCK);
	    fblock plaintext;
	    plaintext.insert(plaintext.end(),part.begin(), part.end());
    	    block ciphertext = AES::Encrypt(key, plaintext, clen_size, FILEBLOCK);
	    append(&head,ciphertext);
	    len = len + FILEBLOCK;
    }
    DictF[addr]=head;
}

void Oriel::endSetup()
{
	I->setupInsert(Imap);
	fcnt->setupInsert(fcntmap);
	del->setupInsert(updtmap);
	if(!local)
	   ac->setupInsert(acmap);
}

vector<string> Oriel::localbatchSearch(string keyword)
{
    vector<string> files;
    int updc;
    Bid mapKey(keyword);
    
    string updcst = (fcnt->find(mapKey)); 
    if(updcst == "")
	    updc = 0;
    else 
	    updc = stoI(updcst);
    if (updc == 0) 
        return files;
    int blocks= get_block_num(updc, COM);
    vector<Bid> bids;
    bids.reserve(blocks);
    for(int i=1;i<=blocks;i++)
    {
	Bid b = createBid(keyword,i);
	bids.push_back(b);
    }
    vector<string> fids;
    fids.reserve(blocks);
    fids = I->batchSearch(bids);
    int fetched = 0;
    vector<int> filind;
    filind.reserve(updc);
    int point;
    for(auto ids:fids)
    {
	    point = 0;
	    while(fetched < updc && point < COM)
	    {
	    	string str = ids.substr(point*FID_SIZE,FID_SIZE);
	    	point++;
		if(str == "######")
			continue;
	    	fetched++;
		int ii = stoI(str);
		filind.push_back(ii);
	     }
    }
    for(int ind : filind)
    {
	int accsCnt = (localAC[ind]);
        prf_type file;
        memset(file.data(), 0, AES_KEY_SIZE);
	string id = to_string(ind);
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
	localAC[ind]=(++accsCnt);
	DictF.erase(addr);
        getAESRandomValue(file.data(), 0, accsCnt, accsCnt, addr.data());
        DictF[addr]=newhead;
    }
    return files;
}

vector<string> Oriel::batchSearch(string keyword)
{
    vector<string> files;
    int updc;
    Bid mapKey(keyword);
    
    string updcst = (fcnt->find(mapKey)); 
    if(updcst == "")
	    updc = 0;
    else 
	    updc = stoI(updcst);
    if (updc == 0) 
        return files;
    int blocks= get_block_num(updc, COM);
    vector<Bid> bids;
    bids.reserve(blocks);
    for(int i=1;i<=blocks;i++)
    {
	Bid b = createBid(keyword,i);
	bids.push_back(b);
    }
    vector<string> fids;
    fids.reserve(blocks);
    fids = I->batchSearch(bids);
    int fetched = 0;
    vector<int> filind;
    filind.reserve(updc);
    int point;
    for(auto ids:fids)
    {
	    point = 0;
	    while(fetched < updc && point < COM)
	    {
	    	string str = ids.substr(point*FID_SIZE,FID_SIZE);
	    	point++;
		if(str == "######")
			continue;
	    	fetched++;
		int ii = stoI(str);
		filind.push_back(ii);
	     }
    }
    for(int ind : filind)
    {
	Bid acKey(to_string(ind));
	int accsCnt = stoI(ac->find(acKey));
        prf_type file;
        memset(file.data(), 0, AES_KEY_SIZE);
	string id = to_string(ind);
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
	accsCnt = accsCnt+1;
	ac->insert(acKey,to_string(accsCnt));
	DictF.erase(addr);
        getAESRandomValue(file.data(), 0, accsCnt, accsCnt, addr.data());
        DictF[addr]=newhead;
    }
    return files;
}

void Oriel::insertkw(string keyword, int ind) 
{
    Bid mapKey(keyword);
    int updc = stoI(I->find(mapKey));
    if (updc == 0) 
	 uniquekw++;  
    updc++;
    fcnt->insert(mapKey,to_string(updc));
    Bid updKey = createBid(keyword, ind);
    del->insert(updKey, to_string(updc));
    Bid key = createBid(keyword, updc);
    I->insert(key, to_string(ind));
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
