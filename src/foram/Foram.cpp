#include "Foram.h"
#include<string>
#include<math.h>
#include<queue>
#include <unistd.h>
#include<tuple>
#include <boost/algorithm/string.hpp>

int inserted = 0;

int to_Int(string s)
{
	stringstream convstoi(s);
	int ret;
	convstoi >> ret;
	return ret;
}


Foram::Foram(bool usehdd, int maxSize, int updSize) 
{
    this->useHDD = usehdd;
    bytes<Key> key1{0};
    bytes<Key> key2{1};
    srch = new OMAP(maxSize*4, key1);
    updt = new OMAPf(updSize*4, key2);
}

Foram::~Foram() {
    delete srch;
    delete updt;
}

void Foram::insert(vector<string> kws, vector<string> blocks, string ind)
{ 
     inserted =0;
//INSERTING KEYWORDS*******************************
     cout << "inserting keywords for:"<< ind << endl;
     map<string,int> U;
     for(auto kw: kws)
     {
        Bid key = createBid(kw, FCNT);
        auto uc = (updt->find(key));
        int updc; 
    
        if (uc == "") 
             updc=0;
        else
	    updc = to_Int(uc);
       
        updc = updc+1;
        updt->insert(key, to_string(updc));
	cout << "Inserting:"<< key << endl;
	U.insert(pair<string,int>(kw,updc));
        Bid updKey = createBid(kw, ind);
        updt->insert(updKey, to_string(updc));//pad,can we store number in updc 
	cout << "Inserting:"<< updKey <<":"<< to_string(updc)<< endl;
     }
     //updt->finalize(x);
     
     for (auto kw: kws) 
     {
        Bid key = createBid(kw, U.at(kw));
        string pr = ind; // pad later
        srch->insert(key, pr);
        inserted++;
     }
     //srch->finalize(x);
     cout << "inserted all the keywords(total keyword(s):"<<inserted<<")"<<endl;

//INSERTING FILES************************************************
     string blk;
     int i =1;
     for(auto block : blocks)
     {       
         blk = block;
   	 Bid mk = createBid(ind, i);
   	 srch->insert(mk,blk);
   	 i++;
     }
     blk = "";
     Bid mk = createBid(ind, blocks.size()+1);
     srch->insert(mk,blk);
     cout << "inserted "<<  blocks.size() <<" block(s) of " << ind << endl;
}



void Foram::setupInsert(vector <string> kws, vector<string> blocks, string ind) 
{
     inserted =0;
//INSERTING KEYWORDS*******************************
     map<Bid,string> batchUpd1;
     map<Bid,string> batchUpd2;
     cout << "inserting keywords for:"<< ind << endl;
     map<string,int> U;
     for(auto kw: kws)
     {
        Bid key = createBid(kw, FCNT);
        auto uc = (updt->find(key));
        int updc; 
    
        if (uc == "") 
             updc = 0;
        else
      	     updc = to_Int(uc);

        updc = updc+1;
	batchUpd1.insert(pair<Bid, string>(key,to_string(updc)));
        Bid updKey = createBid(kw, ind);
        batchUpd2.insert(pair<Bid, string>(updKey, to_string(updc)));//pad 
	U.insert(pair<string,int>(kw,updc));
     }
     updt->batchInsert(batchUpd1);
     updt->batchInsert(batchUpd2);
     //UNCOMMENT LATER
     //updt->finalize(x);

     map<Bid,string> batchSrchInsert;     
     for (auto kw: kws) 
     {
    	cout << "[" << kw << "] :" << ind << endl;
        Bid srcKey = createBid(kw, U.at(kw));
	batchSrchInsert.insert(pair<Bid, string>(srcKey,ind));
        inserted++;
     }
     srch->batchInsert(batchSrchInsert);
     cout << "inserted all the keywords(total keyword(s):"<<inserted<<")"<< endl;

//INSERTING FILES BLOCKS IN BATCH****************************************
     map<Bid,string> batchFileBlocks;
     int i =1;
     Bid fb;
     for(auto block : blocks)
     {       
   	 fb = createBid(ind, i);
	 batchFileBlocks.insert(pair<Bid,string>(fb,block));
   	 i++;
     }
     string blk = "";
     fb = createBid(ind, blocks.size()+1);
     batchFileBlocks.insert(pair<Bid,string>(fb,blk));
     srch->batchInsert(batchFileBlocks); 
     //srch->finalize(x);
     cout << "inserted "<<  blocks.size() <<" block(s) of " << ind << endl;
    updt->printTree();
}



string Foram::removefileblock(string ind) 
{
	updt->printTree();
	cout <<"-------------------------------------" << endl;
	srch->printTree();
	cout <<"++++++++++++++++++++++++++++++++++++++" << endl;
	string file = "";
	int blk = 1;
	Bid del = createBid(ind, blk);
	string ret = (srch->find(del));
	while (ret != "")
	{
		file = file.append(ret.substr(FID_SIZE,ret.size()));
		blk++;
		del = createBid(ind, blk);
		ret = srch->find(del);
	}
	for(int i = 1; i<=blk ; i++)
	{
		del= createBid(ind,i);
		srch->insert(del,"");
		//srch->remove(del);	
	}
	return file;
}



void Foram::removekw(vector <string> kws, string ind) 
{
	for(auto kw : kws)
	{
    		Bid delKey = createBid(kw, ind);
		cout << "removing delkey:["<< delKey <<"]"<< endl;
    		string delcnt = updt->find(delKey);
		if(delcnt != "")
		{
	    	int del_cnt = to_Int(delcnt);
	    	
		Bid fcntKey = createBid(kw,FCNT);
		string fcnt = updt->find(fcntKey);
	    	int updc = to_Int(fcnt);
		int newfilecnt = updc-1;
		updt->insert(fcntKey,to_string(newfilecnt));

		Bid lastKey = createBid(kw,updc);
		string last_id = srch->find(lastKey);
		if(del_cnt != updc)
		{
			Bid delkwKey = createBid(kw,del_cnt);
			srch->insert(delkwKey, last_id);
			Bid lastupdKey = createBid(kw,last_id);
			updt->insert(lastupdKey,delcnt);
		}
		//	srch->remove(lastKey);
		cout <<"REMOVED||"<<endl;
		updt->removenaive(delKey);
		updt->printTree();
		}
	}
	//updt->finalize();
	//srch->finalize();
}



map<string,string> Foram::search(string keyword)
{
    vector<string> result;
    int updc;
    vector<Bid> bids;
    map<string,string> files;

    Bid mapKey = createBid(keyword, FCNT);
    auto updt_cnt = updt->find(mapKey);

    if (updt_cnt != "") 
    {
	updc = to_Int(updt_cnt);
	//cout <<"updc is not NULL "<< endl;
    }
    else
    {
	//cout <<"updc is NULL "<< endl;
	    return files;
    }

    vector<string> ids;
    
    for(int id = 1; id <= updc; id++)
    {
	Bid fileid = createBid(keyword,id);
	auto ret = (srch->find(fileid));
	ids.push_back(ret);

    } 
    
    for(auto id : ids)
    {
	int j = 1;
	Bid block = createBid(id,j);
        string cont = (srch->find(block));
	while(cont != "")
	{
		if(files.find(id)!=files.end())
		{
			string con = files.at(id);
			con.append(cont);
			files.erase(id);
			files.insert(pair<string,string>(id,con));
		}
		else
		{
			files.insert(pair<string,string>(id,cont));
		}
		j++;
		block = createBid(id,j);
		//cout << "blocknum j:" << j << endl;
		cont = (srch->find(block));
	}
    }
    return files;
}


string Foram::searchfileblock(string ind, int blk) 
{
    Bid mapKey = createBid(ind, blk);
    auto result = srch->find(mapKey);
    return result;
}


vector<string> Foram::searchkw(string keyword) 
{
    vector<string> result;
    vector<Bid> bids;
    Bid mapKey = createBid(keyword, FCNT);
    auto updt_cnt = updt->find(mapKey);
    int updc;

    if (updt_cnt != "") 
    {
        stringstream convstoi(updt_cnt);
        convstoi >> updc;

        for (int i = 1; i <= updc; i++) 
	{   
            Bid bid = createBid(keyword, i);
            bids.push_back(bid);
        }
    }
    else
    {
	    return result;
    }

    auto tmpRes = srch->batchSearch(bids);
    for(auto item:tmpRes){
        result.push_back(item);
    }
    return result;
}

/**
 * This function is used for initial setup of scheme because normal update is time consuming
 */
void Foram::beginSetup() 
{
    setupPairs1.clear();
    setupPairs2.clear();
}

/**
 * This function is used for finishing setup of scheme because normal update is time consuming
 */
void Foram::endSetup() {
    //updt->batchInsert(setupPairs1);
    srch->batchInsert(setupPairs2);
}

Bid Foram::createBid(string keyword, int number) 
{
    Bid bid(keyword);
    auto arr = to_bytes(number);
    std::copy(arr.begin(), arr.end(), bid.id.end() - 4);
    return bid;
}

Bid Foram::createBid(string keyword, string id) 
{
    Bid bid(keyword);
    std::copy(id.begin(), id.end(), bid.id.end() - id.size());
    return bid;
}


