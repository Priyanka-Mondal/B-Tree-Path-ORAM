#include "Oriel.h"
#include<string>
#include<math.h>
#include<queue>
#include <unistd.h>
#include<tuple>
#include <boost/algorithm/string.hpp>

int inserted = 0;
int uniquekw = 0;

int stoI(string del_cnt)
{
        int updc;
        stringstream convstoi(del_cnt);
        convstoi >> updc;
        return updc;
}

string delimiters("|+#(){}[]0123456789*?&@=,:!\"><; _-./  \n");
set<string> neg = {"\n","\0", " ", "-","?","from","to", "in"};



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


vector<string> divideString(string str, int sz, int id)
{
        int str_size = str.length();
	//sz = sz - FID_SIZE;
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

Oriel::Oriel(bool usehdd, int maxSize) {
    this->useHDD = usehdd;
    bytes<Key> key1{0};
    bytes<Key> key2{1};
    I = new OMAPf(maxSize*32, key1);
    del = new OMAPf(maxSize*8,key2);
    ac = new OMAPf(maxSize, key2); 
}

Oriel::~Oriel() {
    delete I;
    delete del;
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
      vector<string> blocks;
      blocks = divideString(cont,BLOCK,ind);
      if(!batch)
      	insertWrapper(kws, blocks, ind);
      else
      {
	setupinsertWrap(kws,blocks,ind);
      }
}



void Oriel::setupinsertWrap(vector<string> kws,vector<string> blocks,int ind)
{ 
     int totk=0;
     ofstream alpha;
     alpha.open("alpha.txt",ios::app);	
     //cout << "inserting kw for:"<< ind << endl;
     for (auto kw: kws) 
     {
   	setupinsertkw(kw,ind); // insert all keywords
	totk++;
	tvol[kw]=tvol[kw]+blocks.size();
	float al = (float(idvol[kw]))/(float(tvol[kw]));
	alpha<< kw <<" "<< idvol[kw] <<" "<< tvol[kw]<<" "<<al<<endl;
     }
     //cout << "inserted all the kw (total keywords: " <<totk <<")"<< endl;
     //cout << endl<<"--TOTAL keywords inserted so far: "<<inserted<<endl;
     //cout <<"--TOTAL unique keywords inserted so far: "<<uniquekw<<endl;

}

void Oriel::setupinsertkw(string keyword, int ind) 
{
    inserted++;
    Bid mapKey(keyword);
    int updc = (I->setupfind(mapKey));
    if (updc == 0) 
	 uniquekw++;  
    updc++;
    I->setupinsert(mapKey,updc);
    Bid updKey = createBid(keyword, ind);
    del->setupinsert(updKey, updc);
    Bid key = createBid(keyword, updc);
    I->setupinsert(key, ind);
}


void Oriel::insertWrapper(vector<string> kws,vector<string> blocks,int ind)
{ 
     int totk=0;
     cout << "inserting kw for:"<< ind << endl;
     for (auto kw: kws) 
     {
   	insertkw(kw,ind); // insert all keywords
	totk++;
     }
     cout << "inserted all the kw (total keywords: " <<totk <<")"<< endl;
//     insertFile(ind,blocks); // insert all file blocks
     cout << endl<<"--TOTAL keywords inserted so far: "<<inserted<<endl;
     cout <<"--TOTAL unique keywords inserted so far: "<<uniquekw<<endl;
}

void Oriel::insertkw(string keyword, int ind) 
{
    inserted++;
    Bid mapKey(keyword);
    int updc = (I->find(mapKey));
    if (updc == 0) 
	 uniquekw++;  
    updc++;
    I->insert(mapKey,updc);
    Bid updKey = createBid(keyword, ind);
    del->insert(updKey, updc);
    Bid key = createBid(keyword, updc);
    I->insert(key, ind);
}

/*

void Oriel::remove(string ind) 
{
	string file = "";
	Bid blockcnt(ind);
	string block_cnt = ac->find(blockcnt);
	int bc = 0; 
	if(block_cnt=="")
	{
		cout <<" File does NOT EXIST!"<<endl;
		return;
	}
	else 
	{
		block_cnt = block_cnt.substr(FID_SIZE,block_cnt.size());
		bc = stoI(block_cnt);
	}
        ac->remove(blockcnt);	
	Bid blk;
	for (int i = 1; i <= bc ; i++)
	{
		blk = createBid(ind,i);
		string ret = I->find(blk).second;
		file = file.append(ret);
		I->remove(blk);	
	}
	cout <<"Removed "<< bc <<" blocks from I"<<endl;
	vector<string> kws1;
	boost::split(kws1, file, boost::is_any_of(delimiters));
	vector<string> kws = getUniquedWords(kws1);
	removekw(kws, ind);
}


void Oriel::removekw(vector<string> kws, string ind) 
{
    for(auto keyword : kws)
    {
      Bid mapKey = createBid(keyword, ind);
      string delcnt = del->find(mapKey);
      if (delcnt!="")
      {
        int del_cnt = stoI(delcnt);
	Bid firstKey(keyword);
        string filecnt = ac->find(firstKey);
	if(filecnt=="")
		return;
	int updc = stoI(filecnt);
	int bn = get_block_num(updc,COM);
        int pos = get_position(updc,COM);
	Bid lastKey = createBid(keyword,bn);
	string block = I->find(lastKey).second;
	string lastid = block.substr((pos-1)*FID_SIZE,FID_SIZE);
	updc--;
  	if (updc > 0) 
    	{
    	    ac->insert(firstKey,to_string(updc));
            if (updc + 1 != del_cnt) 
            {
                Bid curKey = createBid(keyword, lastid); 
                del->insert(curKey, delcnt);
                int pos_in_blockdel = get_position(del_cnt,COM);
                int block_del = get_block_num(del_cnt, COM);
                Bid cur = createBid(keyword, block_del);
        	string bl_del = I->find(cur).second;
    		
    		block.replace((pos-1)*FID_SIZE,FID_SIZE,"########");
		I->insert(lastKey,make_pair(KB,block)); // cur
    	        
		string blsec;
    	        if(bn == block_del)
    	         	blsec = block;
    	        else
             	        blsec = bl_del;
    	        
                blsec.replace((pos_in_blockdel-1)*FID_SIZE,FID_SIZE,lastid);
    	        I->insert(cur,make_pair(KB,blsec));
         }
         else
         {
 	       block.replace((pos-1)*FID_SIZE,FID_SIZE,"########");
 	       I->insert(lastKey,make_pair(KB,block));
         }
       } 
       else 
       {
		ac->remove(firstKey);
		I->remove(lastKey);
       }
       del->remove(mapKey); // delete in del
    }
  }
}
*/

vector<int> Oriel::setupsearch(string keyword)
{
    vector<int> fileids; 
    Bid mapKey(keyword);
    int updc = I->setupfind(mapKey); 
    if(updc == 0)
        return fileids;
    int pos = 1;
    while(pos <= updc)
    {
    	mapKey =createBid(keyword,pos);
	pos++;
	auto ret = I->setupfind(mapKey);
	fileids.push_back(ret);
    }
    return fileids;
    //later return files
}

vector<int> Oriel::search(string keyword)
{
    vector<int> fileids; 
    Bid mapKey(keyword);
    int updc = I->find(mapKey); 
    if (updc == 0)
        return fileids;
    int pos = 1;
    while(pos <= updc)
    {
    	mapKey =createBid(keyword,pos);
	pos++;
	auto ret = I->find(mapKey);
	fileids.push_back(ret);
    }
    return fileids;
    //later return files
}

Bid Oriel::createBid(string keyword, int number) 
{
    Bid bid(keyword);
    auto arr = to_bytes(number);
    std::copy(arr.begin(), arr.end(), bid.id.end() - 4);
    return bid;
}

Bid Oriel::createBid(string keyword, string id) 
{
    Bid bid(keyword);
    std::copy(id.begin(), id.end(), bid.id.end() - id.size());
    return bid;
}


void Oriel::print()
{
	del->printTree();
	I->printTree();
}
