#include "BOrion.h"
#include<string>
#include<math.h>
#include<queue>
#include <unistd.h>
#include<tuple>

BOrion::BOrion(bool usehdd, int maxSize) {
    this->useHDD = usehdd;
    bytes<Key> key1{0};
    bytes<Key> key2{1};
    srch = new OMAP(maxSize*4, key1);
    //updt = new OMAP(maxSize*4, key2);
}

BOrion::~BOrion() {
    delete srch;
    //delete updt;
}

void BOrion::insertWrapper(vector<string> kws, vector<string> blocks, string ind)
{
     
     for (auto kw: kws)
	     insert(kw,ind);
     insertFile(ind,blocks);
}

void BOrion::insertFile(string ind, vector<string> blocks)
{
	Bid mapKey = createBid(ind, 0);
	//Bid kwKey = createBid(ind, -1);
	int sz = blocks.size();
        pair <int, string> par;
	par.first = FS;
	par.second = ind;
	//par.second.append(" ");
	par.second.append(to_string(sz));
	srch->insert(mapKey, par);
	//srch->insert(kwKey,"text,keywords");
	int i =1;
        for(auto block : blocks)
	{       pair <int, string> pr;
		pr.first = FB;
		pr.second = block;
		Bid mapKey = createBid(ind, i);
		srch->insert(mapKey,pr);
		////cout << i <<"in if BLK["<<block <<"\n";
		i++;
	}

}

void BOrion::insert(string keyword, string ind) 
{
    //Bid mapKey = createBid(keyword, ind);
    //auto updt_cnt = updt->find(mapKey);
    Bid mapKey = createBid(keyword, 0);
    auto updt_cnt = (srch->find(mapKey)).second;

    //updt_cnt/16 will give us actual updc, 
    //because now we store 16 files in one block
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
       updc = updc+1;
       pair<int , string> par;
       par.first = KS;
       par.second = to_string(updc);
       srch->insert(mapKey, par);
       //cout << "updc value:"<< updc << endl;
       int pos_in_block = updc%COM; // 1,2,3,4, ..., 16
       if (pos_in_block == 0) pos_in_block = COM;
       float bnum = ceil(updc/COM); // 4 file-ids each block
      
       int block_num ;
       if(updc%COM==0)
	      block_num = bnum;
       else 
	       block_num = bnum+1;
       
       //cout <<"Keyword-" << keyword << endl;
       //cout << "block_num value:"<< block_num << endl;
       //cout << "pos_in_block value:"<< pos_in_block << endl;
       Bid key = createBid(keyword, block_num);
	
	if(updc%COM == 1)
	{
           //cout << "SIZEofIND:"<< ind.length() << endl;
	   ind.insert(pos_in_block*FID_SIZE,ID_SIZE-pos_in_block*FID_SIZE,'#');
	   pair <int, string> pr;
	   pr.first =KB;
	   pr.second = ind;
           srch->insert(key, pr);
	   //cout <<"NEWBLOCK:"<< ind <<endl;

	}
	else
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
	}
        //LastIND[keyword] = ind; // no need to keep this
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
    auto updt = srch->find(mapKey);
    string updt_cnt = updt.second;
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

/*
void BOrion::remove(string keyword, int ind) {
    Bid mapKey = createBid(keyword, ind);
    string updt_cnt = updt->find(mapKey);
    if (stoi(updt_cnt) > 0) {
        updt->insert(mapKey, to_string(-1));
        UpdtCnt[keyword]--;
        if (UpdtCnt[keyword] > 0) {
            if (UpdtCnt[keyword] + 1 != stoi(updt_cnt)) {
                Bid curKey = createBid(keyword, LastIND[keyword]);
                updt->insert(curKey, updt_cnt);
                Bid curKey2 = createBid(keyword, stoi(updt_cnt));
                srch->insert(curKey2, to_string(LastIND[keyword]));
            }
            Bid key = createBid(keyword, UpdtCnt[keyword]);
            string idstr = srch->find(key);
            int lastID = stoi(idstr);
            LastIND[keyword] = lastID;
        } else {
            LastIND.erase(keyword);
        }
    }
}
*/


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
	cout << "SEARCHING FOR:" << keyword << endl;
	srand(time(NULL));
	vector<pair<int,string>> result;
	map<string,string> files;
        int fetched=0, updc;
        int totOMAPacc = 0;
    vector<Bid> bids;
    Bid mapKey = createBid(keyword, 0);
    auto updt = srch->find(mapKey); // mandatory first search
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
    
    queue<Bid> fileids;
    queue<string> firstblockid;
    queue<string> firstblocknum;

    mapKey = createBid(keyword,1);
    auto freq = srch->find(mapKey); //mandatory second search
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
 
    int ran; // randomly select between 3 options
    int idblock = 2; // next index block to be fetched is block 2
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
		    int cnt = 0;
		    while(cnt<ran)
		    {
			    string str = firstblockid.front();
			    string numb = firstblocknum.front();
			    int num;
			    stringstream convstoi(numb);
			            convstoi >> num;
			    int bnums = 1;
			    while(bnums<=num)
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
	    result = srch->batchSearch(batch);
	    totOMAPacc = totOMAPacc+batch.size();
cout << "TOTAL OMAP ACCESS so far:"<<totOMAPacc<< endl;
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
			//cout << "SIZE of the File:"<< bnum << endl;
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
			   //cout << "CONT:"<< id << " :"<< con << endl;
			   //cout<<"------------------------------------"<<endl;
			}
			else
			{
				files.insert(pair<string,string>(id,bcontent));
			   //cout << "11CONT:"<< id << " :"<< bcontent << endl;
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
cout << "TOTAL OMAP ACCESS so far:"<<totOMAPacc<< endl;
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
		srch->batchSearch(batch);
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
		srch->batchSearch(batch);
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
		srch->batchSearch(batch);
		totOMAPacc = totOMAPacc+ran;
		cout << "IM dummy access at LARGE:"<<totOMAPacc<< endl;
	}
}
return files;
}


vector<pair<int,string>> BOrion::search(string keyword) 
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

        for (int i = 1; i <= updc; i++) {
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

