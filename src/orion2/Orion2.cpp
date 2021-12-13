#include "Orion2.h"

Orion2::Orion2(bool usehdd, int kwSize, int FileSize) {
    this->useHDD = usehdd;
    bytes<Key> key1{0};
    bytes<Key> key2{1};
    srch = new OMAP(kwSize*4, key1);
    updt = new OMAP(kwSize*4, key2);
    files = new OMAPf(FileSize*8, key1);
}

Orion2::~Orion2() {
    delete srch;
    delete updt;
    delete files;
}

void Orion2::insertFile(int ind, vector<string> blocks)
{
	Bid mapKey = createBidF(ind, 0);
	int sz = blocks.size();
	//cout << "size of file:" << sz << "\n\n" ;
	files->insert(mapKey, to_string(sz));
	int i = 1;
	for(auto block : blocks)
	{
		mapKey = createBidF(ind, i);
		i++;
		files->insert(mapKey, block); 
		string str = files->find(mapKey);
		//cout <<"IMMEDIATE:["<< str <<"]\n" ;
	}
}
void Orion2::insert(string keyword, int ind) {
    Bid mapKey = createBid(keyword, ind);
    auto updt_cnt = updt->find(mapKey);
    if (updt_cnt == "") {
        if (UpdtCnt.count(keyword) == 0) {
            UpdtCnt[keyword] = 0;
        }
        UpdtCnt[keyword]++;
        updt->insert(mapKey, to_string(UpdtCnt[keyword]));
        Bid key = createBid(keyword, UpdtCnt[keyword]);
        srch->insert(key, to_string(ind));
        LastIND[keyword] = ind;
    }
}

/**
 * This function executes an insert in setup mode. Indeed, it is not applied until endSetup()
 */
void Orion2::setupInsert(string keyword, int ind) {
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

void Orion2::remove(string keyword, int ind) {
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

/**
 * This function executes a remove in setup mode. Indeed, it is not applied until endSetup()
  */
void Orion2::setupRemove(string keyword, int ind) {
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

vector<string> Orion2::searchFile(int ind){
	vector<string> result;
	Bid bid = createBidF(ind, 0);
	string sz = files->find(bid);
	int numblk;
	stringstream convstoi(sz);
	        convstoi >> numblk;
	vector<Bid> keys;
	for(int j=1; j<=numblk; j++)
	{
		bid = createBidF(ind, j);
		keys.push_back(bid);		
		//string str = files->find(bid);
		//result.push_back(str);
	}
	result = files->batchSearch(keys);
	
	return result;
}

vector<int> Orion2::search(string keyword) {
    vector<int> result;
    vector<Bid> bids;
    if (UpdtCnt.count(keyword) != 0) {
        for (int i = 1; i <= UpdtCnt[keyword]; i++) {
            Bid bid = createBid(keyword, i);
            bids.push_back(bid);
        }
    }
    auto tmpRes = srch->batchSearch(bids);
    for(auto item:tmpRes){
        result.push_back(stoi(item));
    }
    return result;
}

/**
 * This function is used for initial setup of scheme because normal update is time consuming
 */
void Orion2::beginSetup() {
    setupPairs1.clear();
    setupPairs2.clear();
}

/**
 * This function is used for finishing setup of scheme because normal update is time consuming
 */
void Orion2::endSetup() {
    updt->batchInsert(setupPairs1);
    srch->batchInsert(setupPairs2);
}

Bid Orion2::createBid(string keyword, int number) {
    Bid bid(keyword);
    auto arr = to_bytes(number);
    std::copy(arr.begin(), arr.end(), bid.id.end() - 4);
    return bid;
}

Bid Orion2::createBidF(int ind, int blocknum) {
    Bid bid(ind);
    auto arr = to_bytes(blocknum);
    std::copy(arr.begin(), arr.end(), bid.id.end() - 4);
    return bid;
}

