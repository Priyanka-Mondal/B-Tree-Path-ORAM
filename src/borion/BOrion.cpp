#include "BOrion.h"
#include<string>
#include<math.h>

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
	srch->insert(mapKey, to_string(sz));
	//srch->insert(kwKey,"text,keywords");
	int i =1;
        for(auto block : blocks)
	{
		Bid mapKey = createBid(ind, i);
		srch->insert(mapKey,block);
		////cout << i <<"in if BLK["<<block <<"\n";
		i++;
	}

}

void BOrion::insert(string keyword, string ind) {
    //Bid mapKey = createBid(keyword, ind);
    //auto updt_cnt = updt->find(mapKey);
    Bid mapKey = createBid(keyword, 0);
    auto updt_cnt = srch->find(mapKey);
    //updt_cnt/4 will give us actual updc, 
    //because now we store 4 files in one block
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
       srch->insert(mapKey, to_string(updc));
       cout << "updc value:"<< updc << endl;
       int pos_in_block = updc%COM; // 1,2,3,4, ..., 16
       if (pos_in_block == 0) pos_in_block = COM;
       float bnum = ceil(updc/COM); // 4 file-ids each block
      
       int block_num ;
       if(updc%COM==0)
	      block_num = bnum;
       else 
	       block_num = bnum+1;

       cout << "block_num value:"<< block_num << endl;
       cout << "pos_in_block value:"<< pos_in_block << endl;
       Bid key = createBid(keyword, block_num);
	
	if(updc%COM == 1)
	{
           //cout << "SIZEofIND:"<< ind.length() << endl;
	   ind.insert(pos_in_block*FID_SIZE,ID_SIZE-pos_in_block*FID_SIZE,'#');
           srch->insert(key, ind);
	   cout <<"NEWBLOCK:"<< ind <<endl;

	}
	else
	{
	    string oldblock = srch->find(key);
            cout << "SIZEofOLDIND:"<< oldblock.length() << endl;
	    cout <<"OLDBLOCK:" << oldblock << endl;
	    oldblock.replace((pos_in_block-1)*FID_SIZE,FID_SIZE,ind);
	    cout <<"OLDNEWBLOCK:" << oldblock << endl;
	    srch->insert(key, oldblock);
	}
        //LastIND[keyword] = ind; // no need to keep this
    //}
}

/**
 * This function executes an insert in setup mode. Indeed, it is not applied until endSetup()
 */
void BOrion::setupInsert(string keyword, string ind) {
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
    auto updt_cnt = srch->find(mapKey);
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
        setupPairs2[key]= ind;
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

//vector<string> BOrion::searchWrapper(string keyword)
//{
//	vector<string> result;
//	vector<string> files;
//	
//	result = search(keyword);
//	for(auto id:result)
//	{
//		vector<string> file;
//		file = search(id);
//		for(auto blk:file)
//		    files.push_back(blk);
//	}
//	return files;
//}


vector<string> BOrion::search(string keyword) {
    vector<string> result;
    vector<Bid> bids;
    Bid mapKey = createBid(keyword, 0);
    auto updt_cnt = srch->find(mapKey);
//    cout << "I am printing updt_cnt in search:" << updt_cnt << "\n";
//    Bid mk = createBid("test2", 1);
//         string first = srch->find(mk);
//	      cout << "Indi[["<< first <<"]\n";

    int updc;

    if (updt_cnt != "") {
        stringstream convstoi(updt_cnt);
        convstoi >> updc;

        for (int i = 1; i <= updc; i++) {
            Bid bid = createBid(keyword, i);
            bids.push_back(bid);
        }
    }
    auto tmpRes = srch->batchSearch(bids);
    for(auto item:tmpRes){
        //result.push_back(stoi(item));
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

