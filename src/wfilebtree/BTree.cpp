#include "BTree.h"

BTree::BTree(int maxSize, bytes<Key> key) 
	: rd(), mt(rd()), dis(0, (pow(2, floor(log2(maxSize/Z))+1)-1)/2)
{
    bram = new BRAM(maxSize, key);
    totleaves = (pow(2, floor(log2(maxSize/Z))+1)-1)/2;
    cout <<"total leaves in Btree:(0.."<< totleaves<<")"<<totleaves+1<<endl;
    //cout <<maxSize<<"--------------------------------------------"<<endl;
    brootKey = 0;
    brootPos = -1;

}
BTree::~BTree() 
{
	delete bram;
}

BTreeNode* BTree::newBTreeNode(int nextbid, int leafpos) 
{
    BTreeNode* node = new BTreeNode();
    node->bid = nextbid;
    node->pos = leafpos;
    //node->height = 0;
    for(int i =0;i<D-1;i++)
    {
	    node->keys[i] = Bid(0); 
	    node->values[i]=0;
	    node->cbids[i]=0;
	    node->cpos[i]=-1;
    }
    node->cbids[D-1]=0;
    node->cpos[D-1]=-1;
    return node;
}

int BTree::keynum(BTreeNode* bn)
{
	int knum = 0;
	int i = 0;
	while(i<D-1 && bn->values[i]!=0)
	{
		i++;
		knum++;
	}
	return knum;
}
bool BTree::isleaf(BTreeNode* bn)
{
	if(bn->cpos[0]==-1)
		return true;
	else return false;
}

int BTree::nextBid()
{
	nextbid++;
	return nextbid;
}

void BTree::insert(Bid kw, int id)
{
	bram->start(false);
	brootKey = insertkw(kw,id,brootKey,brootPos);
	bram->finalize(brootKey,brootPos);
}

int BTree::insertkw(Bid kw, int id, int brootKey, int &brootPos)
{
	if (brootKey == 0)
	{
		//BTreeNode *root = newBTreeNode(true, nextBid(),RandomPath());
		BTreeNode *root = newBTreeNode(nextBid(),RandomPath());
		root->keys[0] = kw;
		root->values[0] = id;
		//root->knum = 1; 
		brootPos = root->pos;
		brootKey = root->bid;
		brootPos = bram->WriteBTreeNode(root->bid, root);
		return brootKey;
	}
	else 
	{
		BTreeNode *root = bram->ReadBTreeNode(brootKey,brootPos);
		int rootknum = keynum(root);
		if (rootknum == 2*T-1)
		{
			BTreeNode *s = newBTreeNode(nextBid(),RandomPath());
			s->cbids[0] = root->bid;
			s->cpos[0] = root->pos;
			//s->height = root->height+1;
			//BTreeNode *z = newBTreeNode(root->isleaf, nextBid(),RandomPath());
			BTreeNode *z= newBTreeNode(nextBid(),RandomPath());
			splitChild(s,0, root, z);
			bram->WriteBTreeNode(z->bid,z);
			bram->WriteBTreeNode(s->bid,s);
			bram->WriteBTreeNode(root->bid,root);
			BTreeNode *sc;
			if (s->keys[0] < kw)
				sc = s;
			else
				sc = root;
			insertNFull(kw,id,sc);
			brootKey = s->bid;
			brootPos = s->pos;
		}
		else 
		{
			insertNFull(kw,id,root);
		}
	}
return brootKey;
}
void BTree::insertNFull(Bid kw,int id, BTreeNode*& node)
{
	//int i = node->knum-1;
	int i = keynum(node)-1;
	//if(node->isleaf == true)
	if(isleaf(node) == true)
	{
		while(i>=0 && node->keys[i]>kw)
		{
			node->keys[i+1] = node->keys[i];
			node->values[i+1] = node->values[i];
			i--;
		}
		node->keys[i+1] = kw;
		node->values[i+1] = id;
		//node->knum = node->knum+1;
		bram->WriteBTreeNode(node->bid,node);
	}
	else
	{
		while(i>=0 && node->keys[i] > kw)
			i--;
      		BTreeNode *ci = bram->ReadBTreeNode(node->cbids[i+1],node->cpos[i+1]);
		int ciknum = keynum(ci);
		if (ciknum == 2*T-1)
		{
			//BTreeNode *z = newBTreeNode(isleaf(ci), nextBid(),RandomPath());
			BTreeNode *z = newBTreeNode(nextBid(),RandomPath());
			splitChild(node,i+1, ci, z);
			bram->WriteBTreeNode(z->bid,z);
			bram->WriteBTreeNode(ci->bid,ci);
			bram->WriteBTreeNode(node->bid,node);
			if (node->keys[i+1] < kw)
			{
				ci = z;
			}
		}
		insertNFull(kw,id,ci);
	}
}

void BTree::splitChild(BTreeNode *&par, int i, BTreeNode *&y, BTreeNode *&z)
{
	//z->height = y->height;
	for (int j = 0; j < T-1; j++)
	{
		z->keys[j] = y->keys[j+T];
		z->values[j] = y->values[j+T];
	}
	if (isleaf(y) == false)
	{
		for (int j = 0; j < T; j++)
		{
			z->cbids[j] = y->cbids[j+T];
			z->cpos[j] = y->cpos[j+T];
		}
	}
	int parknum = keynum(par);
	for (int j = parknum; j >= i+1; j--)
	{
		par->cbids[j+1] = par->cbids[j];
		par->cpos[j+1]=par->cpos[j];
	}
	par->cbids[i+1] = z->bid;
	par->cpos[i+1]=z->pos;
	
	for (int j = parknum-1; j >= i; j--)
	{
		par->keys[j+1] = par->keys[j];
		par->values[j+1] = par->values[j];
	}
	
	par->keys[i] = y->keys[T-1];
	par->values[i] = y->values[T-1];
	//par->knum = par->knum + 1;
	//y->knum = T - 1;
	for(int k = T-1; k<D-1; k++) // for keynum
	{
		y->values[k]=0;
	}
	//assert(keynum(y) == y->knum);
}

vector<int> BTree::batchSearch(vector<Bid> bids)
{
	vector<int> results;
	bram->start(false);
	for(auto b:bids)
	{
		int res = 0;
		int mh = minHeight;
		searchkw(brootKey,brootPos,b,res,mh);
		//cout <<"["<<res<<"] ";
		results.push_back(res);
	}
	bram->finalize(brootKey,brootPos);
	return results;
}

int BTree::search(Bid kw)
{
	bram->start(false);
	int res = 0;
	int mh = minHeight;
    	bram->searchi_bytes = 0;
	bram->rtt = 0;
	searchkw(brootKey,brootPos,kw,res,mh);
	bram->finalize(brootKey,brootPos);
    	searchi_bytes = bram->searchi_bytes;
	rtt = bram->rtt;
	return res;
}

void BTree::searchkw(int rootKey, int rootPos, Bid kw, int &res, int mh)
{
	if(res != 0)
	     return;
	BTreeNode* node = bram->ReadBTreeNode(rootKey,rootPos,mh);
	int nodeknum = keynum(node);
        int i = 0;
        while (i < nodeknum && kw > node->keys[i])
	{
                i++;
	}
        if (i<nodeknum && node->keys[i] == kw)
	{
		res = node->values[i];
		//cout <<node->keys[i]<<" FOUND in BTree at height:"<<mh<<endl;
                return ;
	}
	else if (isleaf(node) == true)
	{
		//cout <<"NOT Found in BTree"<<endl;
                return;
	}
        searchkw(node->cbids[i],node->cpos[i],kw,res,mh-1);
}

int BTree::RandomPath() 
{
    int val = dis(mt);
    return val;
}


void BTree::remove(Bid k)
{
  bram->start(false);
  removekw(k);
  bram->finalize(brootKey,brootPos);
}

void BTree::removekw(Bid k) 
{
  int mh = minHeight;
  if (brootKey==0) 
  {
    cout << "The tree is empty\n";
    return;
  }
  BTreeNode* root = bram->ReadBTreeNode(brootKey,brootPos,mh);
  deletion(k,root,mh);

  //if (root->knum == 0) 
  if (keynum(root) == 0) 
  {
    //if (root->isleaf)
    if (isleaf(root))
    {
      brootKey = 0;
      brootPos = -1;
    }
    else
    {
     brootKey = root->cbids[0];
     brootPos = root->cpos[0];
    }
    int rootbid = root->bid;
    root->bid = 0;
    bram->WriteBTreeNode(rootbid,root);
    //delete root;
  }
  return;
}

int BTree::findKey(Bid k, BTreeNode* node) 
{
  int nodeknum = keynum(node);
  int idx = 0;
  while (idx < nodeknum && node->keys[idx] < k)
    ++idx;
  return idx;
}

void BTree::deletion(Bid kw, BTreeNode *&node, int mh) 
{
  int idx = findKey(kw,node);
  int nodeknum = keynum(node);
  if (idx < nodeknum && node->keys[idx] == kw) //#
  {
    if (isleaf(node))
    {
	  removeFromLeaf(idx,node);
	  if(keynum(node) != 0)
	  	bram->WriteBTreeNode(node->bid, node);
	  else
	  {
		  int nodebid = node->bid;
		  node->bid = 0;
		  bram->WriteBTreeNode(nodebid,node);
	  }
    }
    else
      removeFromNonLeaf(idx,node,mh);
  } 
  else 
  {
    if (isleaf(node)) 
    {
      cout << "The key " << kw << " does not exist in the tree\n";
      return;
    }
    bool flag = ((idx == nodeknum) ? true : false);
    BTreeNode* ci = bram->ReadBTreeNode(node->cbids[idx],node->cpos[idx],(mh-1));

    if (keynum(ci) < T)
    {
	fill(idx,node,mh);
    }
    if (flag && (idx > keynum(node)))
    {
	BTreeNode* ci1 = bram->ReadBTreeNode(node->cbids[idx-1],node->cpos[idx-1],mh-1);
	deletion(kw,ci1,mh-1);
    }
    else
    {
    	BTreeNode* ci = bram->ReadBTreeNode(node->cbids[idx],node->cpos[idx],mh-1);
	deletion(kw,ci,mh-1);
    }
  }
  return;
}

void BTree::removeFromLeaf(int idx, BTreeNode *&node) 
{
  int nodeknum = keynum(node);
  for (int i = idx + 1; i < nodeknum; ++i)
  {
    node->keys[i-1] = node->keys[i];
    node->values[i-1] = node->values[i];
  }
  node->keys[nodeknum-1]=Bid(0);
  node->cpos[nodeknum]=-1; //# for isleaf
  node->values[nodeknum-1]=0;
  return;
}

void BTree::removeFromNonLeaf(int idx, BTreeNode *&node, int mh)
{
  Bid k = node->keys[idx];
  BTreeNode* ci = bram->ReadBTreeNode(node->cbids[idx],node->cpos[idx],(mh-1));
  BTreeNode* ci1 = bram->ReadBTreeNode(node->cbids[idx+1],node->cpos[idx+1],(mh-1));//<<==
  if (keynum(ci) >= T) ////
  {
    auto pred = getPredecessor(idx,node,mh);
    node->keys[idx] = pred.first;
    node->values[idx] = pred.second;

    bram->WriteBTreeNode(node->bid,node);
    deletion(pred.first,ci,(mh-1));
  }
  else if (keynum(ci1) >= T) 
  {
    auto succ = getSuccessor(idx,node,mh);
    node->keys[idx] = succ.first;
    node->values[idx] = succ.second;

    bram->WriteBTreeNode(node->bid,node);
    deletion(succ.first,ci1,(mh-1));
  }
  else 
  {
    merge(idx,node,mh);
    deletion(k,ci,mh-1);
  }
  return;
}

pair<Bid,int> BTree::getPredecessor(int idx, BTreeNode* node, int mh) 
{
	BTreeNode *cur = bram->ReadBTreeNode(node->cbids[idx],node->cpos[idx],mh-1);
	int h = mh-2;
	while (!isleaf(cur))
	{
	      cur=bram->ReadBTreeNode(cur->cbids[keynum(cur)],cur->cpos[keynum(cur)],h);
	      h--;
	}
	return make_pair(cur->keys[keynum(cur)-1],cur->values[keynum(cur)-1]);
}

pair<Bid,int> BTree::getSuccessor(int idx, BTreeNode* node,int mh)
{
	BTreeNode *cur=bram->ReadBTreeNode(node->cbids[idx+1],node->cpos[idx+1],mh-1);
	int h = mh-2;
	while (!isleaf(cur))
	{
		cur=bram->ReadBTreeNode(cur->cbids[0],cur->cpos[0],h);
		h--;
	}
	return make_pair(cur->keys[0],cur->values[0]);
}

void BTree::fill(int idx, BTreeNode *&node,int mh) 
{
	if(idx!=0)
	{
		BTreeNode *ci=bram->ReadBTreeNode(node->cbids[idx-1],node->cpos[idx-1],mh-1);
		if(keynum(ci) >= T)
		{
			borrowFromPrev(idx,node,mh);
			return;
		}
	}
	if (idx != keynum(node))
	{
		BTreeNode *ci=bram->ReadBTreeNode(node->cbids[idx+1],node->cpos[idx+1],mh-1);
		if(keynum(ci)>=T)
		{
			borrowFromNext(idx,node,mh);
			return;
		}
	}
	if (idx != keynum(node))
		merge(idx,node,mh);
	else //if(idx==node->knum)//if (idx !=0)
		merge(idx-1,node,mh);
  return;
}

void BTree::borrowFromPrev(int idx, BTreeNode *&node, int mh) 
{
  BTreeNode *child = bram->ReadBTreeNode(node->cbids[idx],node->cpos[idx],mh-1);
  BTreeNode *sibling = bram->ReadBTreeNode(node->cbids[idx-1],node->cpos[idx-1],mh-1);

  for (int i = keynum(child)-1; i >= 0; --i)
  {
    child->keys[i+1] = child->keys[i];
    child->values[i+1] = child->values[i];
  }
  if (!isleaf(child)) 
  {
    for (int i = keynum(child); i >= 0; --i)
    {
      child->cbids[i + 1] = child->cbids[i];
      child->cpos[i + 1] = child->cpos[i];
    }
  }

  child->keys[0] = node->keys[idx-1];
  child->values[0] = node->values[idx-1];

  if (!isleaf(child))
  {
    child->cbids[0] = sibling->cbids[keynum(sibling)];
    child->cpos[0] = sibling->cpos[keynum(sibling)];
  }
  node->keys[idx-1] = sibling->keys[keynum(sibling)-1];
  node->values[idx-1] = sibling->values[keynum(sibling)-1];

  sibling->cpos[keynum(sibling)]=-1; //# for isleaf
  sibling->values[keynum(sibling)-1]=0; //# for keynum
  bram->WriteBTreeNode(child->bid,child);
  bram->WriteBTreeNode(sibling->bid,sibling);
  bram->WriteBTreeNode(node->bid,node);
  return;
}

void BTree::borrowFromNext(int idx, BTreeNode *&node,int mh) 
{
  BTreeNode *child = bram->ReadBTreeNode(node->cbids[idx],node->cpos[idx],mh-1);
  BTreeNode *sibling = bram->ReadBTreeNode(node->cbids[idx+1],node->cpos[idx+1],mh-1);

  if (!isleaf(child))
  {
    child->cbids[keynum(child) + 1] = sibling->cbids[0];
    child->cpos[keynum(child) + 1] = sibling->cpos[0];
  }
  child->keys[keynum(child)] = node->keys[idx];
  child->values[keynum(child)] = node->values[idx];

  node->keys[idx] = sibling->keys[0];
  node->values[idx] = sibling->values[0];

  for (int i = 1; i < keynum(sibling); ++i)
  {
    sibling->keys[i - 1] = sibling->keys[i];
    sibling->values[i - 1] = sibling->values[i];
  }
  if (!isleaf(sibling)) 
  {
    for (int i = 1; i <= keynum(sibling); ++i)
    {
      sibling->cbids[i - 1] = sibling->cbids[i];
      sibling->cpos[i - 1] = sibling->cpos[i];
    }
  }
  sibling->values[keynum(sibling)-1] = 0; //# for keynum
  bram->WriteBTreeNode(child->bid,child);
  bram->WriteBTreeNode(sibling->bid,sibling);
  bram->WriteBTreeNode(node->bid,node);
  return;
}

void BTree::merge(int idx, BTreeNode *&node,int mh)
{
   BTreeNode *child = bram->ReadBTreeNode(node->cbids[idx],node->cpos[idx],mh-1);
   BTreeNode *sibling = bram->ReadBTreeNode(node->cbids[idx+1],node->cpos[idx+1],mh-1);
   child->keys[T-1] = node->keys[idx];
   child->values[T-1] = node->values[idx];

    for (int i=0; i<keynum(sibling); ++i)
    {
        child->keys[i+T] = sibling->keys[i];
        child->values[i+T] = sibling->values[i];
    }
    if (!isleaf(child))
    {
        for(int i=0; i<=keynum(sibling); ++i)
	{
            child->cbids[i+T] = sibling->cbids[i];
            child->cpos[i+T] = sibling->cpos[i];
	}
    }
    for (int i=idx+1; i<keynum(node); ++i)
    {
        node->keys[i-1] = node->keys[i];
        node->values[i-1] = node->values[i];
    }
    for (int i=idx+2; i<=keynum(node); ++i)
    {
        node->cbids[i-1] = node->cbids[i];
        node->cpos[i-1] = node->cpos[i];
    }
    node->cpos[keynum(node)]=-1; //# for isleaf
    node->values[keynum(node)-1]=0; //# for keynum
    //delete(sibling); //??
    int sbid = sibling->bid;
    sibling->bid = 0;
    bram->WriteBTreeNode(child->bid,child);
    bram->WriteBTreeNode(sbid,sibling);
    bram->WriteBTreeNode(node->bid,node);
    return;
}

int maxHeight = 0;
int getMaxNodes(int h)
{
	int sum = 0;
	for(int i = 0; i<=h ;i++)
	{
		sum+=pow(D,i)*(D-1);
	}
	return sum;
}


void BTree::setupInsert(map<Bid,int>input)
{
	int inputSize = input.size();
	cout <<"input size:"<<inputSize<<endl;
	minHeight = (ceil(log(input.size()+1))/log(D))-1;
	int maxNodes = getMaxNodes(minHeight);
	if(maxNodes < inputSize)
	{
		minHeight = minHeight+1;
		maxNodes = getMaxNodes(minHeight);
	}
	cout <<"Height of BTree:"<<minHeight<<endl;
	cout <<"maxNodes:"<<maxNodes<<endl;
	bram->maxHeight = minHeight;
	Bid last = input.rbegin()->first;
	cout <<"last:"<<last<<endl;
	for(int m = input.size(); m<maxNodes; m++)
	{
		++last;
		input[last]=99999;
	}
	cout <<"Padded input size:"<<input.size()<<endl;
	/*
	for(auto it = input.begin();it!=input.end();it++)
	{
		if(next(it) != input.end())
		{
			if(next(it)->first>=it->first)
			 cout <<"TRUE"<<endl;
			else if(next(it)->first < it->first)
			{
				cout <<"violation";
				break;
			}
		}
	}
	*/
	brootPos = RandomPath();
	brootKey = nextBid();
	brootKey = createBTreeNode(brootKey,brootPos,input,minHeight);
}

vector<map<Bid,int>> createGroups(map<Bid,int> mp)
{
	vector<map<Bid,int>> groups;
	groups.reserve(D);
	int gb,gl;
	int g;
	gb = ceil(mp.size()/double(D));
	gl = floor(mp.size()/double(D));
	if(mp.size()<D)
		g = mp.size();
	else if((gb*D-mp.size())==gb)
		g = gl;
	else
		g = gb;
	auto it = mp.begin();
	for(int v = 0; v < D; v++)
	{
		map<Bid,int> m;
		for(int gg = 0; gg<g; gg++)
		{
			if(it != mp.end())//redundant check ?
			{
				Bid b = it->first;
				m[b] = it->second;
				it++;
			}
		}
		groups.push_back(m);
		if(mp.size()<D)
			break;
	}
	return groups;
}

int BTree::createBTreeNode(int nextbid, int &leafpos, map<Bid,int> input, int minHeight)
{
	BTreeNode* node = newBTreeNode(nextbid,leafpos);
	//node->height = minHeight;
	vector<map<Bid,int>> groups;
	groups.reserve(D);
	groups = createGroups(input);
	if(groups.size() ==1)
	{
		int i = 0;
		for(auto it = groups[0].begin(); it!= groups[0].end(); it++)
		{
			assert(groups[0].size()<=D-1);
			assert(T-1 <= groups[0].size());
			node->keys[i] = it->first;
			node->values[i] = it->second;
			i++;
		}
		sn.push_back(node);
	}
	else if(groups.size()>1)
	{
		for(int i=0;i<D-1;i++)
		{
			map<Bid,int>::iterator it = groups[i].end();
			it--;
			node->keys[i] = it->first;
			node->values[i] = it->second;
			groups[i].erase(it);
			node->cbids[i] = nextBid();
			node->cpos[i] = RandomPath();
		}
		node->cbids[D-1]= nextBid();
		node->cpos[D-1]= RandomPath();
		sn.push_back(node);
		for(int i=0;i<D;i++)
	 		node->cbids[i] = createBTreeNode(node->cbids[i],
					node->cpos[i],groups[i],minHeight-1);
	}
	return node->bid;
}

void BTree::endSetup()
{
	sort(sn.begin(),sn.end(),[ ](const BTreeNode* lhs, const BTreeNode * rhs)
	{
        	return lhs->pos < rhs->pos;
	});
	bram->setupInsert(sn);
}

