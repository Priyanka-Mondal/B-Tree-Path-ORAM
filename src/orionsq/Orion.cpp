#include "Orion.h"
#include <boost/algorithm/string.hpp>


Orion::Orion(bool usehdd, int maxSize ) {
    this->useHDD = usehdd;
    bytes<Key> key1{0};
    bytes<Key> key2{1};
    srch = new OMAPf(maxSize, key1);
    updt = new OMAPf(maxSize, key1);
    fcnt = new OMAPf(maxSize, key1);
    file = new OMAP(maxSize,key2);
}

Orion::~Orion() {
    delete srch;
    delete updt;
}


int inserted = 0; 
int uniquekw = 0;
int fileblks = 0;
string delimiters("|+#(){}[]0123456789*?&@=,:!\"><; _-./  \n");
set<string> neg = {"\n","\0", " ", "-","?","from","to", "in"};

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
    map<string, int> mp;
    string word;
    for(auto word : kws)
    {
	    if(word.size()<=12 && (neg.find(word)==neg.end()))
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
	      if(neg.find(*it)!=neg.end())
	      {
		 kws.erase(it--);
	      }
      }
      vector<string> blocks;
      blocks = divideString(cont,BLOCK);
      if(batch)
	  setupinsert(kws,blocks,fileid);
      else
          batchInsert(kws, blocks, fileid);
}


void Orion::setupinsert(vector<string> kws, vector<string> blocks, int ind) 
{
    for(auto kw: kws)
    {	
  	      Bid firstKey(kw);
	      int fc = fcnt->setupfind(firstKey);
  	      if(fc == 0)
		uniquekw++;
  	      fc++;
  	      Bid mapKey = createBid(kw, ind);
  	      updt->setupinsert(mapKey, fc); 
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
      cout << "SETUP inserted keywords and blocks(kw:"<<kws.size() <<",b:"<<blocks.size()<<")  uk:"<<uniquekw<<" fb:"<<fileblks<< endl;

}

void Orion::batchInsert(vector<string> kws, vector<string> blocks, int ind) 
{
    map<Bid,int> updtbids;
    map<Bid,int> fcntbids;
    map<Bid,int> srchbids;
    map<Bid,string> filebids;
    for(auto kw: kws)
    {		
  	 Bid firstKey(kw);
  	 int fc = fcnt->find(firstKey);
  	 if(fc == 0)
	   uniquekw++;
  	 fc++;
	 fcntbids.insert(make_pair(firstKey,fc));
  	 Bid updKey = createBid(kw, ind);
  	 updtbids.insert(make_pair(updKey, fc));
  	 Bid srchKey = createBid(kw, fc);
  	 srchbids.insert(make_pair(srchKey, ind));
    }
    string id = to_string(ind);
    Bid blkcnt(id);
    fcntbids.insert(make_pair(blkcnt,blocks.size()));
    int block_num = 1;
    for(auto blk: blocks)
    {
         Bid fb = createBid(id,block_num);
         filebids.insert(make_pair(fb,blk));
         block_num++;
    }
    fcnt->batchInsert(fcntbids);
    updt->batchInsert(updtbids);
    srch->batchInsert(srchbids);
    file->batchInsert(filebids);
    inserted = inserted+kws.size();
    fileblks = fileblks+block_num;
    cout << "BATCH inserted keywords and blocks(kw:"<<kws.size() <<",b:"<<blocks.size()<<") of:"<<ind<<" uk:"<<uniquekw<<" fb:"<<fileblks<< endl;
}

void Orion::insert(vector<string> kws, vector<string> blocks, int ind) 
{
    for(auto kw: kws)
    {		
  	      Bid firstKey(kw);
  	      int fc = fcnt->find(firstKey);
  	      if(fc == 0)
		uniquekw++;
  	      fc++;
  	      Bid mapKey = createBid(kw, ind);
  	      updt->insert(mapKey, fc);
  	      fcnt->insert(firstKey, fc);
  	      Bid key = createBid(kw, fc);
  	      srch->insert(key, ind);
    }
      string id = to_string(ind);
      Bid blkcnt(id);
      fcnt->insert(blkcnt,blocks.size());
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
    int fc = fcnt->find(firstKey);
    cout <<"UPDC:"<< fc<< endl;
    if (fc == 0) 
	return fileblocks;
    for (int i = 1; i <= fc; i++) 
    {
	Bid bid = createBid(keyword, i);
        int id = srch->find(bid);
	string fileid = to_string(id);
	Bid blkcnt(fileid);
        int blocknum = fcnt->find(blkcnt);
	for (int j= 1;j<=blocknum;j++)
	{
		Bid block = createBid(fileid,j);
        	string cont = file->find(block);
		fileblocks.push_back(make_pair(id,cont));
	}
    }//client will figure out files
    return fileblocks;
}

map<int,string> Orion::batchSearch(string keyword) 
{
    vector<int> result;
    map<int,string> files;
    vector<Bid> bids;
    Bid firstKey(keyword);
    int fc = fcnt->find(firstKey);
    cout <<"UPDC:"<< fc<< endl;
    if (fc == 0) 
	    return files;
    for (int i = 1; i <= fc; i++) 
    {
            Bid bid = createBid(keyword, i);
            bids.push_back(bid);
    }
    result = srch->batchSearch(bids);
    for(int id:result)
    {
        string fileid = to_string(id);
	Bid blkcnt(fileid);
        int blocknum = fcnt->find(blkcnt);
	bids.clear();
	for (int j= 1;j<=blocknum;j++)
	{
		Bid block = createBid(fileid,j);
		bids.push_back(block);
	}
        	vector<string> conts = file->batchSearch(bids);
		for(string cont:conts)
		{
			if(files.find(id)!=files.end())
			{
				string con = files.at(id);
				con.append(cont);
				files.erase(id);
				files.insert(pair<int,string>(id,con));
			}
			else
				files.insert(pair<int,string>(id,cont));
		}
     }
    return files;
}

void Orion::remove(int id) 
{
	string ind = to_string(id);
	string cont = "";
	int blk = 1;
	Bid del = createBid(ind, blk);
	string ret = (file->find(del));
	if(ret=="")
	{
		cout <<" File does NOT EXIST!"<<endl;
		return;
	}
	while (ret != "last")
	{
		cont = cont.append(ret);
		blk++;
		del = createBid(ind, blk);
		ret = file->find(del);
		file->remove(del);	
	}
	cout <<"Removed "<<blk<<" blocks from srch"<<del<<endl;
	
	vector<string> kws1;
	boost::split(kws1, cont, boost::is_any_of(delimiters));
	vector<string> kws = getUniquedWords(kws1);
	removekw(kws, id);
}



void Orion::removekw(vector <string> kws, int id) 
{
	for(auto kw : kws)
	{
    		Bid delKey = createBid(kw, id);
    		int delcnt = updt->find(delKey);
		if(delcnt != 0)
		{
			Bid fcntKey(kw);
			int fc = fcnt->find(fcntKey);
			int newfilecnt = fc-1;
			Bid lastKey = createBid(kw,fc);
			int lastid = srch->find(lastKey);
			if(delcnt != fc)
			{
				Bid delkwKey = createBid(kw,delcnt);
				srch->insert(delkwKey, lastid);
				Bid lastupdKey = createBid(kw,lastid);
				updt->insert(lastupdKey,delcnt);
			}
			updt->remove(delKey);
			srch->remove(lastKey);
			if(newfilecnt == 0)
				fcnt->remove(fcntKey);
			else
				fcnt->insert(fcntKey,newfilecnt);
		}
	}
}

Bid Orion::createBid(string keyword, int number) {
    Bid bid(keyword);
    auto arr = to_bytes(number);
    std::copy(arr.begin(), arr.end(), bid.id.end() - 4);
    return bid;
}

void Orion::print()
{
	fcnt->printTree();
	updt->printTree();
	srch->printTree();
}
