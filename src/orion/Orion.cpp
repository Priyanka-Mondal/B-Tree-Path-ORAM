#include "Orion.h"
#include <boost/algorithm/string.hpp>
#include "stopword.hpp"
#include<unordered_map>

Orion::Orion(bool usehdd, int filecnt , int filesize, bool local) 
{
    this->useHDD = false;//usehdd;
    this->local= local;
    bytes<Key> key1{0};
    bytes<Key> key2{1};
    srch = new OMAPf(filecnt*10, key1);
    //updt = new OMAPf(filecnt, key1);
}

Orion::~Orion() 
{
    delete srch;
    UpdtCnt.clear();
}


int inserted = 0; 
int uniquekw = 0;
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
      //vector<string> blocks;
      //if(batch)
	//  batchInsert(kws,blocks,fileid);
      //else
      cout <<"ORION INSERT"<<endl;
          insert(kws, fileid);
}

void Orion::insert(vector<string> kws, int ind) 
{
    for(auto kw: kws)
    {	
	    	int fc = 0;
    		if(fcntbids.count(kw)!=0)	    
  	      		fc = fcntbids[kw];
  	      fc++;
  	      Bid mapKey = createBid(kw, ind);
  	      //updt->insert(mapKey, fc);
	      //UpdtCnt[mapKey]=fc;
  	      fcntbids[kw]=fc;
	      Bid key = createBid(kw, fc);
  	      srch->insert(key, ind);
    }
}

int Orion::search(string keyword) 
{
    int files = 0;
    int fc = 0;
    if(fcntbids.count(keyword)!=0)
	    fc = fcntbids[keyword];
    if (fc == 0) 
	return files;
    for (int i = 1; i <= fc; i++) 
    {
	Bid bid = createBid(keyword, i);
        int id = srch->find(bid);
	cout <<"["<<id<<"] ";
	files++;
    }
    cout <<endl;
    return files;
}
/*
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
}
void Orion::endSetup() 
{
        srch->setupInsert(srchbids);
	//updt->setupInsert(updtbids);
	//file->setupInsert(filebids);
	if(!local)
         fcnt->setupInsert(fcntbids);
}
*/

/*
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

*/
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
