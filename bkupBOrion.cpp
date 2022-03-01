#include "BOrion.h"
#include<string>
#include<math.h>
#include<queue>
#include <unistd.h>
#include<tuple>
#include <boost/algorithm/string.hpp>

int inserted = 0;
int uniquekw = 0;

int stoI(string updt_cnt)
{
        int updc;
        stringstream convstoi(updt_cnt);
        convstoi >> updc;
        return updc;
}

int get_block_num(int updc, int com)
{
       float bnum = ceil(updc/com); // 16 file-ids each block
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


vector<string> divideString(string str, int sz, string id)
{
        int str_size = str.length();
	sz = sz - FID_SIZE;
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
		     string ttemp =id; //id
		     ttemp.append(temp); 
                     result.push_back(ttemp);    
                  }
                  temp="";
            }
	    temp +=str[i];
	}
        string ttemp = id;//id;
	ttemp.append(temp);	
	result.push_back(ttemp);
	return result;
}

BOrion::BOrion(bool usehdd, int maxSize) {
    this->useHDD = usehdd;
    bytes<Key> key1{0};
    bytes<Key> key2{1};
    srch = new OMAP(maxSize*16, key1);
    updt = new OMAPf(maxSize, key2); // use key2
    fcnt = new OMAPf(maxSize,key2);
}

BOrion::~BOrion() {
    delete srch;
    delete updt;
}

void BOrion::insertWrap(string cont, string ind, bool batch)
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


void BOrion::setupInsertWrapper(vector<string> kws,vector<string> blocks,string ind)
{ 
	beginSetup();
        setupInsertkws(kws,ind);
	endSetup();
        cout <<"-------Keywords batch inserted---------"<<endl;
	
	beginSetup();
        setupInsertFile(ind,blocks);
	endSetup();
}


void BOrion::setupInsertFile(string ind, vector<string> blocks)
{
	Bid numKey(ind);
	int num_block = blocks.size();
	string nb = ind;
	nb.append(to_string(num_block));
	fcntmap[numKey] = nb;
	int i = 1;
	for(auto block:blocks)
	{
		Bid blkKey = createBid(ind,i);
		srchmap[blkKey]=make_pair(FB,block);
		i++;
	}
	cout <<i<<" blocks inserted for " << ind <<endl;
}


void BOrion::setupInsertkws(vector<string> kws, string ind) 
{
  for(auto keyword:kws)
  {
    cout <<keyword<<":" <<ind<<endl;
    inserted++;
    Bid mapKey(keyword);
    auto updt_cnt = (fcnt->find(mapKey));
    int updc; 
    if (updt_cnt == "") 
    {  
         uniquekw++;
         updc=0;
    }
    else
    {
        updc = stoI(updt_cnt);
    }
    updc++;

    fcntmap[mapKey] = to_string(updc);
    Bid updKey = createBid(keyword, ind);
    updmap[updKey]=to_string(updc);

    int pos_in_block = get_position(updc,COM);
    int block_num = get_block_num(updc,COM);
    Bid key = createBid(keyword, block_num);
    	
    if(pos_in_block == 1)
    {
	   string id = ind;
           id.insert(FID_SIZE, BLOCK-FID_SIZE, '#'); // padding happpens
	   srchmap[key]=make_pair(KB,id);
    }
    else if (pos_in_block >=2 && pos_in_block <=COM)
    {
          string oldblock = (srch->find(key)).second;
	  if(oldblock.size()>0)
          oldblock.replace((pos_in_block-1)*FID_SIZE,FID_SIZE,ind);
	  srchmap[key]=make_pair(KB,oldblock);
     }
     else
     {
          cout << endl << "{pos_in_block greater than 16}" << endl;
     }
  }
	cout <<"end of keyword batch creation"<<endl;
}

void BOrion::setupinsertWrap(vector<string> kws,vector<string> blocks,string ind)
{ 
     int totk=0;
     cout << "inserting kw for:"<< ind << endl;
     for (auto kw: kws) 
     {
   	setupinsertkw(kw,ind); // insert all keywords
	totk++;
     }
     cout << "inserted all the kw (total keywords: " <<totk <<")"<< endl;
     insertFile(ind,blocks); // insert all file blocks
     cout << endl<<"--TOTAL keywords inserted so far: "<<inserted<<endl;
     cout <<"--TOTAL unique keywords inserted so far: "<<uniquekw<<endl;
}

void BOrion::setupinsertkw(string keyword, string ind) 
{
    inserted++;
    Bid mapKey(keyword);
    cout <<"setup finding"<<mapKey<<endl;
    auto updt_cnt = (fcnt->setupfind(mapKey));
    int updc; 
    if (updt_cnt == "") 
    {
	 uniquekw++;  
         updc=0;
    }
    else
    {
        updc = stoI(updt_cnt);
    }
    updc++;
    cout <<keyword<<":updt cnt is:"<< updt_cnt <<"/"<<updc<<endl;
    fcnt->setupinsert(mapKey,to_string(updc));

    Bid updKey = createBid(keyword, ind);
    updt->insert(updKey, to_string(updc));//pad, can we store number in updc 
    int pos_in_block = get_position(updc,COM);
    int block_num = get_block_num(updc,COM);
       
    Bid key = createBid(keyword, block_num);
    if(pos_in_block == 1)
    {
           string id = ind;
           id.insert(FID_SIZE, BLOCK-FID_SIZE, '#'); // padding happpens
           srch->insert(key, make_pair(KB,id));
    }
    else if (pos_in_block >=2 && pos_in_block <=COM)
    {
          string oldblock = (srch->setupfind(key)).second;
          oldblock.replace((pos_in_block-1)*FID_SIZE,FID_SIZE,ind);
	  srch->insert(key, make_pair(KB,oldblock));
    }
    else
    {
         cout << endl << "{pos_in_block greater than 8}" << endl;
    }
}
void BOrion::insertWrapper(vector<string> kws,vector<string> blocks,string ind)
{ 
     int totk=0;
     cout << "inserting kw for:"<< ind << endl;
     for (auto kw: kws) 
     {
   	insertkw(kw,ind); // insert all keywords
	totk++;
     }
     cout << "inserted all the kw (total keywords: " <<totk <<")"<< endl;
     insertFile(ind,blocks); // insert all file blocks
     cout << endl<<"--TOTAL keywords inserted so far: "<<inserted<<endl;
     cout <<"--TOTAL unique keywords inserted so far: "<<uniquekw<<endl;
}

void BOrion::insertkw(string keyword, string ind) 
{
    inserted++;
    Bid mapKey(keyword);
    auto updt_cnt = (fcnt->find(mapKey));
    int updc; 
    if (updt_cnt == "") 
    {
	 uniquekw++;  
         updc=0;
    }
    else
    {
        updc = stoI(updt_cnt);
    }
    updc++;
    fcnt->insert(mapKey,to_string(updc));

    Bid updKey = createBid(keyword, ind);
    updt->insert(updKey, to_string(updc));//pad, can we store number in updc 

    int pos_in_block = get_position(updc,COM);
    int block_num = get_block_num(updc,COM);
       
    Bid key = createBid(keyword, block_num);
    if(pos_in_block == 1)
    {
           string id = ind;
           id.insert(FID_SIZE, BLOCK-FID_SIZE, '#'); // padding happpens
           srch->insert(key, make_pair(KB,id));
    }
    else if (pos_in_block >=2 && pos_in_block <=COM)
    {
          string oldblock = (srch->find(key)).second;
          oldblock.replace((pos_in_block-1)*FID_SIZE,FID_SIZE,ind);
	  srch->insert(key, make_pair(KB,oldblock));
    }
    else
    {
         cout << endl << "{pos_in_block greater than 8}" << endl;
    }
}


void BOrion::insertFile(string ind, vector<string> blocks)
{
	Bid mapKey(ind);
	int sz = blocks.size();
        string par = ind;
	par.append(to_string(sz));// pad later to 64 byte
	fcnt->insert(mapKey, par);
	int i =1;
        for(auto block : blocks)
	{       
		Bid mk = createBid(ind, i);
		srch->insert(mk,make_pair(FB,block));
		i++;
	}
	cout << "inserted "<<  blocks.size() <<" blocks of " << ind << endl;
}

void BOrion::remove(string ind) 
{
	string file = "";
	Bid blockcnt(ind);
	string block_cnt = fcnt->find(blockcnt);
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
        fcnt->remove(blockcnt);	
	Bid blk;
	for (int i = 1; i <= bc ; i++)
	{
		blk = createBid(ind,i);
		string ret = srch->find(blk).second;
		file = file.append(ret);
		srch->remove(blk);	
	}
	cout <<"Removed "<< bc <<" blocks from srch"<<endl;
	vector<string> kws1;
	boost::split(kws1, file, boost::is_any_of(delimiters));
	vector<string> kws = getUniquedWords(kws1);
	removekw(kws, ind);
}


void BOrion::removekw(vector<string> kws, string ind) 
{
    for(auto keyword : kws)
    {
      Bid mapKey = createBid(keyword, ind);
      string delcnt = updt->find(mapKey);
      if (delcnt!="")
      {
        int del_cnt = stoI(delcnt);
	Bid firstKey(keyword);
        string filecnt = fcnt->find(firstKey);
	if(filecnt=="")
		return;
	int updc = stoI(filecnt);
	int bn = get_block_num(updc,COM);
        int pos = get_position(updc,COM);
	Bid lastKey = createBid(keyword,bn);
	string block = srch->find(lastKey).second;
	string lastid = block.substr((pos-1)*FID_SIZE,FID_SIZE);
	updc--;
  	if (updc > 0) 
    	{
    	    fcnt->insert(firstKey,to_string(updc));
            if (updc + 1 != del_cnt) 
            {
                Bid curKey = createBid(keyword, lastid); 
                updt->insert(curKey, delcnt);
                int pos_in_blockdel = get_position(del_cnt,COM);
                int block_del = get_block_num(del_cnt, COM);
                Bid cur = createBid(keyword, block_del);
        	string bl_del = srch->find(cur).second;
    		
    		block.replace((pos-1)*FID_SIZE,FID_SIZE,"########");
		srch->insert(lastKey,make_pair(KB,block)); // cur
    	        
		string blsec;
    	        if(bn == block_del)
    	         	blsec = block;
    	        else
             	        blsec = bl_del;
    	        
                blsec.replace((pos_in_blockdel-1)*FID_SIZE,FID_SIZE,lastid);
    	        srch->insert(cur,make_pair(KB,blsec));
         }
         else
         {
 	       block.replace((pos-1)*FID_SIZE,FID_SIZE,"########");
 	       srch->insert(lastKey,make_pair(KB,block));
         }
       } 
       else 
       {
		fcnt->remove(firstKey);
		srch->remove(lastKey);
       }
       updt->remove(mapKey); // delete in updt
    }
  }
}


vector<pair<string,string>> BOrion::setupsearch(string keyword)
{
    vector<pair<string,string>> result;
    vector <string> counts;
    Bid mapKey(keyword);
    auto updt_cnt = fcnt->setupfind(mapKey); //0. get file count i.e. updc 
    int updc;    
    if (updt_cnt != "") 
	updc = stoI(updt_cnt);
    else
        return result;

    int pos = 1;
    mapKey = createBid(keyword,pos);
    auto freq = srch->setupfind(mapKey); //1. first COM fileids
    int fetched = 0;
    vector<Bid> fileids; // fileids
    vector<Bid> fileblocks; // fileids
    int point = 0;
    while(fetched < updc && point < COM)
    {
	    string str = freq.second.substr(point*FID_SIZE,FID_SIZE);
            Bid bid = createBid(str,0); 
	    fileids.push_back(bid);
	    fetched++;
	    point++;
    }
    pos++;
    counts = fcnt->batchSearch(fileids); //2. block-size of first COM fileids
    for(auto id : counts)
    {
	string idd = id.substr(0,FID_SIZE);
	string size = id.substr(FID_SIZE,id.size());
	int sz = stoI(size);
	for(int i = 1;i<=sz;i++)
	{
		Bid fb = createBid(idd,i);
		fileblocks.push_back(fb);
	}
    }
    fileids.clear();
    while(fetched < updc) // get rest of ids
    {
    	    mapKey = createBid(keyword,pos);
            auto freq = srch->setupfind(mapKey); // 3. next COM fileids
	    point = 0;
	    while(fetched < updc && point < COM)
	    {
	    	string str = freq.second.substr(point*FID_SIZE,FID_SIZE);
            	Bid bid = createBid(str,0); // for first file block
	    	fileids.push_back(bid);
	    	point++;
	    	fetched++;
	    }
	    pos++;
    }
    counts = fcnt->batchSearch(fileids);//3. block-size of rest of the files
    for(auto id : counts)
    {
	string idd = id.substr(0,FID_SIZE);
	string size = id.substr(FID_SIZE,id.size());
	int sz = stoI(size);
	for(int i = 1;i<=sz;i++)
	{
		Bid fb = createBid(idd,i);
		fileblocks.push_back(fb);
	}
    }
    result.clear();
    result = srch->batchSearch(fileblocks); // 4. all file blocks
    // the client should identify the blocks of same files
    return result;
}

vector<pair<string,string>> BOrion::search(string keyword)
{
    vector<pair<string,string>> result;
    vector <string> counts;
    Bid mapKey(keyword);
    auto updt_cnt = fcnt->find(mapKey); //0. get file count i.e. updc 
    int updc;    
    if (updt_cnt != "") 
	updc = stoI(updt_cnt);
    else
        return result;

    int pos = 1;
    mapKey = createBid(keyword,pos);
    auto freq = srch->find(mapKey); //1. first COM fileids
    int fetched = 0;
    vector<Bid> fileids; // fileids
    vector<Bid> fileblocks; // fileids
    int point = 0;
    while(fetched < updc && point < COM)
    {
	    string str = freq.second.substr(point*FID_SIZE,FID_SIZE);
            Bid bid = createBid(str,0); 
	    fileids.push_back(bid);
	    fetched++;
	    point++;
    }
    pos++;
    counts = fcnt->batchSearch(fileids); //2. block-size of first COM fileids
    for(auto id : counts)
    {
	string idd = id.substr(0,FID_SIZE);
	string size = id.substr(FID_SIZE,id.size());
	int sz = stoI(size);
	for(int i = 1;i<=sz;i++)
	{
		Bid fb = createBid(idd,i);
		fileblocks.push_back(fb);
	}
    }
    fileids.clear();
    while(fetched < updc) // get rest of ids
    {
    	    mapKey = createBid(keyword,pos);
            auto freq = srch->find(mapKey); // 3. next COM fileids
	    point = 0;
	    while(fetched < updc && point < COM)
	    {
	    	string str = freq.second.substr(point*FID_SIZE,FID_SIZE);
            	Bid bid = createBid(str,0); // for first file block
	    	fileids.push_back(bid);
	    	point++;
	    	fetched++;
	    }
	    pos++;
    }
    counts = fcnt->batchSearch(fileids);//3. block-size of rest of the files
    for(auto id : counts)
    {
	string idd = id.substr(0,FID_SIZE);
	string size = id.substr(FID_SIZE,id.size());
	int sz = stoI(size);
	for(int i = 1;i<=sz;i++)
	{
		Bid fb = createBid(idd,i);
		fileblocks.push_back(fb);
	}
    }
    result.clear();
    result = srch->batchSearch(fileblocks); // 4. all file blocks
    // the client should identify the blocks of same files
    return result;
}

map<string,string> BOrion::searchWrapper(string keyword)
{
	srand(time(NULL));
	vector<pair<string,string>> result;
	vector<string> firstresult;
	map<string,string> files;
        int fetched=0, updc;
    	vector<Bid> bids;
    	//Bid mapKey = createBid(keyword, 0);
	Bid mapKey(keyword);
    
    auto updt_cnt = fcnt->find(mapKey); // mandatory first search: #fileids
    if (updt_cnt != "") {
	updc = stoI(updt_cnt);
	cout <<"updt_cnt is "<<updt_cnt<<endl;
    }
    else
    {
	    cout <<"updt count is 0"<<endl;
	    return files;
    }
    
    queue<Bid> fileids; // fileids
    queue<string> firstblockid; //ids of files
    queue<string> firstblocknum;//#blocks of files
    mapKey = createBid(keyword,1);
    
    auto freq = srch->find(mapKey); //mandatory second search: first 8 fileids
    
    auto fileids1 = freq.second;  // get first <=8 fileids
    int pos = 0;
    while(fetched < updc && fetched < COM)
    {
	    string str = freq.second.substr(pos*FID_SIZE,FID_SIZE);
	    cout <<"the index:["<<str<<"]"<<endl<<endl;
            Bid bid = createBid(str,0); // for first file block
	    fileids.push(bid);
	    pos++;
	    fetched++;
    }
    int ran; // random number
    int idblock = 2; // next index block to be fetched is block 2
    
    //randomize next OMAP accesses 
    while(fetched < updc || fileids.size()>0 || firstblockid.size()>0)
    {
	    vector<Bid> batch;
	    vector<Bid> firstbatch;
	    if(fetched < updc)
	    {
		int idblksleft = ceil((updc-fetched)/COM);
		if ((updc-fetched)%COM != 0) idblksleft++;
		cout << "idblksleft:["<<idblksleft<<"]"<<endl;
		
		ran = rand()%idblksleft+1;
		int cnt = 0;
		while(cnt<ran)
		{
			Bid bid = createBid(keyword,idblock+cnt);
			batch.push_back(bid);
			cnt++;
			idblock++;
		}
	    }
	    if(fileids.size() > 0)
	    {
		ran = rand()%fileids.size()+1;
		int cnt = 0;
		while(cnt<ran)
		{
			firstbatch.push_back(fileids.front());
			fileids.pop();
			cnt++;
		}
	    }
	    if(firstblockid.size() > 0)
	    {
		    ran = rand()%firstblockid.size()+1;
		    int cnt = 1;
		    while(cnt<=ran)
		    {
			    string str = firstblockid.front();
			    string numb = firstblocknum.front();
			    int num = stoI(numb);
			    int bnums = 1;
			    while(bnums<=num) // all blocks of file fetched
			    {
			        Bid bid = createBid(str,bnums);
				bnums++;
			        batch.push_back(bid);
			    }
			    cnt++;
			    firstblockid.pop();
			    firstblocknum.pop();
		    }
	    }
	    //cout << "before BATCH SEARCH"<< endl;
	    result = srch->batchSearch(batch);
	    firstresult = fcnt->batchSearch(firstbatch);
	    //cout << "after BATCH SEARCH"<< endl;
	   
	    for(auto first : firstresult)
	    {
		string id = first.substr(0,FID_SIZE);
		int sz = first.length(); // change to :until #
		string bnum = first.substr(FID_SIZE,sz);
		//cout << "SIZE of File: "<< id << " IS:"<< bnum << endl;
		firstblocknum.push(bnum);
		firstblockid.push(id);
	    }

	    for(auto blk : result)
	    {
	       //cout << "first:" << blk.first << "second:" <<blk.second <<endl;
	        if(blk.first==KB)
		{
			int cnt = 0;
			while(cnt < COM && fetched<updc)
			{
			string str = blk.second.substr(cnt*FID_SIZE,FID_SIZE);
				Bid bid = createBid(str,0);
				fileids.push(bid);
				fetched++;
				cnt++;
			}
		}
	/*	
		if(blk.first==FS)
		{
			string id = blk.second.substr(0,FID_SIZE);
			int sz = blk.second.length(); // change to :until #
			string bnum = blk.second.substr(FID_SIZE,sz);
		//cout << "SIZE of File: "<< id << " IS:"<< bnum << endl;
			firstblocknum.push(bnum);
			firstblockid.push(id);
		}
		*/
		if(blk.first==FB)
		{
			string id = blk.second.substr(0,FID_SIZE);
			int sz = blk.second.length();
			string bcontent = blk.second.substr(FID_SIZE,sz);
		//cout << "CONT ofFile:"<< id << "IS:"<< bcontent << endl;

			if(files.find(id)!=files.end())
			{
				string con = files.at(id);
				con.append(bcontent);
				files.erase(id);
				files.insert(pair<string,string>(id,con));
		//   cout << "CONT:"<< id << " :"<< con << endl;
		//	   cout<<"------------------------------------"<<endl;
			}
			else
			{
				files.insert(pair<string,string>(id,bcontent));
		//   cout << "***CONT:"<< id << " :"<< bcontent << endl;
			   //cout<<"------------------------------------"<<endl;
			}
		}
	    }
    }
return files;
}



void BOrion::beginSetup() 
{
    srchmap.clear();
    updmap.clear();
    fcntmap.clear();
}

void BOrion::endSetup() 
{
    fcnt->batchInsert(fcntmap);
    updt->batchInsert(updmap);
    srch->batchInsert(srchmap);
}

Bid BOrion::createBid(string keyword, int number) 
{
    Bid bid(keyword);
    auto arr = to_bytes(number);
    std::copy(arr.begin(), arr.end(), bid.id.end() - 4);
    return bid;
}

Bid BOrion::createBid(string keyword, string id) 
{
    Bid bid(keyword);
    std::copy(id.begin(), id.end(), bid.id.end() - id.size());
    return bid;
}


void BOrion::print()
{
	updt->printTree();
	srch->printTree();
}
