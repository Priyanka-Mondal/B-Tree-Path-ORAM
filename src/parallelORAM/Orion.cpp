#include "Orion.h"
#include <boost/algorithm/string.hpp>
#include "stopword.hpp"
#include<thread>
#include <cstdlib>
#include <iostream>
#include <string>
#include <vector>
#include <ctime>
#include <numeric>
#include <cmath>
#include <sstream>
#include <chrono>
#include <ctime>
#include <mutex>
#include<future>

Orion::Orion(int filesize,bool local):rd(),mt(rd()),dis(0,(pow(2,floor(log2(filesize/Z))+1)-1)/2) 
{
    this->local= local;
    bytes<Key> key1{0};
    fileoram = new FileORAM(filesize, key1);
}

Orion::~Orion() 
{
    delete fileoram;
}

int fileblks = 0;

vector<string> divideString(string str, int sz)
{
        int str_size = str.length();
	if (str_size% sz !=0)
	{
		int pad = ceil(str_size/sz)+1;
		pad = pad*sz-str_size;
	        str.insert(str.size(), pad, '#');
	}
	str_size = str.length(); // new length
  	int i;
        vector<string> result;
        string temp="";
	/*int cover = 0;
	while(cover < str_size)
	{
		temp.insert(0,str,cover,BLOCK);
		cover=cover+BLOCK;
		result.push_back(temp);
	        temp="";	
	}*/
	for (i = 0; i < str_size; i++) 
	{
            if (i % sz == 0) 
	    {
		  if(i!=0)
                  {
		     string ttemp =""; //id
		     ttemp.append(temp); 
                     result.push_back(ttemp);    
                  }
                  temp="";
            }
	    temp +=str[i];
	}
        string ttemp = "";//id;
	ttemp.append(temp);	
	result.push_back(ttemp);
	return result;
}

int Orion::stoI(string updt_cnt)
{
	int updc;
	stringstream convstoi(updt_cnt);
	convstoi >> updc;
	return updc;
}
void Orion::insertWrap(string cont, int fileid, bool batch)
{
      vector<string> blocks = divideString(cont,BLOCK);
      batchInsert(blocks,fileid);
}

Fnode* Orion::newNode(Fbid key, string value, int pos) 
{
    Fnode* node = new Fnode();
    node->key = key;
    std::fill(node->value.begin(), node->value.end(), 0);
    std::copy(value.begin(), value.end(), node->value.begin());
    node->pos = pos;
    return node;
}

void Orion::batchInsert(vector<string> blocks, int ind) 
{
    string id = to_string(ind);
    int block_num = 1;
    localBCNT[ind]=blocks.size();
    fileoram->start(false);
    for(auto blk: blocks)
    {
	   Fbid fbid = createFbid(id,block_num);
	   int pos = RandomPath();
	   fileoram->localPos[fbid] = pos;
	   Fnode *fnode = newNode(fbid,blk,pos);
	   fileoram->WriteFnode(fbid,fnode);
	   block_num++;
    }
    fileoram->finalize();
    fileblks = fileblks+block_num-1;
    cout<<"BATCH inserted blocks of fileid: "<<ind<<" fb:"<<fileblks<< endl;
}
int Orion::RandomPath() {
    int val = dis(mt);
    return val;
}
vector<string> Orion::asyncSearch(vector<int> result) 
{
	vector<string> conts;
	fileoram->start(true);
	int block_num = 0;
   	for(auto id:result)
   	{
   		string fID = to_string(id);
		block_num = block_num+localBCNT[id];
	//creat block_num treads
	}
	int t = 0;
	vector<future<Fnode*>> asynctasks;
	asynctasks.reserve(block_num);
	for(auto id: result)
	{
		for(int i = 1; i<=localBCNT[id];i++)
		{
   			string fID = to_string(id);
			Fbid fbid = createFbid(fID,i);
			int pos = fileoram->localPos[fbid];
			//std::launch::async|std::launch::deferred,
asynctasks.push_back(async(std::launch::async|std::launch::deferred, &FileORAM::ReadFnode,fileoram, fbid, pos,pos));
		t++;
		}
	}
	for(auto &a:asynctasks)
	{
		Fnode* fnode = a.get();
		string temp ="";
        	temp.assign(fnode->value.begin(), fnode->value.end());
        	//temp = temp.c_str();
		//cout <<fnode->pos<<" "<<temp;
		conts.push_back(temp);
	}
	
	fileoram->finalize();
	return conts;
}

vector<string> Orion::syncSearch(vector<int> result) 
{
	vector<string> conts;
	fileoram->start(true);
	int block_num = 0;
   	for(auto id:result)
   	{
   		string fID = to_string(id);
		block_num = block_num+localBCNT[id];
	//creat block_num treads
	}
	int t = 0;
	for(auto id: result)
	{
		for(int i = 1; i<=localBCNT[id];i++)
		{
   			string fID = to_string(id);
			Fbid fbid = createFbid(fID,i);
			int pos = fileoram->localPos[fbid];
			Fnode *fnode = fileoram->ReadFnode(fbid,pos,pos);
			string temp="";
        		temp.assign(fnode->value.begin(), fnode->value.end());
        		temp = temp.c_str();
			conts.push_back(temp);
		}
	}
	fileoram->finalize();
	return conts;
}

/*
void Orion::insert(vector<string> kws, vector<string> blocks, int ind) 
{
    for(auto kw: kws)
    {		
  	      Bid firstKey(kw);
  	      int fc = srch->find(firstKey);
  	      fc++;
  	      Bid mapKey = createBid(kw, ind);
  	      //updt->insert(mapKey, fc);
	      //UpdtCnt[mapKey]=fc;
  	      srch->insert(firstKey, fc);
  	      Bid key = createBid(kw, fc);
  	      srch->insert(key, ind);
    }
      string id = to_string(ind);
      Bid blkcnt(id);
      srch->insert(blkcnt,blocks.size());
      int block_num = 1;
      for(auto blk: blocks)
      {
	      Bid fb = createBid(id,block_num);
	      file->insert(fb,blk);
	      block_num++;
      }
      inserted = inserted+kws.size();
      fileblks = fileblks+block_num;
      cout << "INSERTED keywords and blocks(kw:"<<kws.size() <<",b:"<<blocks.size()<<") of:"<<ind<<" uk:"<<uniquekw<<" fb:"<<fileblks<< endl;
}

vector<pair<int,string>> Orion::search(string keyword) 
{
    vector<pair<int,string>> fileblocks;
    Bid firstKey(keyword);
    int fc = srch->find(firstKey);
    if (fc == 0) 
	return fileblocks;
    for (int i = 1; i <= fc; i++) 
    {
	Bid bid = createBid(keyword, i);
        int id = srch->find(bid);
	string fileid = to_string(id);
	Bid blkcnt(fileid);
        int blocknum = srch->find(blkcnt);
	for (int j= 1;j<=blocknum;j++)
	{
		Bid block = createBid(fileid,j);
		string cont = file->find(block);
		fileblocks.push_back(make_pair(id,cont));
	}
    }
    return fileblocks;
}

vector<string> Orion::simplebatchSearch(string keyword) 
{
	ofstream bcfc;
	bcfc.open("bcfc.txt",ios::app);
	vector<string> conts;
	int fc = 0;
        if(localFCNT.count(keyword)>0)
        	fc = localFCNT[keyword];
	else 
	    return conts;
        bcfc<< fc <<" ";
    	vector<Bid> bids;
    	bids.reserve(fc);
   	for (int i = 1; i <= fc; i++) 
   	{
   	         Bid bid = createBid(keyword, i);
   	         bids.push_back(bid);
   	}
   	vector<int> result;
   	result.reserve(fc); 
   	result = srch->batchSearch(bids);
   	bids.clear();
	int tot = 0;
   	for(auto id:result)
   	 {
	 	int blocknum = localBCNT[id];
		tot = tot + blocknum;
   	        string fID = to_string(id);
   	        for (int j= 1;j<=blocknum;j++)
   	        {
   	     		Bid block = createBid(fID,j);
   	     		bids.push_back(block);
   	     	}
   	  }
	bcfc << tot <<endl;
   	  //conts = file->batchSearch(bids);
   	  //cout <<bids.size()<<"/"<< conts.size()<<endl;
    return conts;
}
vector<string> Orion::batchSearch(string keyword) 
{
    vector<string> conts;
    Bid firstKey(keyword);
    int fc = fcnt->find(firstKey);
    if (fc == 0) 
	    return conts;
    vector<Bid> bids;
    bids.reserve(fc);
    for (int i = 1; i <= fc; i++) 
    {
            Bid bid = createBid(keyword, i);
            bids.push_back(bid);
    }
    vector<int> result;
    result.reserve(fc); 
    result = srch->batchSearch(bids);
    bids.clear();
    map<string,int> bcnt;
    for(int id:result)
    {
        string fileid = to_string(id);
	Bid blkcnt(fileid);
	bcnt[fileid] = fcnt->find(blkcnt);
    }
    bids.clear();
    for(auto it : bcnt)
    {
	string fID = it.first;
	for (int j= 1;j<=it.second;j++)
	{
		Bid block = createBid(fID,j);
		bids.push_back(block);
	}
     }
     conts = file->batchSearch(bids);
     cout <<bids.size()<<"/"<< conts.size()<<endl;
    return conts;
}

void Orion::remove(int id) 
{
	string ind = to_string(id);
	string cont = "";
	int blk = 1;
	Bid delKey(ind);
	int blkcnt = srch->find(delKey);
	if(blkcnt == 0)
	{
		cout <<" File does NOT EXIST!"<<endl;
		return;
	}
	srch->remove(delKey);
	for(int bc = 1; bc <= blkcnt; bc++)
	{
		delKey = createBid(ind, bc);
		string ret = file->find(delKey);
		cont = cont.append(ret);
		file->remove(delKey);	
	}
	cout <<"Removed "<<blk<<" blocks from srch:"<<ind<<endl;
        	
	vector<string> kws1;
string delimiters("|+#(){}[]0123456789*?&@=,:!\"><; _-./  \n");
	boost::split(kws1, cont, boost::is_any_of(delimiters));
	vector<string> kws = getUniquedWords(kws1);
	//removekw(kws, id);
}

*/
/*
void Orion::removekw(vector <string> kws, int id) 
{
	for(auto kw : kws)
	{
    		Bid delKey = createBid(kw, id);
		cout <<"here---"<<endl;
    		int delcnt = UpdtCnt[delKey];//updt->find(delKey);
		cout <<"here---2"<<endl;
		if(delcnt != 0)
		{
			Bid fcntKey(kw);
			int fc = srch->find(fcntKey);
			int newfilecnt = fc-1;
			Bid lastKey = createBid(kw,fc);
			int lastid = srch->find(lastKey);
			if(delcnt != fc)
			{
				Bid delkwKey = createBid(kw,delcnt);
				srch->insert(delkwKey, lastid);
				Bid lastupdKey = createBid(kw,lastid);
				//updt->insert(lastupdKey,delcnt);
	      			UpdtCnt[lastupdKey]=delcnt;
			}
			//updt->remove(delKey);
	      		UpdtCnt[delKey]=0;
			srch->remove(lastKey);
			if(newfilecnt == 0)
				srch->remove(fcntKey);
			else
				srch->insert(fcntKey,newfilecnt);
		}
		else
		{
			cout <<"FILE DOES NOT EXIST"<<endl;
			return;
		}
	}
}
*/
Fbid Orion::createFbid(string id, int number) {
    Fbid bid(id);
    auto arr = to_bytes(number);
    std::copy(arr.begin(), arr.end(), bid.id.end() - 4);
    return bid;
}
