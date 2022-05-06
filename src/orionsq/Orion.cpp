#include "Orion.h"
#include <boost/algorithm/string.hpp>
#include "stopword.hpp"
#include<unordered_map>
#include<chrono>

using namespace std::chrono;

Orion::Orion(bool usehdd,int filecnt,int filesize, bool local) 
{
    this->useHDD = false;//usehdd;
    this->local= local;
    bytes<Key> key1{0};
    bytes<Key> key2{1};
    srch = new OMAPf(filecnt, key1);
    fcnt = new OMAPf(filecnt, key1);
    file = new OMAP(filesize,key2);
}

Orion::~Orion() 
{
    delete srch;
    delete file;
    delete fcnt;
    UpdtCnt.clear();
}


int inserted = 0; 
int uniquekw = 0;
int fileblks = 0;
string delimiters("|+#(){}[]0123456789*?&@=,:!\"><; _-./  \n");

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
      if(batch)
	  batchInsert(kws,blocks,fileid);
      else
          insert(kws, blocks, fileid);
}


void Orion::batchInsert(vector<string> kws, vector<string> blocks, int ind) 
{
    for(auto kw: kws)
    {		
  	 Bid firstKey(kw);
  	 int fc = 0;
	 if(!local)
	 {
  	 	if(fcntbids.count(firstKey) > 0)
	   		fc = fcntbids[firstKey];
	 }
	 else if(localFCNT.count(kw)>0)
		 fc = localFCNT[kw];
  	 fc= fc+1;
	 if(!local)
	 	fcntbids[firstKey]=fc;
	 else
		 localFCNT[kw]=fc;
  	 //Bid updKey = createBid(kw, ind);
  	 //updtbids[updKey]=fc;
  	 Bid srchKey = createBid(kw, fc);
  	 srchbids[srchKey]= ind;
    }
    string id = to_string(ind);
    Bid blkcnt(id);
    if(!local)
    	fcntbids[blkcnt]=blocks.size();
    else
	localBCNT[ind]=blocks.size();
    int block_num = 1;
    for(auto blk: blocks)
    {
         Bid fb = createBid(id,block_num);
         filebids[fb]=blk;
         block_num++;
    }
    fileblks = fileblks+block_num;
    cout<<"BATCH inserted keywords and blocks(kw:"<<kws.size() <<",b:"<<blocks.size()<<") of:"<<ind<<" fb:"<<fileblks<< endl;
}
void Orion::endSetup() 
{
        srch->setupInsert(srchbids);
	//updt->setupInsert(updtbids);
	file->setupInsert(filebids);
	if(!local)
         fcnt->setupInsert(fcntbids);
}

void Orion::insert(vector<string> kws, vector<string> blocks, int ind) 
{
    for(auto kw: kws)
    {		
  	      Bid firstKey(kw);
  	      int fc = fcntbids[firstKey];
  	      fc++;
  	      Bid mapKey = createBid(kw, ind);
  	      //updt->insert(mapKey, fc);
	      UpdtCnt[mapKey]=fc;
  	      fcntbids[firstKey]=fc;
	      Bid key = createBid(kw, fc);
  	      srch->insert(key, ind);
	      cout <<endl<<endl;
    }
      string id = to_string(ind);
      Bid blkcnt(id);
      srch->insert(blkcnt,blocks.size());
      localBCNT[ind]=blocks.size();
      int block_num = 1;
      for(auto blk: blocks)
      {
	      Bid fb = createBid(id,block_num);
	      file->insert(fb,blk);
	      cout <<block_num<<blk<<endl;
	      block_num++;
      }
      inserted = inserted+kws.size();
      fileblks = fileblks+block_num-1;
      cout << "INSERTED keywords and blocks(kw:"<<kws.size() <<",b:"<<blocks.size()<<") of:"<<ind<<" uk:"<<uniquekw<<" fb:"<<fileblks<< endl;
      
}

vector<pair<int,string>> Orion::search(string keyword, ofstream& sres, double speed, double latency) 
{
    vector<pair<int,string>> fileblocks;
    int fc = 0;
   if(localFCNT.count(keyword)!=0)
	   fc = localFCNT[keyword];
    if (fc == 0) 
	return fileblocks;

    auto start = high_resolution_clock::now();
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
    }

	 auto stop = high_resolution_clock::now();
	 int totBytes = srch->searchi_bytes + file->searchf_bytes;
	 int rtt = srch->rtt + file->rtt;
	 cout <<"rtt:"<<rtt<<endl;
	 auto duration = duration_cast<milliseconds>(stop-start);
	 double time = double(totBytes)/speed;
	 time = time*1000;
	 int totTime = time+ duration.count()+(latency*rtt); //mili
         sres<<keyword<<" "<< duration.count()<<" "<<totTime<<" "<<fileblocks.size()<<" " <<srch->searchi_bytes <<" "<< file->searchf_bytes<<endl;

cout<<keyword<<" | "<<duration.count()<<" | "<<totTime<<" | "<<fileblocks.size()<<" | "<<srch->searchi_bytes<<" | "<< file->searchf_bytes<<endl;

    return fileblocks;
}

vector<string> Orion::simplebatchSearch(string keyword,ofstream& sres,double speed,double latency) 
{
	vector<string> conts;
	int fc = 0;
        if(localFCNT.count(keyword)>0)
        	fc = localFCNT[keyword];
	else 
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
	auto start = high_resolution_clock::now();
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
   	 conts = file->batchSearch(bids);
	 auto stop = high_resolution_clock::now();
	 int totBytes = srch->searchi_bytes + file->searchf_bytes;
	 auto duration = duration_cast<milliseconds>(stop-start);
	 double time = double(totBytes)/speed;
	 time = time*1000;
	 int totTime = time+ duration.count()+latency; //mili

         sres<<keyword<<" "<< duration.count()<<" "<<totTime<<" "<<conts.size()<<" " <<srch->searchi_bytes <<" "<< file->searchf_bytes<<endl;

cout<<keyword<<" | "<<duration.count()<<" | "<<totTime<<" | "<<conts.size()<<" | "<<srch->searchi_bytes<<" | "<< file->searchf_bytes<<endl;
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

Bid Orion::createBid(string keyword, int number) 
{
    Bid bid(keyword);
    auto arr = to_bytes(number);
    std::copy(arr.begin(), arr.end(), bid.id.end() - 4);
    return bid;
}

void Orion::print()
{
	srch->printTree();
	//updt->printTree();
	//srch->printTree();
}
/*
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
