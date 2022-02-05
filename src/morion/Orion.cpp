#include "Orion.h"

Orion::Orion(bool usehdd, int maxSize) {
    this->useHDD = usehdd;
    bytes<Key> key1{0};
    bytes<Key> key2{1};
    srch = new OMAP(maxSize*4, key1);
    updt = new OMAP(maxSize*4, key2);
    fcnt = new OMAP(maxSize*4, key2);
}

Orion::~Orion() {
    delete srch;
    delete updt;
}

int stoI(string updt_cnt)
{
	int updc;
	stringstream convstoi(updt_cnt);
	convstoi >> updc;
	return updc;
}

void Orion::insert(vector<string> kws, int ind) 
{
    for(auto kw: kws)
    {
  	  //auto updt_cnt = updt->find(mapKey);
  	  //if (updt_cnt == "") 
	  //{
  	      Bid firstKey = createBid(kw,0);
  	      auto filecnt = fcnt->find(firstKey);
  	      
  	      int fc;
  	      if(filecnt == "")
  	      	fc = 0;
  	      else fc = stoi(filecnt);
  	      
  	      fc++;
  	      
  	      Bid mapKey = createBid(kw, ind);
  	      updt->insert(mapKey, to_string(fc));
  	      fcnt->insert(firstKey, to_string(fc));
  	      
  	      Bid key = createBid(kw, fc);
  	      srch->insert(key, to_string(ind));
    	//}
    }
}

vector<int> Orion::search(string keyword) {
    
    vector<int> result;
    vector<Bid> bids;
    Bid firstKey = createBid(keyword,0);
    string filecnt = updt->find(firstKey);
    int fc;

    if(filecnt == "")
	 fc = 0;
    else
         fc = stoI(filecnt);

        for (int i = 1; i <= fc; i++) {
            Bid bid = createBid(keyword, i);
            bids.push_back(bid);
        }
    auto tmpRes = srch->batchSearch(bids);
    for(auto item:tmpRes){
        result.push_back(stoi(item));
    }
    return result;
}



void Orion::remove(string keyword, int ind) 
{
    Bid mapKey = createBid(keyword, ind);
    string updt_cnt = updt->find(mapKey);
    int uc;
    if(updt_cnt =="")
	uc = 0;
    else
	uc = stoi(updt_cnt);

    if (uc > 0) 
    {
            updt->insert(mapKey, to_string(-1));
            Bid firstKey = createBid(keyword,0);
            string filecnt = updt->find(firstKey);
            int fc;
            if(filecnt == "")	
            	fc = 0;
            else 
            	fc = stoi(filecnt);
            int ufc = fc-1;
            updt->insert(firstKey, to_string(ufc));
            if (fc > 0) 
            {
                if (fc != uc) 
                {
                    Bid lastKey = createBid(keyword,fc);
            	    int lastID = stoi(srch->find(lastKey));
                    Bid curKey = createBid(keyword, lastID);
                    updt->insert(curKey, to_string(uc));
                    Bid curKey2 = createBid(keyword, uc);
                    srch->insert(curKey2, to_string(lastID));
                }
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
