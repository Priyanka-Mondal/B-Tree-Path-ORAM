#include "BOrion.h"
#include<string>
#include<math.h>
#include<queue>

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
	par.first = FB;
	par.second = to_string(sz);
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

void BOrion::insert(string keyword, string ind) {
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
       par.first = ID;
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

       //cout << "block_num value:"<< block_num << endl;
       //cout << "pos_in_block value:"<< pos_in_block << endl;
       Bid key = createBid(keyword, block_num);
	
	if(updc%COM == 1)
	{
           //cout << "SIZEofIND:"<< ind.length() << endl;
	   ind.insert(pos_in_block*FID_SIZE,ID_SIZE-pos_in_block*FID_SIZE,'#');
	   pair <int, string> pr;
	   pr.first =ID;
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
	     pr.first =ID;
	     pr.second = oldblock;
	     srch->insert(key, pr);
	}
        //LastIND[keyword] = ind; // no need to keep this
    //}
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

vector<pair<int,string>> BOrion::searchWrapper(string keyword)
{
	srand(time(NULL));
	vector<pair<int,string>> result;
	vector<pair<int,string>> files;
        int ran; // randomly select between 3 options
        int fetched=0, updc;
	pair<int,string> fileids;
	int fetchedid = 1;

    vector<Bid> bids;
    Bid mapKey = createBid(keyword, 0);
    auto updt = srch->find(mapKey); // mandatory first search
    string updt_cnt = updt.second;
    if (updt_cnt != "") {
        stringstream convstoi(updt_cnt);
        convstoi >> updc;
    }
    else
    {
	    return result;
    }
    queue<string> fupdc;
    if (updc >= 1) // at least one file-id for this keyword
    {
	Bid mapKey = createBid(keyword, 1);
	fileids  = srch->find(mapKey); // mandatory second search
	                               // first 16 fileids
	cout << "file ids all: " << fileids.second << endl;
    	while(fupdc.size()<=updc && fupdc.size()<COM) //0 to 15
	{// getting first 16 at most fileids
		int sz = fupdc.size();
		string temp = fileids.second.substr(FID_SIZE*sz,FID_SIZE);
		fupdc.push(temp);
		cout << "the file id is: "<< sz <<"[" << temp << "]" << endl;
		fetchedid = fetchedid+1;
	}
    }
int head =0;
queue<int> fileblks;
queue<string> fids;
cout << fetchedid << "/" << updc << endl;

while(fetchedid <=updc || fileblks.size()>0)
{
	ran = (rand()%3); // to select one of the three conditions
	ran=0;
	if(ran == 0)
	cout <<"ran value is 0:"<< ran <<endl;
	if(ran==0)
	{
 	    if(fetchedid < updc)
	    {   
	cout <<"ran value is:"<< ran <<endl;
		int block_num = (fetchedid/COM)+1;
		Bid mapKey = createBid(keyword, block_num);
		//fileids  = srch->find(mapKey);
		bids.push_back(mapKey);
		//fetchedid = fetchedid+16; do this later after batchsearch
		int ran2 = rand()%fupdc.size() + 1;
		while(ran2>0) // fetch number of blocks for ran files
		{
			ran2--;
			string s ;
			if(fupdc.size()>0)
			{
	   		  s = fupdc.front();
	   		  cout << "fupdc:" << s <<"ran:"<< ran <<endl;
	   		  fupdc.pop();
	   		  mapKey = createBid(s,0);
	   		  bids.push_back(mapKey);
			}
		}
		if(fileblks.size()>0)
		{
			ran = rand()%fileblks.size() + 1;
			int siz;
		        siz = fileblks.front();
			fileblks.pop();
			string fi;
		        fi = fids.front();
			fids.pop();
			int i = 1;
			while(i <= siz)
			{
			    cout << "fi:" <<fi <<"i:"<< i <<endl;
			    mapKey = createBid(fi,i);
			    i++;
			    bids.push_back(mapKey);
			}
		}
	    }
	}
	/*
	else if (ran == 1)
	{
	}
	
	else if (ran == 2)
	{
	}
*/
}
files = srch->batchSearch(bids);
//here need to update fileblks, fileids, fetchedid etc and go back to while
	return files;
}


vector<pair<int,string>> BOrion::search(string keyword) {
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

