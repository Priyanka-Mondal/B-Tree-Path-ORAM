#include "BOrion.h"
#include<string>
#include<math.h>
#include<queue>
#include <unistd.h>
#include<tuple>
#include <boost/algorithm/string.hpp>

int inserted = 0;
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
BOrion::BOrion(bool usehdd, int maxSize) {
    this->useHDD = usehdd;
    bytes<Key> key1{0};
    bytes<Key> key2{1};
    srch = new OMAP(maxSize*4, key1);
    updt = new OMAPf(maxSize*4, key1); // use key2
}

BOrion::~BOrion() {
    delete srch;
    delete updt;
}

void BOrion::insertWrapper(vector<string> kws, vector<string> blocks, string ind)
{ int totk=0;
	cout << "inserting kw " << endl;
     for (auto kw: kws) // cannot batchinsert as updtCount is at server
     {
    	cout << "[" << kw << "]]" << endl;
   	insert(kw,ind); // insert all keywords
	totk++;
     }
	cout << "inserted all the  kw totk: " <<totk<< endl;

     // so add some fake accesses
     // Will uncomment later
     /*
     int ran = rand()%(kws.size())+ceil(0.5*kws.size());
     int cnt = 0;
     while(cnt < ran)  
     {
	Bid bid = createBid("dummy",cnt);
	//cout <<"ran:" << ran << "cnt:" << cnt;
	srch->find(bid);
	cnt++;
     }*/
     insertFile(ind,blocks); // insert all file blocks

}

void BOrion::insert(string keyword, string ind) 
{
	inserted++;
    //Bid mapKey = createBid(keyword, ind);
    //auto updt_cnt = updt->find(mapKey);
  //  cout << "[at insert 1:" << inserted <<"]";
    Bid mapKey = createBid(keyword, 0);
    auto uc = (srch->find(mapKey));
    //cout << "got uc 2" << endl;
    auto updt_cnt = uc.second;
    //cout << "got updt_cnt 3" << endl;
    //updt_cnt/16 will give us actual updc, 
    //because now we store 16 files in one block
    int updc; 
    if (updt_cnt == "") {  
         updc=0;
    }
    else
    {
        //cout << "got convstoi 3.5[" << updt_cnt <<"]"<< endl;
        stringstream convstoi(updt_cnt);
        //cout << "got convstoi 3.6[" << updt_cnt <<"]"<< endl;
        convstoi >> updc;
        //cout << "got updc 4[" <<updc<<"]"<< endl;
    }
       updc = updc+1;
       pair<int , string> par;
       par.first = KS;
       par.second = to_string(updc); // pad here
       srch->insert(mapKey, par);
       
       Bid lastKey = createBid(keyword, LAST); // last
       par.first = LAST;
       par.second = ind;  // last // pad later
       srch->insert(lastKey,par); //last

       Bid updKey = createBid(keyword, ind);
       updt->insert(updKey, to_string(updc));//pad, can we store number in updc 
       //string test = updt->find(updKey);

       //cout << "inserted updc 5" << endl;
       //cout << "updc value:"<< updc << endl;
       //int pos_in_block = updc%COM; // 1,2,3,4, ..., 16
       //if (pos_in_block == 0) pos_in_block = COM;
	int pos_in_block = get_position(updc,COM);
       //float bnum = ceil(updc/COM); // 16 file-ids each block
       //int block_num ;
       //if(updc%COM == 0)
//	      block_num = bnum;
 //      else 
//	      block_num = bnum+1;
	int block_num = get_block_num(updc,COM);
       
       //cout <<"Keyword-" << keyword << endl;
       //cout << "block_num value:"<< block_num << endl;
       //cout << "pos_in_block value:"<< pos_in_block << endl;
       Bid key = createBid(keyword, block_num);
	
	//if(updc%COM == 1)
	if(pos_in_block == 1)
	{
           //cout << "SIZEofIND:"<< ind.length() << endl;
	//ind.insert(pos_in_block*FID_SIZE, ID_SIZE-pos_in_block*FID_SIZE, '#');
	ind.insert(FID_SIZE, ID_SIZE-FID_SIZE, '#'); // padding happpens
	   pair <int, string> pr;
	   pr.first =KB;
	   pr.second = ind; // pad later
           srch->insert(key, pr);
	   //cout <<"NEWBLOCK:"<< ind <<endl;
        //cout <<" updc:"  <<updc <<"::"<< ind <<"\n" ;

	}
	else if (pos_in_block >=2 && pos_in_block <=16)
	{
	    string oldblock = (srch->find(key)).second;
            //cout << "SIZEofOLDIND:"<< oldblock.length() << endl;
	    //cout <<"OLDBLOCK:" << oldblock << endl;
	    oldblock.replace((pos_in_block-1)*FID_SIZE,FID_SIZE,ind);
	    //cout <<"OLDNEWBLOCK:" << oldblock << endl;
	     pair <int, string> pr;
	     pr.first =KB;
	     pr.second = oldblock;
	     srch->insert(key, pr);
        //cout <<" updc:"  <<updc <<"::"<< oldblock <<"\n" ;
	}
       else
       {
	   cout << endl << "{pos_in_block greater than 16}" << endl;
       }
        //LastIND[keyword] = ind; // no need to keep this
}

void BOrion::insertFile(string ind, vector<string> blocks)
{
	//cout << "at insertfile:" << ind << endl;
	Bid mapKey = createBid(ind, 0);
	map<Bid,pair<int,string>> batch;
	int sz = blocks.size();
        pair <int, string> par;
	par.first = FS;
	par.second = ind; 
	//par.second.append(" ");
	par.second.append(to_string(sz));// pad later to 64 byte
	srch->insert(mapKey, par);
	//srch->insert(kwKey,"text,keywords");
	int i =1;
	/*cout << "about to insert blockss" << endl;
	for(auto block : blocks)
	{
		cout << "block printing[" << block <<"]" << endl;
	}*/
        for(auto block : blocks)
	{       pair <int, string> pr;
		pr.first = FB;
		pr.second = block;
		Bid mk = createBid(ind, i);
		//cout << "Block inserteD:[[" << block <<  "]]"<< endl <<endl ;
		srch->insert(mk,pr);
		//batch.insert(make_pair(mk,pr));
		////cout << i <<"in if BLK["<<block <<"\n";
		i++;
	}
	// no need for fake blocks as batchInsert calls 
	// treeHandler->finishOperation at the end to pad 
	// 4.35 * (depth of AVLTree) times
	//srch->batchInsert(batch);
	cout << "inserted blocks" << endl;
}


/**
 * This function executes an insert in setup mode. Indeed, it is not applied until endSetup()
 */

void BOrion::setupInsert(string keyword, pair<int,string> ind) {
// are we using it anywhere ?
/* 
    Bid mapKey = createBid(keyword, ind);

    if (UpdtCnt.count(keyword) == 0) {
        UpdtCnt[keyword] = 0;
    }
    UpdtCnt[keyword]++;
    setupPairs1[mapKey]=to_string(UpdtCnt[keyword]);
    Bid key = createBid(keyword, UpdtCnt[keyword]);
*/
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





string BOrion::removefileblock(string ind, int blk) 
{
	Bid del = createBid(ind, blk);
	auto ret = (srch->find(del)).second;
	ret = ret.substr(4,ret.size());
	pair <int, string> pr;
	pr.first = -1;
	pr.second = "";
	srch->insert(del, pr);
	return ret;

}

string BOrion::searchfileblocknum(string ind) 
{
	Bid firs = createBid(ind, 0);
	auto ret = srch->find(firs).second;
	if(ret.size() > 4)
	{
	ret = ret.substr(4,ret.size());
	cout << "num bloc:" << ret << endl;
	pair <int, string> pr;
	pr.first = -1;
	pr.second = "";
	srch->insert(firs, pr);
	}
	return ret;

}
void BOrion::removekw(string keyword, string ind) 
{
    Bid mapKey = createBid(keyword, ind);
    string delcnt = updt->find(mapKey);
    stringstream convstoi(delcnt);
    int del_cnt;
    convstoi >> del_cnt;
    cout << "DELCNT:"<< del_cnt << endl;
    
    Bid lastKey = createBid(keyword,LAST);
    auto last = srch->find(lastKey);
    cout << "DELCNT::"<< del_cnt <<" last.sec [" << last.second<<"]" << endl;
    if (del_cnt > 0) {
        updt->insert(mapKey, to_string(LAST)); 
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
            Bid curKey = createBid(keyword, last.second); // retrieve (last)
            updt->insert(curKey, delcnt);

    //cout << "DELCNT::"<< del_cnt <<" last.sec [" << last.second<<"]" << endl;
       
       //int pos_in_blockdel = del_cnt%COM; // 1,2,3,4, ..., 16
       //if (pos_in_blockdel == 0) pos_in_blockdel = COM;
       int pos_in_blockdel = get_position(del_cnt,COM);
       int block_del = get_block_num(del_cnt, COM);
//       float bnum = ceil(del_cnt/COM); // 16 file-ids each block
//       int block_del ;
//       if(del_cnt%COM == 0)
//	      block_del = bnum;
//       else 
//	      block_del = bnum+1;

                Bid cur = createBid(keyword, block_del);
                auto bl_del = srch->find(cur);
		string blsec;

       int pos_in_block = get_position(updt_cnt,COM);
       int block_num = get_block_num(updt_cnt, COM);
//       int pos_in_block = updt_cnt%COM; // 1,2,3,4, ..., 16
//       if (pos_in_block == 0) pos_in_block = COM;
//       bnum = ceil(updt_cnt/COM); // 16 file-ids each block
//       int block_num ;
//       if(updt_cnt%COM == 0)
//	      block_num = bnum;
//       else 
//	      block_num = bnum+1;

                Bid cur2 = createBid(keyword, block_num);
                auto bl = srch->find(cur2);
	    	string block = bl.second;


	    cout << "BLOCKBe[" << block <<"]"<< endl << endl ; 
	    string st = block.substr((pos_in_block-1)*FID_SIZE,FID_SIZE);
	    block.replace((pos_in_block-1)*FID_SIZE,FID_SIZE,"####");
	    bl.second = block;
	    cout << "BLOCKAf[" << block <<"]"<< endl << endl ;
	    srch->insert(cur2,bl); // cur

		if(block_num == block_del)
			blsec = block;
		else
       			blsec = bl_del.second;
	    cout << "BLOCKBE[" << blsec <<"]"<< endl << endl ; 
            blsec.replace((pos_in_blockdel-1)*FID_SIZE,FID_SIZE,st);
	    bl_del.second = blsec;
	    cout << "BLOCKAF[" << blsec <<"]"<< endl << endl ;
	    srch->insert(cur,bl_del);
            }
      else
      {
	      cout << "IN ELLSE" << endl << endl ;
       int pos_in_block = get_position(updt_cnt,COM);
       int block_num = get_block_num(updt_cnt, COM);
//       int pos_in_block = updt_cnt%COM; // 1,2,3,4, ..., 16
//       if (pos_in_block == 0) pos_in_block = COM;
//       int bnum = ceil(updt_cnt/COM); // 16 file-ids each block
//       int block_num ;
//       if(updt_cnt%COM == 0)
//	      block_num = bnum;
//       else 
//	      block_num = bnum+1;

                Bid cur2 = createBid(keyword, block_num);
                auto bl = srch->find(cur2);
	    	string block = bl.second;

	    block.replace((pos_in_block-1)*FID_SIZE,FID_SIZE,"####");
	    bl.second = block;
	    cout << "BLOLAa[" << block <<"]"<< endl << endl;
	    srch->insert(cur2,bl);
      }
       int pos_in_block = get_position(updc,COM);
       int block_num = get_block_num(updc, COM);
//       int pos_in_block = updc%COM; // 1,2,3,4, ..., 16
//       if (pos_in_block == 0) pos_in_block = COM;
//       int bnum = ceil(updc/COM); // 16 file-ids each block
//       int block_num ;
//       if(updc%COM == 0)
//	      block_num = bnum;
//       else 
//	      block_num = bnum+1;
            
	    Bid key = createBid(keyword, block_num);
            string lastidblock = (srch->find(key)).second;
	    //get the id of pos_in_block;
 string lastid = lastidblock.substr((pos_in_block-1)*FID_SIZE,FID_SIZE);
            pair <int, string> lst;
	    lst.first = LAST; // -1
	    lst.second = lastid;
	    srch->insert(lastKey, lst);
	    // delete the last entry of the last block
	    //
        } else {
		cout << "MAKING last empty" << endl;
		pair <int, string> lst;
		lst.first = LAST;
		lst.second = "";
	    srch->insert(lastKey, lst); // or -1 ?
        }
    }
}



/**
 * This function executes a remove in setup mode. Indeed, it is not applied until endSetup()
  */
/*
void BOrion::setupRemove(string keyword, int ind) {
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

map<string,string> BOrion::searchWrapper(string keyword)
{
	//cout << "SEARCHING FOR:" << keyword << endl;
	srand(time(NULL));
	vector<pair<int,string>> result;
	map<string,string> files;
        int fetched=0, updc;
        int totOMAPacc = 0;
    	vector<Bid> bids;
    	Bid mapKey = createBid(keyword, 0);

    
    auto updt = srch->find(mapKey); // mandatory first search: #fileids


    totOMAPacc++;
    string updt_cnt = updt.second;
    if (updt_cnt != "") {
        stringstream convstoi(updt_cnt);
        convstoi >> updc;
    }
    else
    {
	    return files;
    }
    
    queue<Bid> fileids; // fileids
    queue<string> firstblockid; //ids of files
    queue<string> firstblocknum;//#blocks of files
    mapKey = createBid(keyword,1);
    
    
    auto freq = srch->find(mapKey); //mandatory second search: first 16 fileids
    
    
    totOMAPacc++;
    auto fileids1 = freq.second;  // get first <=16 fileids
    int pos = 0;
    while(fetched < updc && fetched < COM)
    {
	    string str = freq.second.substr(pos*4,4);
	    //cout <<"the index:["<<str<<"]"<<endl<<endl;
            Bid bid = createBid(str,0); // for first file block
	    fileids.push(bid);
	    pos++;
	    fetched++;
    }
    //cout <<"fetched at the end:" << fetched <<endl;
    int ran; // random number
    int idblock = 2; // next index block to be fetched is block 2
    
    
    //randomize next OMAP accesses 
    while(fetched < updc || fileids.size()>0 || firstblockid.size()>0)
    {
	    vector<Bid> batch;
	    if(fetched < updc)
	    {
		int idblksleft = ceil((updc-fetched)/COM);
		if ((updc-fetched)%COM != 0) idblksleft++;
		//cout << "idblksleft:["<<idblksleft<<"]"<<endl;
		
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
			batch.push_back(fileids.front());
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
			    int num;
			    stringstream convstoi(numb);
			            convstoi >> num;
		//cout << endl <<"****num:***" << num << endl;
			    int bnums = 1;
			    while(bnums<=num) // all blocks of file fetched
			    {
			        Bid bid = createBid(str,bnums);
				bnums++;
			        batch.push_back(bid);
			//cout << endl <<"****bnum:***" << bnums << endl;
			    }
			    cnt++;
			    firstblockid.pop();
			    firstblocknum.pop();
		    }
	    }
	    result = srch->batchSearch(batch);
	    //totOMAPacc = totOMAPacc+batch.size();
//cout << "TOTAL OMAP ACCESS so far::"<<totOMAPacc<< endl;
	    for(auto blk : result)
	    {
	       //cout << "first:" << blk.first << "second:" <<blk.second <<endl;
	        if(blk.first==1)
		{
			int cnt = 0;
			while(cnt < COM && fetched<updc)
			{
			string str = blk.second.substr(cnt*4,4);
				Bid bid = createBid(str,0);
				fileids.push(bid);
				fetched++;
				cnt++;
			}
		}
		
		if(blk.first==2)
		{
			string id = blk.second.substr(0,4);
			int sz = blk.second.length(); // change to :until #
			string bnum = blk.second.substr(4,sz);
		//cout << "SIZE of File: "<< id << " IS:"<< bnum << endl;
			firstblocknum.push(bnum);
			firstblockid.push(id);
		}
		if(blk.first==3)
		{
			string id = blk.second.substr(0,4);
			int sz = blk.second.length();
			string bcontent = blk.second.substr(4,sz);
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

//for(auto itr = files.begin(); itr!=files.end();itr++)
//{
//	cout << "FIRST:"<< itr->first<< " SECOND:" << itr->second <<endl;
//	cout << "--------------------------------" << endl;
//}
//cout << "TOTAL OMAP ACCESS so far:"<<totOMAPacc<< endl;
//Do we need the following padding ?
//As batchSearch already pads with (depth * 1.45) fake reads
/*
if(totOMAPacc<SMALL)
{
	while(totOMAPacc<SMALL)
	{
		vector<Bid> batch;
		ran = rand()% (SMALL - totOMAPacc)+COM; //at least 16 ?
		int cnt = 0;
		while(cnt<ran)
		{
			Bid b = createBid("dummy",cnt);
			cnt++;
			batch.push_back(b);
		}
		auto fake = srch->batchSearch(batch);
		for (auto fak : fake)
		{ 
		//	if(fak.second != NULL)
			{
			cout << "FAKE:"<< cnt << " :" << fak.second <<endl;
			}
		//	else
		//	{
		//cout << "There no entry for (dummy," << cnt <<")" << endl;
		//	}

		}

		totOMAPacc = totOMAPacc+ran;
		cout << "IM dummy access at SMALL:"<<totOMAPacc<< endl;
	}
}
else if(totOMAPacc<MEDIUM)
{
	while(totOMAPacc<MEDIUM)
	{
		vector<Bid> batch;
		ran = rand()% (SMALL - totOMAPacc)+COM; //at least 16 ?
		int cnt = 0;
		while(cnt<ran)
		{
			Bid b = createBid("dummy",cnt);
			cnt++;
			batch.push_back(b);
		}
		auto fake = srch->batchSearch(batch);
		cout << "Size of fake:" << fake.size();
		for (auto fak : fake)
		{ 
		//	if(fak.second != NULL)
			{
			cout << "FAKE:"<< cnt << " :" << fak.second <<endl;

			}
		}
		totOMAPacc = totOMAPacc+ran;
		cout << "IM dummy access at MEDIUM:"<<totOMAPacc<< endl;
	}
}
else // if totOMAPacc > LARGE already then no dummy access done
{
	while(totOMAPacc<LARGE)
	{
		vector<Bid> batch;
		ran = rand()% (SMALL - totOMAPacc)+COM; //at least 16 ?
		int cnt = 0;
		while(cnt<ran)
		{
			Bid b = createBid("dummy",cnt);
			cnt++;
			batch.push_back(b);
		}
		//TODO: shuffle entries of batch
		auto fake = srch->batchSearch(batch);
		totOMAPacc = totOMAPacc+ran;
		cout << "IM dummy access at LARGE:"<<totOMAPacc<< endl;
	}
}*/
return files;
}


pair<int,string> BOrion::searchfileblock(string ind, int blk) 
{
    Bid mapKey = createBid(ind, blk);
    auto result = srch->find(mapKey);
    return result;
}


vector<pair<int,string>> BOrion::searchkw(string keyword) 
{
    vector<pair<int,string>> result;
    vector<Bid> bids;
    Bid mapKey = createBid(keyword, 0);
    auto updt = srch->find(mapKey);
    string updt_cnt = updt.second;
    int updc;

    if (updt_cnt != "") {
        stringstream convstoi(updt_cnt);
        convstoi >> updc;
	int block_num = get_block_num(updc,COM);

        for (int i = 1; i <= block_num; i++) {  // 
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
void BOrion::beginSetup() {
    setupPairs1.clear();
    setupPairs2.clear();
}

/**
 * This function is used for finishing setup of scheme because normal update is time consuming
 */
void BOrion::endSetup() {
    //updt->batchInsert(setupPairs1);
    srch->batchInsert(setupPairs2);
}

Bid BOrion::createBid(string keyword, int number) {
    Bid bid(keyword);
    auto arr = to_bytes(number);
    std::copy(arr.begin(), arr.end(), bid.id.end() - 4);
    //cout << "AT createbid:" << number <<"::"<<bid << endl;
    return bid;
}

Bid BOrion::createBid(string keyword, string id) {
    Bid bid(keyword);
    //auto arr = to_bytes(id);
    //cout << "size of ras-id:" << id.size() << endl;
    std::copy(id.begin(), id.end(), bid.id.end() - id.size());
    //cout << "AT createbid:" << number <<"::"<<bid << endl;
    return bid;
}


