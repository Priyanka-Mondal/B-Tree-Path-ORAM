#include "MOrion.h"
#include<string>
#include<math.h>
#include<queue>
#include <unistd.h>
#include<tuple>
#include <boost/algorithm/string.hpp>

int inserted = 0;
int uniquekw = 0;
int totblocks = 0;

MOrion::MOrion(bool usehdd, int maxSize, int cnt, bool local, bool notfile) {
    this->useHDD = usehdd;
    this->local = local;
    this->notfile = notfile;
    bytes<Key> key1{0};
    bytes<Key> key2{1};
    srch = new OMAP(maxSize*2, key1);
    fcnt = new OMAPf(cnt,key2);
    //updt = new OMAPf(maxSize*2, key2); 
}
string toS(int id)
{
	string s = to_string(id);
	string front ="";
	if (id < 10)
		front = "0000000";
	else if(id < 100)
		front = "000000";
	else if(id < 1000)
		front = "00000";
	else if(id < 10000)
		front = "0000";
	else if(id < 100000)
		front = "000";
	else if(id < 1000000)
		front = "00";
	else if(id < 10000000)
		front = "0";
	s=front.append(s);
	return s;
}
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

string delimiters("|+#(){}[]0123456789*?&@=,:!\"><; _-./ \n\t");
std::set<std::string> neg = {"_","a","b","c","d","e","f","g","h","i","j","k","l","m","n","o","p","q","r","s","t","u","v","w","x","y","z","\n","\0", " ", "-","?","from","to", "in","on","so","how","me","you","this","that","ok","no","yes","him","her","they","them","not","none","an","under","below","behind","he","she","their","has","our","would","am","may","know","all","can","any","me","or","as","your","it","we","please","at","if","will","are","by","with","be","com","have","is","of","for","and","the","date","cc","up","but","do","what","which","been","where","could","who","would","did","put","done","too","get","got","yet","co","if"};



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
		     //string ttemp =toS(id); //id
		     //ttemp.append(temp); 
                     result.push_back(temp);    
                  }
                  temp="";
            }
	    temp +=str[i];
	}
        //string ttemp = toS(id);//id;
	//ttemp.append(temp);	
	result.push_back(temp);
	return result;
}


MOrion::~MOrion() {
    delete srch;
    //delete updt;
    delete fcnt;
}

void MOrion::insertWrap(string cont, int ind, bool batch)
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
	setupinsertWrapper(kws,blocks,ind);
}




void MOrion::setupinsertWrapper(vector<string>kws,vector<string>blocks,int ind)
{ 
     for (auto kw: kws) 
   	setupinsertkw(kw,ind); 
     if(!notfile)
        setupinsertFile(ind,blocks); 
     inserted= inserted+kws.size();
     cout <<"--TOTAL srch blocks used:"<< (inserted+totblocks)<<endl;

}

void MOrion::setupinsertkw(string keyword, int ind) 
{
    Bid mapKey(keyword);
    int updc =0;
    if(!local){
    if(fcntmap.count(mapKey)>0)
    	updc = fcntmap[mapKey];
    }
    else if(localFCNT.count(keyword)>0)
	updc = localFCNT[keyword];
	   
    if (updc == 0) 
	 uniquekw++;  

    updc++;
    if(!local)
     fcntmap[mapKey]=updc;
    else localFCNT[keyword]=updc;
    //Bid updKey = createBid(keyword, ind);
    //updtmap[updKey]= updc;//pad

    int pos_in_block = get_position(updc,COM);
    int block_num = get_block_num(updc,COM);
    //idvol[keyword] = block_num; 
    Bid key = createBid(keyword, block_num);
    if(pos_in_block == 1)
    {
           string id = toS(ind);
           id.insert(FID_SIZE, BLOCK-FID_SIZE, '#'); // padding happpens
           //srch->setupinsert(key, make_pair(KB,id));
	   srchmap[key]=id;
    }
    else if (pos_in_block >=2 && pos_in_block <=COM)
    {
          string oldblock = srchmap[key];//(srch->setupfind(key)).second;
          oldblock.replace((pos_in_block-1)*FID_SIZE,FID_SIZE,toS(ind));
	  //srch->setupinsert(key, make_pair(KB,oldblock));
	  srchmap[key]=oldblock;
    }
}


void MOrion::setupinsertFile(int id, vector<string> blocks)
{
	string ind = toS(id);
	Bid mapKey(to_string(id));
	int sz = blocks.size();
        //string par = ind;
	//par.append(to_string(sz));// pad later to 64 byte
	//fcnt->setupinsert(mapKey, sz);
	if(!local)
		fcntmap[mapKey]=sz;
	else
		localBCNT[id]=sz;
		fcntmap[mapKey]=sz; //
	int i =1;
        for(auto block : blocks)
	{       
		Bid mk = createBid(to_string(id), i);
		//srch->setupinsert(mk,make_pair(FB,block));
		srchmap[mk]=block;
		i++;
	}
	totblocks= totblocks+blocks.size()+1;
}

void MOrion::endSetup()
{
	srch->setupInsert(srchmap);
	//if(!local)
		fcnt->setupInsert(fcntmap);
}

vector<string> MOrion::simplebatchSearch(string keyword)
{
    vector<string> result;
    int updc;
    if(localFCNT.count(keyword)>0)
        updc = localFCNT[keyword];
    else
        return result;
    int blocks= get_block_num(updc, COM);
    vector<Bid> bids;
    bids.reserve(blocks);
    for(int i=1;i<=blocks;i++)
    {
	Bid b = createBid(keyword,i);
	bids.push_back(b);
    }
    vector<string> fids;
    fids.reserve(blocks);
    fids = srch->batchSearch(bids);
    int fetched = 0;
    vector<int> filind;
    filind.reserve(updc);
    int point=0;
    for(auto ids:fids)
    {
	   
	    while(fetched < updc && point < COM)
	    {
	    	string str = ids.substr(point*FID_SIZE,FID_SIZE);
	    	point++;
	    	fetched++;
		int ii = stoI(str);
		filind.push_back(ii);
	     }
	     point = 0;
    }
    bids.clear();
    for(auto k :filind)
    {
	string ff = to_string(k);
        int sz = localBCNT[k];
	for(int i = 1;i<=sz;i++)
	{
		Bid fb = createBid(ff,i);
		bids.push_back(fb);
	}
    }
    result=srch->batchSearch(bids); 
    return result;
}
vector<string> MOrion::batchSearch(string keyword)
{
    vector<string> result;
    int updc=0;
    if(!local)
    {
    	Bid mapKey(keyword);
    	updc = fcnt->find(mapKey); 
    }
    else 
        updc = localFCNT[keyword];
    if (updc == 0) 
        return result;
    int blocks= get_block_num(updc, COM);
    vector<Bid> bids;
    bids.reserve(blocks);
    for(int i=1;i<=blocks;i++)
    {
	Bid b = createBid(keyword,i);
	bids.push_back(b);
    }
    vector<string> fids;
    fids.reserve(blocks);
    fids = srch->batchSearch(bids);
    int fetched = 0;
    vector<int> filind;
    filind.reserve(updc);
    int point;
    for(auto ids:fids)
    {
	   
	    point = 0;
	    while(fetched < updc && point < COM)
	    {
	    	string str = ids.substr(point*FID_SIZE,FID_SIZE);
	    	point++;
	    	fetched++;
		int ii = stoI(str);
		filind.push_back(ii);
	     }
    }
    if(!notfile)
    {
    bids.clear();
    if(!local) 
    {
	    for(auto f :filind)
	    {
		    string ff = to_string(f);
		    Bid bid(ff);
		    int sz = fcnt->find(bid);
			for(int i = 1;i<=sz;i++)
			{
				Bid fb = createBid(ff,i);
				bids.push_back(fb);
			}
	    }
    }
    else if(local)
    {
	    for(auto k :filind)
	    {
		string ff = to_string(k);
                int sz = localBCNT[k];
		for(int i = 1;i<=sz;i++)
		{
			Bid fb = createBid(ff,i);
			bids.push_back(fb);
		}
	    }
    }
    result=srch->batchSearch(bids); 
    }
    return result;
}
/*
vector<pair<string,string>> MOrion::searchsimple(string keyword)
{
    vector<pair<string,string>> result;
    vector<Bid> fileblocks; 
    Bid mapKey(keyword);
    int updc = fcnt->find(mapKey); 
    //int updc = fcntmap[mapKey]; 
    if (updc == 0) 
        return result;
    int pos = 1;
    int fetched = 0;
    int point = 0;
    while(fetched < updc) 
    {
    	    mapKey = createBid(keyword,pos);
            string ids = srch->find(mapKey).second;
	    //cout <<ids<<endl;
	    point = 0;
	    while(fetched < updc && point < COM)
	    {
	    	string str = ids.substr(point*FID_SIZE,FID_SIZE);
	    	point++;
	    	fetched++;
                string fID = to_string(stoI(str));
            	Bid bid(fID); 
                int sz = fcnt->find(bid);
                //int sz = fcntmap[bid];
		for(int i = 1;i<=sz;i++)
		{
			Bid fb = createBid(fID,i);
			fileblocks.push_back(fb);
		}
            }
	    pos++;
    }
    for(fbid:fileblocks)
    	result.push_back(srch->find(fbid)); 
    return result;
}
*/
void MOrion::insertWrapper(vector<string> kws,vector<string> blocks,int ind)
{ 
     cout << "inserting kw for fileid:"<< ind << endl;
     int i=0;
     for (auto kw: kws) 
     {
        //cout << "inserting :"<< kw << endl;
   	insertkw(kw,ind); // insert all keywords
     }
     cout << "inserted all the kw (total keywords: " <<kws.size() <<")"<< endl;
     insertFile(ind,blocks); // insert all file blocks
     inserted= inserted+kws.size();
     //cout << endl<<"--TOTAL keywords inserted so far: "<<inserted<<endl;
     cout <<"--TOTAL unique keywords inserted so far: "<<uniquekw<<endl;
     cout <<"--TOTAL file blocks:"<< totblocks<< endl;
     cout <<"--TOTAL srch blocks used:"<< (uniquekw+totblocks)<<endl;
}

void MOrion::insertkw(string keyword, int ind) 
{
    Bid mapKey(keyword);
    //int updc = fcnt->find(mapKey);
    int updc = fcntmap[mapKey];
    if (updc == 0) 
	 uniquekw++;  
    cout <<"updc:"<< updc;
    updc++;
    //fcnt->insert(mapKey,updc);
    fcntmap[mapKey]=updc;
    Bid updKey = createBid(keyword, ind);
    //updt->insert(updKey, updc);//pad, can we store number in updc 
    //updtmap[updKey]= updc;//pad, can we store number in updc 

    int pos_in_block = get_position(updc,COM);
    int block_num = get_block_num(updc,COM);
    Bid key = createBid(keyword, block_num);
    if(pos_in_block == 1)
    {
           string id = toS(ind);
           id.insert(FID_SIZE, BLOCK-FID_SIZE, '#'); // padding happpens
           srch->insert(key, id);
    }
    else if (pos_in_block >=2 && pos_in_block <=COM)
    {
          string oldblock = (srch->find(key));
          oldblock.replace((pos_in_block-1)*FID_SIZE,FID_SIZE,toS(ind));
	  srch->insert(key, oldblock);
    }
    else
    {
         cout << endl << "{pos_in_block greater than 8}" << endl;
    }
}


void MOrion::insertFile(int id, vector<string> blocks)
{
	string ind = toS(id);
	Bid mapKey(id);
	int sz = blocks.size();
        string par = ind;
	par.append(to_string(sz));// pad later to 64 byte
	srch->insert(mapKey, par);
	int i =1;
        for(auto block : blocks)
	{       
		Bid mk = createBid(to_string(id), i);
		srch->insert(mk,block);
		i++;
	}
cout << "inserted all blocks of (total:"<<blocks.size()+1 <<") of:"<<ind<<endl;
	totblocks= totblocks+blocks.size()+1;
}

/*
void MOrion::remove(int id) 
{
	string file = "";
	Bid blockcnt(id);
	string bcnt = srch->find(blockcnt).second;
	int bc = 0; 
	if(bcnt=="")
	{
		cout <<" File does NOT EXIST!"<<endl;
		return;
	}
	else 
	{
		bcnt=bcnt.substr(FID_SIZE,bcnt.size());
		bc = stoI(bcnt);
	}
        srch->remove(blockcnt);	
	Bid blk;
	for (int i = 1; i <= bc ; i++)
	{
		blk = createBid(to_string(id),i);
		string ret = srch->find(blk).second;
		ret=ret.substr(FID_SIZE,ret.size());
		file = file.append(ret);
		srch->remove(blk);	
	}
	cout <<"Removed "<< bc <<" blocks from file:"<<id<<endl;
	vector<string> kws1;
	cout <<file<<endl;
	boost::split(kws1, file, boost::is_any_of(delimiters));
	vector<string> kws = getUniquedWords(kws1);
	int i=0;
	for(auto kw : kws)
	{
		cout <<i<<"["<<kw<<"]"<<endl;
		i++;
	}
	removekw(kws, id);
	cout <<"kws.size:"<<kws.size()<<endl;
}

void MOrion::removekw(vector<string> kws, int ind) 
{
    for(auto keyword : kws)
    {
      Bid mapKey = createBid(keyword, ind);
      //int delcnt = updt->find(mapKey);
      int delcnt = updtmap[mapKey];
      if (delcnt!=0)
      {
	Bid firstKey(keyword);
        //int updc = fcnt->find(firstKey);
        int updc = fcntmap[firstKey];
	cout <<keyword<<":UPDC IN DELETE:"<< updc<<endl;
	if(updc==0)
		return;
	int bn = get_block_num(updc,COM);
        int pos = get_position(updc,COM);
	Bid lastKey = createBid(keyword,bn);
	string block = srch->find(lastKey).second;
	string lastid = block.substr((pos-1)*FID_SIZE,FID_SIZE);
	updc--;
  	if (updc > 0) 
    	{
    	    //fcnt->insert(firstKey,updc);
	    fcntmap[firstKey]=updc;
            if (updc + 1 != delcnt) 
            {
                Bid curKey = createBid(keyword, stoI(lastid)); 
                //updt->insert(curKey, delcnt);
                updtmap[curKey]= delcnt;
                int pos_in_blockdel = get_position(delcnt,COM);
                int block_del = get_block_num(delcnt, COM);
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
		//fcnt->remove(firstKey);
		fcntmap[firstKey]=0;
		srch->remove(lastKey);
       }
       //updt->remove(mapKey); // delete in updt
       updtmap[mapKey]=0; // delete in updt
    }
  }
}

*/
/*vector<pair<string,string>> MOrion::setupsearch(string keyword)
{
    vector<pair<string,string>> result;
    vector<pair <string,string>> counts;
    Bid mapKey(keyword);
    //int updc = fcnt->setupfind(mapKey); //0. get file count i.e. updc 
    int updc = fcntmap[mapKey]; //0. get file count i.e. updc 
    if (updc == 0) 
        return result;

    int pos = 1;
    mapKey = createBid(keyword,pos);
 pair<string,string> firstfids = srch->setupfind(mapKey); //1. first COM fileids
    int fetched = 0;
    vector<Bid> fileids; // fileids
    vector<Bid> fileblocks; // fileids
    int point = 0;
    while(fetched < updc && point < COM)
    {
	    string str = firstfids.second.substr(point*FID_SIZE,FID_SIZE);
            Bid bid(str); 
	    fileids.push_back(bid);
	    fetched++;
	    point++;
    }
    pos++;
    counts = srch->batchSearch(fileids); //2. block-size of first COM fileids
    for(auto id1 : counts)
    {
	string id = id1.second;
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
            	Bid bid(str); // for first file block
	    	fileids.push_back(bid);
	    	point++;
	    	fetched++;
	    }
	    pos++;
    }
    counts.clear();
    counts = srch->batchSearch(fileids);//3. block-size of rest of the files
    for(auto id1 : counts)
    {
	string id = id1.second;
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
*/

vector<string> MOrion::search(string keyword)
{
    vector<string> result;
    vector<string> counts;
    Bid mapKey(keyword);
    //int updc = fcnt->find(mapKey); //0. get file count i.e. updc 
    int updc = fcntmap[mapKey]; //0. get file count i.e. updc 
    if (updc == 0) 
        return result;
    cout <<"UPDC:"<< updc;
    int pos = 1;
    mapKey = createBid(keyword,pos);
    string freq = srch->find(mapKey); 
    int fetched = 0;
    vector<Bid> fileids; // fileids
    vector<Bid> fileblocks; // fileids
    int point = 0;
    /*while(fetched < updc && point < COM)
    {
	    string fid = freq.substr(point*FID_SIZE,FID_SIZE);
	    string fID = to_string(stoI(fid));
            Bid bid(fID); 
	    //fileids.push_back(bid);
	    fetched++;
	    point++;
    //}
    //counts = srch->batchSearch(fileids); //2. block-size of first COM fileids
    	    int sz = fcnt->find(bid);
    //for(auto id1 : counts)
    //{
	//string idd = id.substr(0,FID_SIZE);
	for(int i = 1;i<=sz;i++)
	{
		Bid fb = createBid(fID,i);
		fileblocks.push_back(fb);
	}
    }
    pos++;
    fileids.clear();*/
    while(fetched < updc) // get rest of ids
    {
    	    mapKey = createBid(keyword,pos);
            string freq = srch->find(mapKey);// 3. next COM fileids
	    point = 0;
	    while(fetched < updc && point < COM)
	    {
	    	string str = freq.substr(point*FID_SIZE,FID_SIZE);
            	Bid bid(stoI(str)); // for first file block
	    	//fileids.push_back(bid);
	    	point++;
	    	fetched++;
    //}
                //int sz = fcnt->find(bid);//3. block-size of rest of the files
                int sz = fcntmap[bid];//3. block-size of rest of the files
    //for(auto id1 : counts)
    //{
                int fID = stoI(str);
	for(int i = 1;i<=sz;i++)
	{
		Bid fb = createBid(to_string(fID),i);
		fileblocks.push_back(fb);
	}
        }
	    pos++;
    }
    for(fbid:fileblocks)
    	result.push_back(srch->find(fbid)); // 4. all file blocks
    // the client should identify the blocks of same files
    return result;
}
Bid MOrion::createBid(string keyword, int number) 
{
    Bid bid(keyword);
    auto arr = to_bytes(number);
    std::copy(arr.begin(), arr.end(), bid.id.end() - 4);
    return bid;
}

Bid MOrion::createBid(string keyword, string id) 
{
    Bid bid(keyword);
    std::copy(id.begin(), id.end(), bid.id.end() - id.size());
    return bid;
}


void MOrion::print()
{
	//updt->printTree();
	srch->printTree();
}
