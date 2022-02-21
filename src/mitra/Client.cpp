#include "Client.h"
#include <openssl/conf.h>
#include <openssl/evp.h>
#include <openssl/err.h>
#include <string.h>
#include <sse/crypto/prg.hpp>
#include<ctime>

using namespace std;
using namespace boost::algorithm;

int addascii(string k)
{
	int len = k.size();
	int a = 0;
	for(int i = 0; i< len; i++)
	{
		a = a+ int(k[i]);
	}
return a;
}


Client::Client(Server* server, bool deleteFiles, int keyworsSize, int filecnt) {
    this->server = server;
    this->deleteFiles = deleteFiles;
    //this->fakefileids = fakefileids;
    bytes<Key> key{0};
    if (!localStorage) {
        omapw = new OMAP(keyworsSize, key);
	omapf = new OMAP(filecnt, key);
    }
}

Client::Client(bool deleteFiles, int keyworsSize, int filecnt) {
    this->deleteFiles = deleteFiles;
    //this->fakefileids = fakefileids;
    bytes<Key> key{0};
    if (!localStorage) {
        omapw = new OMAP(keyworsSize, key);
        omapf = new OMAP(filecnt, key);
    }
}

Client::~Client() {
}


void Client::addfakefileid(int fileid)
{
	fakefileids.push_back(fileid);
}


void Client::updateFile(OP op, int ind, string content, bool setup) 
{
    if (!localStorage) {
        omapw->treeHandler->oram->totalRead = 0;
        omapw->treeHandler->oram->totalWrite = 0;
    }
    totalUpdateCommSize = 0;
    prf_type file; //
    memset(file.data(), 0, AES_KEY_SIZE); //
    string id = to_string(ind);
    copy(id.begin(), id.end(), file.data());
    int accsCnt = 0;;

    if (localStorage) {
        if (AccsCnt.find(file) == AccsCnt.end()) {
            AccsCnt[file] = 1;
            accsCnt = 1;
        } else {
            AccsCnt[file]++;
            accsCnt = AccsCnt[file];
        }
        if (deleteFiles) {
            if (AccsCnt.find(file) == AccsCnt.end()) {
                AccsCnt[file] = 0;
                accsCnt = 0;
            } else {
                accsCnt = AccsCnt[file];
            }
        }
    } else {
        Bid mapKey = getBid(id);
        string accsCntStr = setup ? setupOMAPf[mapKey] : omapf->incrementFileCnt(mapKey);
        if (accsCntStr != "") 
	{
            //auto parts = Utilities::splitData(fileCntStr, "-");
            //fileCnt = stoi(parts[0]);
            accsCnt = stoi(accsCntStr);
        }
        accsCnt++;
        if (setup) 
	{
            setupOMAPf[mapKey] = to_string(accsCnt);// + "-" + to_string(srcCnt);
        }
    }

    prf_type addr;
    getAESRandomValue(file.data(), 0, accsCnt, accsCnt, addr.data());
//    getAESRandomValue(file.data(), 1, accsCnt, accsCnt, rnd.data());
//    prf_type val = bitwiseXOR(content, op, rnd);
    file_type val;
     copy(content.begin(), content.end(), val.data());
    if (!localStorage) {
        totalUpdateCommSize = (sizeof (prf_type) * 2) + (omapf->treeHandler->oram->totalRead + omapf->treeHandler->oram->totalWrite)*(sizeof (prf_type) + sizeof (int));
    }
    //cout << endl <<"val is"<< endl<< content<< endl<< endl;
    server->updateFile(addr, val);
}


void Client::update(OP op, string keyword, int ind, bool setup) 
{
    cout << "inserting keyword:"<<keyword<<endl;
    if (!localStorage) 
    {
        omapw->treeHandler->oram->totalRead = 0;
        omapw->treeHandler->oram->totalWrite = 0;
    }
    totalUpdateCommSize = 0;
    prf_type k_w;
    memset(k_w.data(), 0, AES_KEY_SIZE);
    copy(keyword.begin(), keyword.end(), k_w.data());

    int k = addascii(keyword);
    srand(k);
    int r = rand()%5; // make it 10 or something LATER
    int bucket = pow(2,r);
    if(BktCnt.find(k_w) ==  BktCnt.end())
	    BktCnt[k_w] = bucket;
    else
	   bucket = BktCnt[k_w]; 
    cout <<"bucket size is:"<< bucket<<endl;  
    int fileCnt = 0, srcCnt = 0;
    if (localStorage) 
    {
        if (FileCnt.find(k_w) == FileCnt.end()) 
	{
            FileCnt[k_w] = 1;
            fileCnt = 1;
        } 
	else 
	{
            FileCnt[k_w]++;
            fileCnt = FileCnt[k_w];
        }
        if (deleteFiles) 
	{
            if (SrcCnt.find(k_w) == SrcCnt.end()) 
	    {
                SrcCnt[k_w] = 0;
                srcCnt = 0;
            } 
	    else 
                srcCnt = SrcCnt[k_w];
        }
    } 
    else 
    {
        Bid mapKey = getBid(keyword);
        string fileCntStr = setup ? 
		            setupOMAP[mapKey] : omapw->incrementFileCnt(mapKey);
        if (fileCntStr != "") 
	{
            auto parts = Utilities::splitData(fileCntStr, "-");
            fileCnt = stoi(parts[0]);
            srcCnt = stoi(parts[1]);
        }
        fileCnt++;
        if (setup) 
	{
            setupOMAP[mapKey] = to_string(fileCnt) + "-" + to_string(srcCnt);
        }
    }
/*****got our bucket count fileCnt and SrcCnt for keyword k_w*****/
prf_type t_k = k_w;


  
  if(fileCnt == 1) // first time inserting keyword // need to fill bucket
  {
    prf_type addr, rnd;
    k_w = t_k;
    getAESRandomValue(k_w.data(), 0, srcCnt, fileCnt, addr.data());
    getAESRandomValue(k_w.data(), 1, srcCnt, fileCnt, rnd.data());
    prf_type val = bitwiseXOR(ind, op, rnd);
    if (!localStorage) 
    {
        totalUpdateCommSize = (sizeof (prf_type) * 2) + (omapw->treeHandler->oram->totalRead + omapw->treeHandler->oram->totalWrite)*(sizeof (prf_type) + sizeof (int));
    }
    server->update(addr, val, "real");
    cout <<"apple inserted!"<<endl;
     vector<int> temp = fakefileids;
     for(int i = 2; i<= bucket; i++)
     {
         prf_type addr, rnd;
	 k_w = t_k;
         getAESRandomValue(k_w.data(), 0, srcCnt, i, addr.data());
         getAESRandomValue(k_w.data(), 1, srcCnt, i, rnd.data());
         int sz = temp.size();
         int r = rand()% sz;
         int id = temp[r];
	 temp.erase(temp.begin()+r);
         prf_type val = bitwiseXOR(id, OP::INS, rnd);
         if (!localStorage) 
	 {
             totalUpdateCommSize = (sizeof (prf_type) * 2) + (omapw->treeHandler->oram->totalRead + omapw->treeHandler->oram->totalWrite)*(sizeof (prf_type) + sizeof (int));
         }
         server->update(addr, val, "fake");
      }
  }
  else if(fileCnt > 1) // we have a bucket for keyword
  {
      vector<prf_type> KList;
 //get the bucket files and convert one fake<-->real
      k_w = t_k;
      for (int i = 1; i <= bucket; i++) 
      {
	   k_w = t_k;
           prf_type rnd;
           getAESRandomValue(k_w.data(), 0, srcCnt, i, rnd.data());
           KList.emplace_back(rnd);
      }
    pair<vector<prf_type>,vector<string>> encIndexespair= server->search(KList);
    vector<prf_type> ids = encIndexespair.first;//
    vector<string> fk = encIndexespair.second;
    map<int, int> remove;
    map<int,string> idf;
    int cnt = 1;
    cout <<"SIZE :"<< ids.size()<<endl;
	k_w = t_k;
	vector<string>::iterator ff = fk.begin();
    for (auto i = ids.begin(); i != ids.end(); i++) 
    {
        prf_type tmp;
        getAESRandomValue(k_w.data(), 1, srcCnt, cnt, tmp.data());
        prf_type decodedString = *i;
        prf_type plaintextBytes = bitwiseXOR(tmp, decodedString);
        int plaintext = (*((int*) &plaintextBytes[0]));
        remove[plaintext] += (2 * plaintextBytes[4] - 1);
	idf[plaintext] =  *ff;
	ff++;
	cout <<"plaintext:"<< plaintext<<endl;
        cnt++;
    }
     if(fileCnt <= bucket)
     { 
	     int inc = 0;
             fileCnt = 1;
	     for( auto f : idf)
	     { 
		     if(f.second == "fake" && inc == 0)
		     {   inc++;
	                 cout << "**fake->real**:"<<f.first<<"->"<<ind<< endl;
                	 prf_type addr, rnd;
                getAESRandomValue(k_w.data(),0,srcCnt,fileCnt,addr.data());
                getAESRandomValue(k_w.data(),1,srcCnt,fileCnt,rnd.data());
                	 prf_type val = bitwiseXOR(ind, op, rnd);
                	 if (!localStorage) 
			 {
                	     totalUpdateCommSize = (sizeof (prf_type) * 2) + (omapw->treeHandler->oram->totalRead + omapw->treeHandler->oram->totalWrite)*(sizeof (prf_type) + sizeof (int));
                	 }
                	 server->update(addr, val, "real");
		     }
		     else
		     {
                	prf_type addr, rnd;
	                 cout << "**same**:"<<f.first<< endl;
                getAESRandomValue(k_w.data(),0,srcCnt,fileCnt,addr.data());
                getAESRandomValue(k_w.data(),1,srcCnt,fileCnt,rnd.data());
                	prf_type val = bitwiseXOR(f.first, op, rnd);
                	 if (!localStorage) 
			 {
                	     totalUpdateCommSize = (sizeof (prf_type) * 2) + (omapw->treeHandler->oram->totalRead + omapw->treeHandler->oram->totalWrite)*(sizeof (prf_type) + sizeof (int));
                	 }
                	 server->update(addr, val, f.second);
		     }
		     ff++;
		     fileCnt++;
	     }
     }
     else if(fileCnt > bucket)
     {
	     k_w = t_k;
           cout <<" Bucket size for== "<< keyword << ":"<< BktCnt[k_w]<< endl; 
	 //double the bucket size and add the new file 
	     vector<string>::iterator ff = fk.begin();
	     fileCnt=1;
	     for(auto rm : remove)
	     {
                	 prf_type addr, rnd;
                getAESRandomValue(k_w.data(),0,srcCnt,fileCnt,addr.data());
                getAESRandomValue(k_w.data(),1,srcCnt,fileCnt,rnd.data());
                	 prf_type val = bitwiseXOR(rm.first, op, rnd);
                	 if (!localStorage) 
			 {
                	     totalUpdateCommSize = (sizeof (prf_type) * 2) + (omapw->treeHandler->oram->totalRead + omapw->treeHandler->oram->totalWrite)*(sizeof (prf_type) + sizeof (int));
                	 server->update(addr, val, *ff); // all real btw
		     }
		 fileCnt++;
		 ff++;
	     }
	 //add new file at bucket+1 th position or randomely choose one
     	  vector<int> temp = fakefileids;
	  int choose = rand()%bucket;
	  choose = choose +bucket+1;
   	  BktCnt[t_k] = bucket*2;
	  k_w = t_k;
           cout <<" New Bucket size "<< keyword << ":"<< BktCnt[k_w]<< endl; 
   	  for (int i = bucket+1; i<=bucket*2;i++)
   	  {
           	 prf_type addr, rnd;
           	 getAESRandomValue(k_w.data(), 0, srcCnt, i, addr.data());
           	 getAESRandomValue(k_w.data(), 1, srcCnt, i, rnd.data());
           	 int sz = temp.size();
           	 int r = rand()% sz;
           	 int id = temp[r];
		 temp.erase(temp.begin()+r);
		 prf_type val;
		 string isfake;
		 if(choose == i)
		 {
			 cout <<"adding real:"<<ind<<endl;
			 val = bitwiseXOR(ind, op, rnd);
			 isfake = "real";
		 }
		 else 
		 {
			 cout <<"adding fake:"<<id<<endl;
           	         val = bitwiseXOR(id, op, rnd);
			 isfake = "fake";
		 }
           	 if (!localStorage) 
		 {
           	     totalUpdateCommSize = (sizeof (prf_type) * 2) + (omapw->treeHandler->oram->totalRead + omapw->treeHandler->oram->totalWrite)*(sizeof (prf_type) + sizeof (int));
            	 }
            server->update(addr, val, isfake); //might change
	  }
  }	  
 }
}


vector<string> Client::searchfile(vector<int> inds) 
{
    vector<string> files;
    vector<prf_type> KList;
    for(auto ind :inds)
    {
        string id = to_string(ind);    
        if (!localStorage) 
	{
            omapf->treeHandler->oram->totalRead = 0;
            omapf->treeHandler->oram->totalWrite = 0;
        }
        totalSearchCommSize = 0;
        string finalRes = "";
        prf_type file;
        memset(file.data(), 0, AES_KEY_SIZE);
        copy(id.begin(), id.end(), file.data());
        Bid mapKey = getBid(id);
        int accsCnt = 0;
        if (localStorage) 
	{
            if (AccsCnt.find(file) == AccsCnt.end()) 
	    {
                continue;
            }
        } 
	else 
	{
            string accsCntStr = omapf->find(mapKey);
            if (accsCntStr != "") 
	    {
                accsCnt = stoi(accsCntStr);
            } 
	    else 
	    {
                continue;
            }
        }
        prf_type rnd;
        getAESRandomValue(file.data(), 0, accsCnt, accsCnt, rnd.data());
        KList.emplace_back(rnd);
    }
    vector<file_type> encIndexes = server->searchFile(KList);
    map<int, string> remove;
    int cnt = 1;
    cout <<"SIZE :"<< encIndexes.size()<<endl;
    for (auto i = encIndexes.begin(); i != encIndexes.end(); i++) 
    {
        file_type decodedString = *i;
        remove[cnt].assign(decodedString.begin(), decodedString.end());
	//cout << cnt<<":[["<<remove[cnt]<<"]]"<<endl<<endl;
        cnt++;
    }
    for(auto ind :inds)
    {
        int accsCnt = 0;
        string id = to_string(ind);    
        prf_type file;
        memset(file.data(), 0, AES_KEY_SIZE);
    	if (deleteFiles) {
        if (localStorage) {
            accsCnt = AccsCnt[file]++;
        }
    }
    for (auto const& cur : remove) {
        if (cur.second != "") {
            files.emplace_back(cur.second); ////
            if (deleteFiles) {
                accsCnt++;
                prf_type addr;
                getAESRandomValue(file.data(),0, accsCnt,accsCnt, addr.data());
                file_type val;//;
                copy(cur.second.begin(), cur.second.end(), val.data());
                server->updateFile(addr, val);
            }
        }
    }
    if (deleteFiles) {
        totalSearchCommSize += (accsCnt * 2 * sizeof (prf_type));
        if (localStorage) {
            AccsCnt[file] = accsCnt;
        } else {
    	    Bid mapKey = getBid(id);
            omapf->insert(mapKey, to_string(accsCnt));
        }
    }
    }
    if (!localStorage) {
        totalSearchCommSize += sizeof (prf_type) * KList.size() + encIndexes.size() * sizeof (prf_type) + (omapf->treeHandler->oram->totalRead + omapf->treeHandler->oram->totalWrite)*(sizeof (prf_type) + sizeof (int));
    }
return files;
}

pair<vector<int>,vector<string>> Client::search(string keyword) {
    if (!localStorage) {
        omapw->treeHandler->oram->totalRead = 0;
        omapw->treeHandler->oram->totalWrite = 0;
    }
    totalSearchCommSize = 0;
    vector<int> finalRes;
    vector<string> fk;
    vector<prf_type> KList;
    prf_type k_w;
    memset(k_w.data(), 0, AES_KEY_SIZE);
    copy(keyword.begin(), keyword.end(), k_w.data());
    int bucket = BktCnt[k_w];
    Bid mapKey = getBid(keyword);
    int fileCnt = 0, srcCnt = 0;
    if (localStorage) {
        if (FileCnt.find(k_w) == FileCnt.end()) {
            return make_pair(finalRes,fk);
        }
        fileCnt = FileCnt[k_w];
        if (deleteFiles) {
            srcCnt = SrcCnt[k_w];
        }
    } else {

        string fileCntStr = omapw->find(mapKey);
        if (fileCntStr != "") {
            auto parts = Utilities::splitData(fileCntStr, "-");
            fileCnt = stoi(parts[0]);
            srcCnt = stoi(parts[1]);
        } else {
            return make_pair(finalRes,fk);
        }
    }

    //for (int i = 1; i <= fileCnt; i++) {
    for (int i = 1; i <= bucket; i++) {
        prf_type rnd;
        getAESRandomValue(k_w.data(), 0, srcCnt, i, rnd.data());
        KList.emplace_back(rnd);
    }

    pair<vector<prf_type>,vector<string>> encIndexespair= server->search(KList);
    vector<prf_type> encIndexes = encIndexespair.first;
    vector<string> fakes = encIndexespair.second;
    map<int, int> remove;
    map<int,string> isf;
    auto ff = fakes.begin();
    int cnt = 1;
    cout <<"SIZE of search:"<< encIndexes.size()<<endl;
    for (auto i = encIndexes.begin(); i != encIndexes.end(); i++) {
        prf_type tmp;
        getAESRandomValue(k_w.data(), 1, srcCnt, cnt, tmp.data());
        prf_type decodedString = *i;
        prf_type plaintextBytes = bitwiseXOR(tmp, decodedString);
        int plaintext = (*((int*) &plaintextBytes[0]));
        remove[plaintext] += (2 * plaintextBytes[4] - 1);
	isf[plaintext] = *ff;
	ff++;
	cout<<"remove:"<< plaintext<<endl;
        cnt++;
    }
    if (deleteFiles) {
        if (localStorage) {
            SrcCnt[k_w]++;
        }
        fileCnt = 0;
        srcCnt++;
    }
    int f = 0;
    fileCnt = 1; //??
    for (auto const& cur : remove) {
        if (cur.second < 0) {
            finalRes.emplace_back(cur.first);
            //if (deleteFiles) {
                fileCnt++;
                prf_type addr, rnd;
                getAESRandomValue(k_w.data(), 0, srcCnt, fileCnt, addr.data());
                getAESRandomValue(k_w.data(), 1, srcCnt, fileCnt, rnd.data());
                prf_type val = bitwiseXOR(cur.first, OP::INS, rnd);
                server->update(addr, val ,isf[cur.first]); 
            //}
        }
    }
    if (deleteFiles) {
        totalSearchCommSize += (fileCnt * 2 * sizeof (prf_type));
        if (localStorage) {
            FileCnt[k_w] = fileCnt;
        } else {
            omapw->insert(mapKey, to_string(fileCnt) + "-" + to_string(srcCnt));
        }
    }
    if (!localStorage) {
        totalSearchCommSize += sizeof (prf_type) * KList.size() + encIndexes.size() * sizeof (prf_type) + (omapw->treeHandler->oram->totalRead + omapw->treeHandler->oram->totalWrite)*(sizeof (prf_type) + sizeof (int));
    }
    return make_pair(finalRes,fakes);
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

void Client::endSetup() {
    if (!localStorage) {
        omapw->setupInsert(setupOMAP);
        cout << "setup size:" << setupOMAP.size() << endl;

        for (int i = 0; i < 500; i++) {
            cout << i << "/500" << endl;
            Bid testBid;
            testBid.setValue(1);
            omapw->insert(testBid, "TEST");
        }
        //omap->printTree();
    }
}
