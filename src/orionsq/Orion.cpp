#include "Orion.h"
#include <boost/algorithm/string.hpp>


Orion::Orion(bool usehdd, int maxSize, int kwSize) {
    this->useHDD = usehdd;
    bytes<Key> key1{0};
    bytes<Key> key2{1};
    srch = new OMAPf(kwSize*4, key1);
    updt = new OMAPf(kwSize*4, key1);
    fcnt = new OMAPf(kwSize*4, key1);
    file = new OMAP(maxSize*4,key2);
}

Orion::~Orion() {
    delete srch;
    delete updt;
}


int inserted = 0; 
int uniquekw = 0;
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

void Orion::insertWrap(string cont, int fileid)
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
      insert(kws, blocks, fileid);
}



void Orion::insert(vector<string> kws, vector<string> blocks, int ind) 
{
    cout <<"insering keywords for: "<< ind<<endl;
    for(auto kw: kws)
    {		
	      inserted++;
  	      Bid firstKey = createBid(kw,FCNT);
  	      auto filecnt = fcnt->find(firstKey);
  	      int fc;
  	      if(filecnt == "")
	      {
  	      	fc = 0;
		uniquekw++;
	      }
  	      else 
		fc = stoI(filecnt);
  	      
  	      fc++;
  	      
  	      Bid mapKey = createBid(kw, ind);
  	      updt->insert(mapKey, to_string(fc));
  	      fcnt->insert(firstKey, to_string(fc));
  	      
  	      Bid key = createBid(kw, fc);
  	      srch->insert(key, to_string(ind));
    }

      cout << "inserted all the kw (total keywords: " <<kws.size() <<")"<< endl;
		
      //insert blocks
      int block_num = 1;
      string id = to_string(ind);
      for(auto blk: blocks)
      {
	      Bid fb = createBid(id,block_num);
	      file->insert(fb,blk);
	      block_num++;
      }
      Bid lastblock = createBid(id,block_num);
      file->insert(lastblock,"last");
      cout <<"inserted "<<block_num-1<<"blocks of " << id<<endl;
      cout << endl<<"--TOTAL keywords inserted so far: "<<inserted<<endl;
      cout <<"--TOTAL unique keywords inserted so far: "<<uniquekw<<endl;

}

map<int,string> Orion::search(string keyword) {
    
    vector<int> result;
    map<int,string> files;
    vector<Bid> bids;
    Bid firstKey = createBid(keyword,FCNT);
    string filecnt = fcnt->find(firstKey);
    int fc;
    if (filecnt != "") 
    {
	fc = stoI(filecnt);
    }
    else
    {
	    return files;
    }
    for (int i = 1; i <= fc; i++) 
    {
            Bid bid = createBid(keyword, i);
            bids.push_back(bid);
    }
    auto tmpRes = srch->batchSearch(bids);
    for(auto item:tmpRes)
    {
        result.push_back(stoi(item));
    }
    for(auto id:result)
    {
	int j = 1;
	string fileid = to_string(id);
	Bid block = createBid(fileid,j);
        string cont = (file->find(block));
	while(cont != "last")
	{
		if(files.find(id)!=files.end())
		{
			string con = files.at(id);
			con.append(cont);
			files.erase(id);
			files.insert(pair<int,string>(id,con));
		}
		else
		{
			files.insert(pair<int,string>(id,cont));
		}
		j++;
		block = createBid(fileid,j);
		cont = (file->find(block));
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
	string ind = to_string(id);
	for(auto kw : kws)
	{
    		Bid delKey = createBid(kw, id);
    		string delcnt = updt->find(delKey);
		if(delcnt != "")
		{
	    		int del_cnt = stoI(delcnt);
			Bid fcntKey = createBid(kw,FCNT);
			string filecnt = fcnt->find(fcntKey);
	    		int fc = stoI(filecnt);
			int newfilecnt = fc-1;
			Bid lastKey = createBid(kw,fc);
			string last_id = srch->find(lastKey);
			if(del_cnt != fc)
			{
				Bid delkwKey = createBid(kw,del_cnt);
				srch->insert(delkwKey, last_id);
				int lastid = stoI(last_id);
				Bid lastupdKey = createBid(kw,lastid);
				updt->insert(lastupdKey,delcnt);
			}
		//cout << "removing delkey from updt:["<< delKey <<"]"<< endl;
			updt->remove(delKey);
		//cout << "removing lastkey from srch:["<< lastKey <<"]"<< endl;
			srch->remove(lastKey);
			if(newfilecnt == 0)
			{
				//cout << "remove firstKey from fcnt:"<<fcntKey<<endl;
				fcnt->remove(fcntKey);
			}
			else
				fcnt->insert(fcntKey,to_string(newfilecnt));
		}
	}
}



void Orion::setupInsert(string keyword, int ind) {
    Bid mapKey = createBid(keyword, ind);
    if (UpdtCnt.count(keyword) == 0) {
        UpdtCnt[keyword] = 0;
    }
    UpdtCnt[keyword]++;
    setupPairs1[mapKey]=to_string(UpdtCnt[keyword]);
    Bid key = createBid(keyword, UpdtCnt[keyword]);
    setupPairs2[key]= to_string(ind);
    LastIND[keyword] = ind;
}


/**
 * This function executes a remove in setup mode. Indeed, it is not applied until endSetup()
  */
void Orion::setupRemove(string keyword, int ind) {
 Bid mapKey = createBid(keyword, ind);
    string updt_cnt = setupPairs1[mapKey];
    if (stoi(updt_cnt) > 0) {
        setupPairs1[mapKey]= to_string(-1);
        UpdtCnt[keyword]--;
        if (UpdtCnt[keyword] > 0) {
            if (UpdtCnt[keyword] + 1 != stoi(updt_cnt)) {
                Bid curKey = createBid(keyword, LastIND[keyword]);
                setupPairs1[curKey]= updt_cnt;
                Bid curKey2 = createBid(keyword, stoi(updt_cnt));
                setupPairs2[curKey2]= to_string(LastIND[keyword]);
            }
            Bid key = createBid(keyword, UpdtCnt[keyword]);
            string idstr = setupPairs2[key];
            int lastID = stoi(idstr);
            LastIND[keyword] = lastID;
        } else {
            LastIND.erase(keyword);
        }
    }
}


/**
 * This function is used for initial setup of scheme because normal update is time consuming
 */
void Orion::beginSetup() {
    setupPairs1.clear();
    setupPairs2.clear();
}

/**
 * This function is used for finishing setup of scheme because normal update is time consuming
 */
void Orion::endSetup() {
    updt->batchInsert(setupPairs1);
    srch->batchInsert(setupPairs2);
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
