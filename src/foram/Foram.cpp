#include "Foram.h"
#include<string>
#include<math.h>
#include<queue>
#include <unistd.h>
#include<tuple>
#include <boost/algorithm/string.hpp>

int inserted = 0;

Foram::Foram(bool usehdd, int maxSize) {
    this->useHDD = usehdd;
    bytes<Key> key1{0};
    bytes<Key> key2{1};
    srch = new OMAP(maxSize*4, key1);
    updt = new OMAPf(maxSize*4, key1); // use key2
}

Foram::~Foram() {
    delete srch;
    delete updt;
}

void Foram::insertWrapper(vector<string> kws, vector<string> blocks, string ind)
{ 
     int totk=0;
     cout << "inserting kw for:"<< ind << endl;
     for (auto kw: kws) // cannot batchinsert as updtCount is at server
     {
    	cout << "[" << kw << "] :" << ind << endl;
   	insert(kw,ind); // insert all keywords
	totk++;
     }
	cout << "inserted all the kw (total keywords: " <<totk <<")"<< endl;

     insertFile(ind,blocks); // insert all file blocks

}

void Foram::insert(string keyword, string ind) 
{
    inserted++;
    Bid mapKey = createBid(keyword, FCNT);
    auto uc = (updt->find(mapKey));
    int updc; 
    if (uc == "") 
    {  
         updc=0;
    }
    else
    {
        stringstream convstoi(uc);
        convstoi >> updc;
    }
   
    updc = updc+1;
    updt->insert(mapKey, to_string(updc));
    Bid updKey = createBid(keyword, ind);
    updt->insert(updKey, to_string(updc));//pad, can we store number in updc 
    

    Bid key = createBid(keyword, updc);
    pair <int, string> pr;
    pr.first =KB;
    pr.second = ind; // pad later
    srch->insert(key, pr);


}

void Foram::insertFile(string ind, vector<string> blocks)
{
	pair <int, string> pr;
	int i =1;
        for(auto block : blocks)
	{       
		pr.first = FB;
		pr.second = block;
		Bid mk = createBid(ind, i);
		srch->insert(mk,pr);
		//batch.insert(make_pair(mk,pr)); // gives **ERROR
		i++;
	}
	pr.first = FB;
	pr.second = "";
	Bid mk = createBid(ind, blocks.size()+1);
	srch->insert(mk,pr);
	//srch->batchInsert(batch); // gives **ERROR
	cout << "inserted "<<  blocks.size() <<" blocks of " << ind << endl;
}


/**
 * This function executes an insert in setup mode. Indeed, it is not applied until endSetup()
 */

void Foram::setupInsert(string keyword, pair<int,string> ind) 
{
    //** NEED MODIFICATION
    Bid mapKey = createBid(keyword, 0);
    auto upd = srch->find(mapKey);
    string updt_cnt = upd.second;
    int updc;
    if (updt_cnt == "") {
         updc=0;
    }
    else
    {
        stringstream convstoi(updt_cnt);
        convstoi >> updc;
    }
        //cout << "updatecnt " << updt_cnt <<" updc:"  <<updc <<"\n" ;
        updc=updc+1;
        //Bid key = createBid(keyword, id);
        //setupPairs1[key]=to_string(updc);
        Bid key = createBid(keyword, updc);
        setupPairs2[key].first= ind.first;
	setupPairs2[key].second= ind.second;
    //LastIND[keyword] = ind;
}




string Foram::removefileblock(string ind, int blk) 
{
	Bid del = createBid(ind, blk);
	auto ret = (srch->find(del)).second;
	if (ret != "")
	{
	ret = ret.substr(FID_SIZE,ret.size());
	cout <<"ret is :" << ret << endl;
	pair <int, string> pr;
	pr.first = LAST;
	pr.second = "";
	srch->insert(del, pr);
	}
	return ret;

}

string Foram::searchfileblocknum(string ind) 
{
	Bid firs = createBid(ind, 0);
	auto ret = srch->find(firs).second;
	if(ret.size() > FID_SIZE)
	{
 		ret = ret.substr(FID_SIZE,ret.size());
		pair <int, string> pr;
		pr.first = LAST;
		pr.second = "";
		srch->insert(firs, pr);
	}
	return ret;
}

/*
void Foram::removekw(string keyword, string ind) 
{
    Bid mapKey = createBid(keyword, ind);
    string delcnt = updt->find(mapKey);
    if (delcnt!="")
    {
    stringstream convstoi(delcnt);
    int del_cnt;
    convstoi >> del_cnt;
    cout << "DELCNT:"<< del_cnt << endl;
    
    Bid lastKey = createBid(keyword,LAST);
    auto last = srch->find(lastKey);
    cout << " last id:[" << last.second<<"]" << endl;
    if (del_cnt > 0) 
    {
        //updt->insert(mapKey, to_string(LAST)); 
	updt->remove(mapKey); // delete in updt
	Bid mk = createBid(keyword,0);
	string updtcnt = srch->find(mk).second;
        stringstream convstoi(updtcnt);
        int updt_cnt;
        convstoi >> updt_cnt;
	int updc = updt_cnt;
	updc--;
	srch->insert(mk,make_pair(KS,to_string(updc)));
        
	if (updc > 0) 
	{
        	if (updc + 1 != del_cnt) 
      		{
            		Bid curKey = createBid(keyword, last.second); 
            		updt->insert(curKey, delcnt);
       			int pos_in_blockdel = get_position(del_cnt,COM);
       			int block_del = get_block_num(del_cnt, COM);

	                Bid cur = createBid(keyword, block_del);
        	        auto bl_del = srch->find(cur);
			string blsec;

		        int pos_in_block = get_position(updt_cnt,COM);
       			int block_num = get_block_num(updt_cnt, COM);

	                Bid cur2 = createBid(keyword, block_num);
        	        auto bl = srch->find(cur2);
	    		string block = bl.second;


	    cout << "Before[" << block <<"]"<< endl << endl ; 
	    string st = block.substr((pos_in_block-1)*FID_SIZE,FID_SIZE);
	    block.replace((pos_in_block-1)*FID_SIZE,FID_SIZE,"####");
	    bl.second = block;
	    cout << "After[" << block <<"]"<< endl << endl ;
	    srch->insert(cur2,bl); // cur

		if(block_num == block_del)
			blsec = block;
		else
       			blsec = bl_del.second;
	    cout << "BEFORE[" << blsec <<"]"<< endl << endl ; 
            blsec.replace((pos_in_blockdel-1)*FID_SIZE,FID_SIZE,st);
	    bl_del.second = blsec;
	    cout << "AFTER[" << blsec <<"]"<< endl << endl ;
	    srch->insert(cur,bl_del);
            }
      else
      {
	       int pos_in_block = get_position(updt_cnt,COM);
       	       int block_num = get_block_num(updt_cnt, COM);

                Bid cur2 = createBid(keyword, block_num);
                auto bl = srch->find(cur2);
	    	string block = bl.second;

	        block.replace((pos_in_block-1)*FID_SIZE,FID_SIZE,"####");
	        bl.second = block;
	        cout << "LAST after[" << block <<"]"<< endl << endl;
	        srch->insert(cur2,bl);
      }
        int pos_in_block = get_position(updc,COM);
	int block_num = get_block_num(updc, COM);
            
	Bid key = createBid(keyword, block_num);
	string lastidblock = (srch->find(key)).second;
	string lastid = lastidblock.substr((pos_in_block-1)*FID_SIZE,FID_SIZE);
        pair <int, string> lst;
	
	lst.first = LAST; // -1
	lst.second = lastid;
	srch->insert(lastKey, lst);
        } 
	else 
	{
		cout << "MAKING last empty" << endl;
		pair <int, string> lst;
		lst.first = LAST;
		lst.second = "";
	        srch->insert(lastKey, lst); // or -1 ?
        }
    }
    }
}
*/


/**
 * This function executes a remove in setup mode. Indeed, it is not applied until endSetup()
  */
/*
void Foram::setupRemove(string keyword, int ind) {
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
*/

map<string,string> Foram::search(string keyword)
{
    vector<pair<int,string>> result;
    int updc;
    vector<Bid> bids;
    map<string,string> files;

    Bid mapKey = createBid(keyword, FCNT);
    auto updt_cnt = updt->find(mapKey);

    if (updt_cnt != "") 
    {
        stringstream convstoi(updt_cnt);
        convstoi >> updc;
    }
    else
    {
	    return files;
    }

    vector<string> ids;
    
    for(int id = 1; id <= updc; id++)
    {
	Bid fileid = createBid(keyword,id);
	auto ret = (srch->find(fileid)).second;
	ids.push_back(ret);

    } 
    
    for(auto id : ids)
    {
	int j = 1;
	Bid block = createBid(id,j);
        string cont = (srch->find(block)).second;
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
		cont = (srch->find(block)).second;
	}
    }
    return files;
}



pair<int,string> Foram::searchfileblock(string ind, int blk) 
{
    Bid mapKey = createBid(ind, blk);
    auto result = srch->find(mapKey);
    return result;
}


vector<pair<int,string>> Foram::searchkw(string keyword) 
{
    vector<pair<int,string>> result;
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


