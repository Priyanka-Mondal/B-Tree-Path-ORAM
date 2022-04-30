#include "Orion.h"
#include <boost/algorithm/string.hpp>
#include "stopword.hpp"
#include<unordered_map>

Orion::Orion(bool usehdd, int size, bool local) 
{
    this->useHDD = false;//usehdd;
    this->local= local;
    bytes<Key> key1{0};
    bytes<Key> key2{1};
    btreeHandler = new BTree(size, key1);
}

Orion::~Orion() 
{
    delete btreeHandler;
}


int inserted = 0; 
int uniquekw = 0;
int fileblks = 0;
string delimiters("|+#(){}[]0123456789*?&@=,:!\"><; _-./  \n");
//string delimiters(" \n");

int stoI(string updt_cnt)
{
	int updc;
	stringstream convstoi(updt_cnt);
	convstoi >> updc;
	return updc;
}


vector<string> getUniquedWords(vector<string> kws)
{
    vector<string> kw;
    unordered_map<string, int> mp;
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
    for (unordered_map<string, int> :: iterator p = mp.begin();
         p != mp.end(); p++)
    {
        if (p->second >= 1)
            kw.push_back(p->first) ;
    }
    mp.clear();
return kw;
}

vector<string> divideString(string str, int sz)
{
        int str_size = str.length();
	if (str_size% sz !=0)
	{
		int pad = ceil(str_size/sz)+1;
		pad = pad*sz-str_size;
	        str.insert(str.size(), pad, '1');
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

void Orion::insertWrap(string cont, int fileid, bool batch)
{
      vector<string> kws1, kws;
      boost::split(kws1, cont, boost::is_any_of(delimiters));
      kws =  getUniquedWords(kws1);
      for (auto it = kws.begin(); it != kws.end(); it++)
      {
	      if(stopwords.find(*it)!=stopwords.end())
		 kws.erase(it--);
      }
      vector<string> blocks;
      blocks = divideString(cont,BLOCK);
      insert(kws, blocks, fileid);
}

void Orion::insert(vector<string> kws, vector<string> blocks, int ind) 
{
    for(auto kw: kws)
    {		
	    /*
  	      Bid firstKey(kw);
  	      int fc = fcntbids[firstKey];
  	      fc++;
  	      Bid mapKey = createBid(kw, ind);
  	      fcntbids[firstKey]=fc;
	      Bid key = createBid(kw, fc);
  	      srch->insert(key, ind);
	      */
	    int fc = 0;
	    if(fcntbtree.count(kw)!=0)
  	      fc = fcntbtree[kw];
	    fc++;
	    fcntbtree[kw] = fc;
	    Bid key = createBid(kw,fc);
	    btreeHandler->insert(key,ind);
    }
}

Bid Orion::createBid(string keyword, int number) 
{
    Bid bid(keyword);
    auto arr = to_bytes(number);
    std::copy(arr.begin(), arr.end(), bid.id.end() - 4);
    return bid;
}

vector<int> Orion::search(string keyword) 
{
    /*
    Bid firstKey(keyword);
    int fc = fcntbids[firstKey];
    if (fc == 0) 
    {
    	btreeHandler->search(keyword);
	return fileblocks;
    }
    for (int i = 1; i <= fc; i++) 
    {
	Bid bid = createBid(keyword, i);
        int id = srch->find(bid);
	
	string fileid = to_string(id);
	Bid blkcnt(fileid);
        int blocknum = localBCNT[id];//srch->find(blkcnt);
	for (int j= 1;j<=blocknum;j++)
	{
		Bid block = createBid(fileid,j);
		string cont = file->find(block);
		fileblocks.push_back(make_pair(id,cont));
	}
    }*/
    int fc = 0; 
    vector<int> files;
    if(fcntbtree.count(keyword)>0)
	    fc = fcntbtree[keyword];
    if(fc == 0)
	    cout <<"NOT FOUND"<<endl;
    else
    {
	    for(int i = 1; i<=fc; i++)
	    { 
    		Bid key = createBid(keyword,i);
    		int res = btreeHandler->search(key);
		if(res!=0)
		{
		   files.push_back(res);
		  // cout <<"["<<res<<"]";
		}
	    }
    }
    return files;
}
void Orion::remove(string kw)
{
	int fc = fcntbtree[kw];
	//cout <<"FC:"<<fc<<endl;
	for(int i = 1 ; i<= fc ; i++)
	{
		Bid key = createBid(kw,i);
		cout <<"removing:"<<kw<<" FC:["<<i<<"]"<<endl;
		btreeHandler->remove(key);
	}
}
/*
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
		cout <<"["<<id<<"]";
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
   	conts = file->batchSearch(bids);
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
	int blkcnt = localBCNT[id];//srch->find(delKey);
	if(blkcnt == 0)
	{
		cout <<" File does NOT EXIST!"<<endl;
		return;
	}
	//srch->remove(delKey);
	localBCNT[id]=0;
	for(int bc = 1; bc <= blkcnt; bc++)
	{
		delKey = createBid(ind, bc);
		string ret = file->find(delKey);
		cont = cont.append(ret);
		file->remove(delKey);	
	}
	vector<string> kws1;
	boost::split(kws1, cont, boost::is_any_of(delimiters));
	vector<string> kws = getUniquedWords(kws1);
	removekw(kws, id);
	
}


void Orion::removekw(vector <string> kws, int id) 
{
	for(auto kw : kws)
	{
		cout <<"removing ["<<kw<<"]"<<endl;
    		Bid delKey = createBid(kw, id);
    		int delcnt = UpdtCnt[delKey];//updt->find(delKey);
		if(delcnt != 0)
		{
			Bid fcntKey(kw);
			//int fc = srch->find(fcntKey);
			int fc = fcntbids[fcntKey];
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
				//srch->remove(fcntKey);
				fcntbids[fcntKey]=newfilecnt;
			else
				//srch->insert(fcntKey,newfilecnt);
				fcntbids[fcntKey]=newfilecnt;
		}
		else
		{
			cout <<"FILE DOES NOT EXIST"<<endl;
			return;
		}
	}
}

void Orion::print()
{
	srch->printTree();
}
void Orion::endSetup() 
{
        srch->setupInsert(srchbids);
}
void Orion::setupinsert(vector<string> kws, vector<string> blocks, int ind) 
{
    for(auto kw: kws)
    {	
  	      Bid firstKey(kw);
	      int fc = fcnt->setupfind(firstKey);
  	      fc++;
  	      Bid mapKey = createBid(kw, ind);
  	      //updt->setupinsert(mapKey, fc); 
	      //UpdtCnt[mapKey]=fc;
  	      fcnt->setupinsert(firstKey, fc); 
  	      Bid key = createBid(kw, fc);
  	      srch->setupinsert(key, ind);
    }
      string id = to_string(ind);
      Bid blkcnt(id);
      fcnt->setupinsert(blkcnt,blocks.size());
      int block_num = 1;
      
      for(auto blk: blocks)
      {
	      Bid fb = createBid(id,block_num);
	      file->setupinsert(fb,blk);
	      block_num++;
      }
      fileblks = fileblks+blocks.size();
      inserted = inserted+kws.size();
      cout << "SETUP inserted (kw:"<<kws.size() <<",fb:"<<blocks.size()<<")  ukw:"<<inserted<<" tfb:"<<fileblks<< endl;
}
*/
