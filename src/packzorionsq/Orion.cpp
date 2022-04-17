#include "Orion.h"
#include <boost/algorithm/string.hpp>
#include "stopword.hpp"


Orion::Orion(bool usehdd, int filecnt , int filesize, bool local): rd(), mt(rd()), dis(0, (pow(2, floor(log2(filesize / Z)) + 1) - 1) / 2) 
{
    this->useHDD = false;//usehdd;
    this->local= local;
    bytes<Key> key1{0};
    bytes<Key> key2{1};
    //fcnt = new OMAPf(filecnt, key1);
    srch = new IndexORAM(filecnt, key1);
    fileoram = new FileORAM(filesize, key1);
    fileleaves = (pow(2, floor(log2(filesize/Z))+1)-1)/2;
    indexleaves = (pow(2, floor(log2(filecnt/Z))+1)-1)/2;
}

Orion::~Orion() 
{
    delete srch;
    delete fileoram;
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
vector<string> getUniquedWords(vector<string> kws)
{
    vector<string> kw;
    map<string, int> mp;
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
    for (map<string, int> :: iterator p = mp.begin();
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
     // if(batch)
	  batchInsert(kws,blocks,fileid);
 //     else
   //       insert(kws, blocks, fileid);
}



Fnode* Orion::newFnode(Fbid key, string value, int pos) 
{
    Fnode* node = new Fnode();
    node->key = key;
    std::fill(node->value.begin(), node->value.end(), 0);
    std::copy(value.begin(), value.end(), node->value.begin());
    node->pos = pos;
    return node;
}
Node* Orion::newNode(Bid key, array<byte_t, NODE_SIZE> value, int pos) 
{
    Node* node = new Node();
    node->key = key;
    std::copy(value.begin(), value.end(), node->value.begin());
    node->pos = pos;
    return node;
}
void Orion::batchInsert(vector<string> kws, vector<string> blocks, int ind) 
{
    for(auto kw: kws)
    {		
  	 Bid firstKey(kw);
  	 int fc = 0;
	 int sc = 0;
	 //string scfc="0-0";
	 /*if(!local)
	 {
  	 	if(fcntbids.count(firstKey) > 0)
	   		scfc = fcntbids[firstKey];
	 }*/
	 //if(localFCNT.count(kw)>0)
	//	 scfc = localFCNT[kw];
	 if(localFC.count(kw)>0)
	 {
		 fc = localFC[kw];
	 	 sc = localSC[kw];
	 }
         //auto parts = Utilities::splitData(scfc, "-");
         //sc = stoi(parts[0]);
         //fc = stoi(parts[1]);
  	 fc= fc+1;
	 //if(!local)
	 //	fcntbids[firstKey]=to_string(sc)+"-"+to_string(fc);
	 //else
		//localFCNT[kw]=to_string(sc)+"-"+to_string(fc);
	 localFC[kw] = fc;
	 int bnum = get_block_num(fc, COM);
	 int pos = get_position(fc, COM);
	 Bid kbid = createBid(kw,bnum);
    	 int poskw = RandomSeedPath(kw,sc,bnum,indexleaves);
	 if(pos == 1)
	 {
		auto arr = to_bytes(ind);
		array<byte_t,NODE_SIZE> t;
		fill(t.begin(),t.end(),0);
		copy(arr.begin(),arr.end(),t.begin());
	 	Node *kwnode = newNode(kbid,t,poskw);
	 	//srchnodes.push_back(kwnode);
		srchmap[kbid]=kwnode;
	 }
	 else //pos >=2
	 {
		Node *kwnode = srchmap[kbid];
		auto arr = to_bytes(ind);
		array<byte_t ,NODE_SIZE> tk;
		//memcpy(dest,src,count)
		copy(kwnode->value.begin(), kwnode->value.end(), tk.begin());
        	copy(arr.begin(), arr.end(), tk.begin()+((pos-1)*FILEID));
		kwnode = newNode(kbid,tk,poskw);
		srchmap[kbid]=kwnode;
	 }
    }
    //string id = to_string(ind);
    //string acbc = "0-"+to_string(blocks.size());
    localAC[ind]=0;
    localBC[ind]=blocks.size();
    //localACNT[id]=acbc;
    int block_num = 1;
    for(auto blk: blocks)
    {
	   //Fbid fbid = createFbid(id,block_num);
	   Fbid fbid = createFbid(ind,block_num);
	   //cout <<"size of fbid:"<<fbid.size()<<endl;
	   int pos = RandomPath(ind,0,block_num,fileleaves);
	   //cout <<"leaf for:"<<id<<"|"<<block_num<<":"<<pos<<"-"<<fbid<<endl;
	   Fnode *fnode = newFnode(fbid,blk,pos);
	   //fileoram->WriteFnode(fbid,fnode);
	   filenodes.push_back(fnode);
	   block_num++;
    }
    //fileoram->finalizeindex();
    fileblks = fileblks+block_num-1;
    cout<<"BATCH inserted keywords and blocks(kw:"<<kws.size() <<",b:"<<blocks.size()<<") of fileid: "<<ind<<" fb:"<<fileblks<< endl;
}

void Orion::endSetup() 
{
	for(n:srchmap) // because of packing I did this
		srchnodes.push_back(n.second);
	srch->setupInsert(srchnodes);
	fileoram->setupInsert(filenodes);
	//if(!local)
         //fcnt->setupInsert(fcntbids);
}

int Orion::RandomPath(int id, int cntr1, int cntr2, int leaves) 
{
    int sum = 0;
    long int rnd = id+(cntr1+1)*(101%leaves)+((cntr2*999)%leaves);
    int pos = rnd%leaves;
    return pos;
}
int Orion::RandomSeedPath(string id,int cntr1, int cntr2, int leaves) 
{
    int sum = 0;
    for(int i=0;i<id.size();i++)
	sum +=id[i];
    long int rnd = sum+(cntr1+1)*(101%leaves)+((cntr2*999)%leaves);
    int pos = rnd%leaves;
    return pos;
}

pair<int,vector<string>> Orion::simplebatchSearch(string keyword) 
{
	srch->searchi_bytes = 0;
	fileoram->searchf_bytes = 0;
	vector<string> conts;
	int fc = 0;
	int sc = 0;
	//string scfc="0-0";
        if(localFC.count(keyword)>0)
	{
        	sc = localSC[keyword];
        	fc = localFC[keyword];
	}
	else 
	    return make_pair(0,conts);
        //auto parts = Utilities::splitData(scfc, "-");
        //sc = stoi(parts[0]);
        //fc = stoi(parts[1]);
	//scfc = to_string(sc+1)+"-"+to_string(fc);
	localSC[keyword] = sc+1;
        vector<int> result;
	result.reserve(fc);
	srch->start(false);
	int bnum = get_block_num(fc,COM);
	int fetched = 0;
   	for (int i = 1; i <= bnum; i++) 
   	{
   	         Bid kbid = createBid(keyword,i);
		 int poskw = RandomSeedPath(keyword,sc,i,indexleaves);
		 int newpos = RandomSeedPath(keyword,sc+1,i,indexleaves);
		 Node* kwnode = srch->ReadNode(kbid,poskw,newpos);
		 if(i < bnum)
		 {
			 int point = 0;
			 while(point < COM)
			 {
				int next;
				array<byte_t,4> arr;
				memcpy(arr.begin(),kwnode->value.begin()+(point*FILEID), 
										FILEID);
        	 		from_bytes(arr, next);
				point++;
				fetched++;
		 		result.push_back(next);
			 }
		 }
		 else if(i == bnum)
		 {
			 int point = 0;
			 while(point <COM && fetched <fc)
			 {
				int next;
				array<byte_t,4> arr;
				memcpy(arr.begin(),kwnode->value.begin()+(point*FILEID), 
										FILEID);
        	 		from_bytes(arr, next);
				point++;
				fetched++;
		 		result.push_back(next);
			 }
		 }
   	}
	srch->finalizeindex();
	//fcnt->incrementSrcCnt();
	//**********************************************************
	fileoram->start(false);
   	 for(auto fID:result)
   	 {
	 	//string acbc = localACNT[fID];
		int ac = localAC[fID];
		int bc = localBC[fID];
	        //auto parts = Utilities::splitData(acbc, "-");
	        //int ac = stoi(parts[0]);
	        //int bc = stoi(parts[1]);
		//acbc = to_string(ac+1)+"-"+to_string(bc);
		localAC[fID] = ac+1;
		for(int i =1;i<=bc;i++)
		{
			Fbid fbid = createFbid(fID,i);
			int pos = RandomPath(fID,ac,i,fileleaves);
			int newpos = RandomPath(fID,ac+1,i,fileleaves);
			Fnode* fnode = fileoram->ReadFnode(fbid,pos,newpos);
			if(fnode != NULL)
			{
				string temp="";
				temp.assign(fnode->value.begin(), fnode->value.end());
        			temp = temp.c_str();
				conts.push_back(temp);
			}
		}
	 //fileoram->finalizeindex(); -->core dump
   	  }
	 fileoram->finalizeindex();
	 int totBytes = srch->searchi_bytes + fileoram->searchf_bytes;
    return make_pair(totBytes,conts);
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
Fbid Orion::createFbid(int id, int number) 
{
    Fbid bid(id);
    auto arr = to_bytes(number);
    std::copy(arr.begin(), arr.end(), bid.id.end() - 4);
    return bid;
}
Bid Orion::createBid(string id, int number) 
{
    Bid bid(id);
    auto arr = to_bytes(number);
    std::copy(arr.begin(), arr.end(), bid.id.end() - 4);
    return bid;
}

