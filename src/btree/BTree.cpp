#include "BTree.h"

BTree::BTree(int maxSize, bytes<Key> key) 
	: rd(), mt(rd()), dis(0, (pow(2, floor(log2(maxSize/Z))+1)-1)/2)
{
    bram = new BRAM(maxSize, key);
    totleaves = (pow(2, floor(log2(maxSize/Z))+1)-1)/2;
    cout <<"total leaves in Btree:(0.."<< totleaves<<")"<<totleaves+1<<endl;
    cout <<maxSize<<"--------------------------------------------"<<endl;
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
    //node->isleaf=leaf;
    node->bid = nextbid;
    node->pos = leafpos;
    //node->knum = 0;
    node->height = 1;
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

int BTree::get_knum(BTreeNode* bn)
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
		int rootknum = get_knum(root);
		if (rootknum == 2*T-1)
		{
			BTreeNode *s = newBTreeNode(nextBid(),RandomPath());
			s->cbids[0] = root->bid;
			s->cpos[0] = root->pos;
			s->height = root->height+1;
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
	int i = get_knum(node)-1;
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
		int ciknum = get_knum(ci);
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
	cout <<par->bid<<" SPLITTING:"<<y->bid<<endl;
	//z->knum = T - 1;
	z->height = y->height;
	for (int j = 0; j < T-1; j++)
	{
		z->keys[j] = y->keys[j+T];
		z->values[j] = y->values[j+T];
	}
	//if (y->isleaf == false)
	if (isleaf(y) == false)
	{
		for (int j = 0; j < T; j++)
		{
			z->cbids[j] = y->cbids[j+T];
			z->cpos[j] = y->cpos[j+T];
		}
	}
	int parknum = get_knum(par);
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
	for(int k = T-1; k<D-1; k++) // for get_knum
	{
		y->values[k]=0;
	}
	//assert(get_knum(y) == y->knum);
}

vector<int> BTree::batchSearch(vector<Bid> bids)
{
	vector<int> results;
	bram->start(false);
	cout <<"brootkey:"<<brootKey<<" brootPos:"<<brootPos<<endl;
	for(auto b:bids)
	{
		int res = 0;
		searchkw(brootKey,brootPos,b,res);
		results.push_back(res);
	}
	bram->finalize(brootKey,brootPos);
	return results;
}

int BTree::search(Bid kw)
{
	bram->start(false);
	int res = 0;
	searchkw(brootKey,brootPos,kw,res);
	bram->finalize(brootKey,brootPos);
	return res;
}

void BTree::searchkw(int rootKey, int rootPos, Bid kw, int &res)
{
	if(res != 0)
	     return;
	BTreeNode* node = bram->ReadBTreeNode(rootKey,rootPos);
	int nodeknum = get_knum(node);
        int i = 0;
        while (i < nodeknum && kw > node->keys[i])
	{
                i++;
	}
        if (i<nodeknum && node->keys[i] == kw)
	{
		res = node->values[i];
		//cout <<node->keys[i]<<" FOUND in BTree:"<<res<<endl;
                return ;
	}
	//else if (node->isleaf == true)
	else if (isleaf(node) == true)
	{
		//cout <<"NOT Found in BTree"<<endl;
                return;
	}
        searchkw(node->cbids[i],node->cpos[i],kw,res);
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
  bram->finalizedel(brootKey,brootPos);
}

void BTree::removekw(Bid k) 
{
  if (brootKey==0) 
  {
    cout << "The tree is empty\n";
    return;
  }
  BTreeNode* root = bram->ReadBTreeNode(brootKey,brootPos);
  deletion(k,root);

  //if (root->knum == 0) 
  if (get_knum(root) == 0) 
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
  int nodeknum = get_knum(node);
  int idx = 0;
  while (idx < nodeknum && node->keys[idx] < k)
    ++idx;
  return idx;
}

void BTree::deletion(Bid kw, BTreeNode *&node) 
{
  int idx = findKey(kw,node);
  int nodeknum = get_knum(node);
  if (idx < nodeknum && node->keys[idx] == kw) //#
  {
    //if (node->isleaf)
    if (isleaf(node))
    {
	  removeFromLeaf(idx,node);
	  //if(node->knum != 0)
	  if(get_knum(node) != 0)
	  	bram->WriteBTreeNode(node->bid, node);
	  else
	  {
		  int nodebid = node->bid;
		  node->bid = 0;
		  bram->WriteBTreeNode(nodebid,node);
	  }
    }
    else
      removeFromNonLeaf(idx,node);
  } 
  else 
  {
    //if (node->isleaf) 
    if (isleaf(node)) 
    {
      cout << "The key " << kw << " does not exist in the tree\n";
      return;
    }
    //bool flag = ((idx == node->knum) ? true : false);
    bool flag = ((idx == nodeknum) ? true : false);
    BTreeNode* ci = bram->ReadBTreeNode(node->cbids[idx],node->cpos[idx]);

    //if (ci->knum < T)
    if (get_knum(ci) < T)
    {
	fill(idx,node);
    }
    //if (flag && (idx > node->knum))
    if (flag && (idx > get_knum(node)))
    {
	BTreeNode* ci1 = bram->ReadBTreeNode(node->cbids[idx-1],node->cpos[idx-1]);
	deletion(kw,ci1);
    }
    else
    {
    	BTreeNode* ci = bram->ReadBTreeNode(node->cbids[idx],node->cpos[idx]);//??
	deletion(kw,ci);
    }
  }
  return;
}

void BTree::removeFromLeaf(int idx, BTreeNode *&node) 
{
  int nodeknum = get_knum(node);
  for (int i = idx + 1; i < nodeknum; ++i)
  {
    node->keys[i-1] = node->keys[i];
    node->values[i-1] = node->values[i];
  }
  //node->keys[node->knum-1]=Bid(0);
  node->keys[nodeknum-1]=Bid(0);
  //node->values[node->knum-1]=0;
  node->cpos[nodeknum]=-1; //# for isleaf
  node->values[nodeknum-1]=0;
  //node->knum = node->knum-1;
  return;
}

void BTree::removeFromNonLeaf(int idx, BTreeNode *&node)
{
  Bid k = node->keys[idx];
  BTreeNode* ci = bram->ReadBTreeNode(node->cbids[idx],node->cpos[idx]);
  BTreeNode* ci1 = bram->ReadBTreeNode(node->cbids[idx+1],node->cpos[idx+1]);//<<==
  //if (ci->knum >= T) ////
  if (get_knum(ci) >= T) ////
  {
    auto pred = getPredecessor(idx,node);
    node->keys[idx] = pred.first;
    node->values[idx] = pred.second;

    bram->WriteBTreeNode(node->bid,node);
    deletion(pred.first,ci);
  }
  //else if (ci1->knum >= T) 
  else if (get_knum(ci1) >= T) 
  {
    auto succ = getSuccessor(idx,node);
    node->keys[idx] = succ.first;
    node->values[idx] = succ.second;

    bram->WriteBTreeNode(node->bid,node);
    deletion(succ.first,ci1);
  }
  else 
  {
    merge(idx,node);
    deletion(k,ci);
  }
  return;
}

pair<Bid,int> BTree::getPredecessor(int idx, BTreeNode* node) 
{
	BTreeNode *cur = bram->ReadBTreeNode(node->cbids[idx],node->cpos[idx]);
	//while (!cur->isleaf)
	while (!isleaf(cur))
	{
	      //cur=bram->ReadBTreeNode(cur->cbids[cur->knum],cur->cpos[cur->knum]);
	      cur=bram->ReadBTreeNode(cur->cbids[get_knum(cur)],cur->cpos[get_knum(cur)]);
	}
	//return make_pair(cur->keys[cur->knum-1],cur->values[cur->knum-1]);
	return make_pair(cur->keys[get_knum(cur)-1],cur->values[get_knum(cur)-1]);
}

pair<Bid,int> BTree::getSuccessor(int idx, BTreeNode* node)
{
	BTreeNode *cur=bram->ReadBTreeNode(node->cbids[idx+1],node->cpos[idx+1]);
	//while (!cur->isleaf)
	while (!isleaf(cur))
	{
		cur=bram->ReadBTreeNode(cur->cbids[0],cur->cpos[0]);
	}
	return make_pair(cur->keys[0],cur->values[0]);
}

void BTree::fill(int idx, BTreeNode *&node) 
{
	if(idx!=0)
	{
		BTreeNode *ci = bram->ReadBTreeNode(node->cbids[idx-1],node->cpos[idx-1]);
		if(get_knum(ci) >= T)
		{
			borrowFromPrev(idx,node);
			return;
		}
	}
	if (idx != get_knum(node))
	{
		BTreeNode *ci=bram->ReadBTreeNode(node->cbids[idx+1],node->cpos[idx+1]);
		if(get_knum(ci)>=T)
		{
			borrowFromNext(idx,node);
			return;
		}
	}
	if (idx != get_knum(node))
		merge(idx,node);
	else //if(idx==node->knum)//if (idx !=0)
		merge(idx-1,node);
  return;
}

// Borrow from previous
void BTree::borrowFromPrev(int idx, BTreeNode *&node) 
{
  BTreeNode *child = bram->ReadBTreeNode(node->cbids[idx],node->cpos[idx]);
  BTreeNode *sibling = bram->ReadBTreeNode(node->cbids[idx-1],node->cpos[idx-1]);

  for (int i = get_knum(child)-1; i >= 0; --i)
  {
    child->keys[i+1] = child->keys[i];
    child->values[i+1] = child->values[i];
  }
  //if (!child->isleaf) 
  if (!isleaf(child)) 
  {
    for (int i = get_knum(child); i >= 0; --i)
    {
      child->cbids[i + 1] = child->cbids[i];
      child->cpos[i + 1] = child->cpos[i];
    }
  }

  child->keys[0] = node->keys[idx-1];
  child->values[0] = node->values[idx-1];

  //if (!child->isleaf)
  if (!isleaf(child))
  {
    child->cbids[0] = sibling->cbids[get_knum(sibling)];
    child->cpos[0] = sibling->cpos[get_knum(sibling)];
  }
  node->keys[idx-1] = sibling->keys[get_knum(sibling)-1];
  node->values[idx-1] = sibling->values[get_knum(sibling)-1];

  sibling->cpos[get_knum(sibling)]=-1; //# for isleaf
  sibling->values[get_knum(sibling)-1]=0; //# for get_knum
  //child->knum += 1;
  //sibling->knum -= 1;
  bram->WriteBTreeNode(child->bid,child);
  bram->WriteBTreeNode(sibling->bid,sibling);
  bram->WriteBTreeNode(node->bid,node);
  return;
}

// Borrow from the next
void BTree::borrowFromNext(int idx, BTreeNode *&node) 
{
  BTreeNode *child = bram->ReadBTreeNode(node->cbids[idx],node->cpos[idx]);
  BTreeNode *sibling = bram->ReadBTreeNode(node->cbids[idx+1],node->cpos[idx+1]);

  //if (!(child->isleaf))
  if (!isleaf(child))
  {
    child->cbids[get_knum(child) + 1] = sibling->cbids[0];
    child->cpos[get_knum(child) + 1] = sibling->cpos[0];
  }
  child->keys[get_knum(child)] = node->keys[idx];
  child->values[get_knum(child)] = node->values[idx];

  node->keys[idx] = sibling->keys[0];
  node->values[idx] = sibling->values[0];

  for (int i = 1; i < get_knum(sibling); ++i)
  {
    sibling->keys[i - 1] = sibling->keys[i];
    sibling->values[i - 1] = sibling->values[i];
  }
  //if (!sibling->isleaf) 
  if (!isleaf(sibling)) 
  {
    for (int i = 1; i <= get_knum(sibling); ++i)
    {
      sibling->cbids[i - 1] = sibling->cbids[i];
      sibling->cpos[i - 1] = sibling->cpos[i];
    }
  }
  sibling->values[get_knum(sibling)-1] = 0; //# for get_knum
  //child->knum += 1;
  //sibling->knum -= 1;
  bram->WriteBTreeNode(child->bid,child);
  bram->WriteBTreeNode(sibling->bid,sibling);
  bram->WriteBTreeNode(node->bid,node);
  return;
}

void BTree::merge(int idx, BTreeNode *&node)
{
   BTreeNode *child = bram->ReadBTreeNode(node->cbids[idx],node->cpos[idx]);
   BTreeNode *sibling = bram->ReadBTreeNode(node->cbids[idx+1],node->cpos[idx+1]);
   child->keys[T-1] = node->keys[idx];
   child->values[T-1] = node->values[idx];

    //for (int i=0; i<sibling->knum; ++i)
    for (int i=0; i<get_knum(sibling); ++i)
    {
        child->keys[i+T] = sibling->keys[i];
        child->values[i+T] = sibling->values[i];
    }
    //if (!child->isleaf)
    if (!isleaf(child))
    {
        //for(int i=0; i<=sibling->knum; ++i)
        for(int i=0; i<=get_knum(sibling); ++i)
	{
            child->cbids[i+T] = sibling->cbids[i];
            child->cpos[i+T] = sibling->cpos[i];
	}
    }
    //for (int i=idx+1; i<node->knum; ++i)
    for (int i=idx+1; i<get_knum(node); ++i)
    {
        node->keys[i-1] = node->keys[i];
        node->values[i-1] = node->values[i];
    }
    //for (int i=idx+2; i<=node->knum; ++i)
    for (int i=idx+2; i<=get_knum(node); ++i)
    {
        node->cbids[i-1] = node->cbids[i];
        node->cpos[i-1] = node->cpos[i];
    }
    node->cpos[get_knum(node)]=-1; //# for isleaf
    node->values[get_knum(node)-1]=0; //# for get_knum
    //delete(sibling); //??
    //child->knum +=sibling->knum+1;
    //node->knum=node->knum-1;
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
	//map<Bid,int> sortedInput = sort(input);
	//FROM HERE
	int minHeight = (ceil(log2(input.size()+1))/log2(D))-1;
	int maxNodes = getMaxNodes(minHeight);
	cout <<"total size:"<<input.size()<<endl;
	cout <<"minHeight:"<<minHeight<<endl;
	cout <<"maxNodes:"<<maxNodes<<endl;
	Bid last = input.rbegin()->first;
	cout <<"last:"<<last<<endl;
	for(int m = input.size(); m<maxNodes; m++)
	{
		++last;
		input[last]=99999;
	}
	cout <<"total size:"<<input.size()<<endl;
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
	brootKey = create_node(brootKey,brootPos,input,minHeight);
}

vector<map<Bid,int>> createGroups(map<Bid,int> mp)
{
	//cout <<"in createGrp"<<endl;
	vector<map<Bid,int>> groups;
	groups.reserve(D);
	int gb,gl;
	int g;
	gb = ceil(mp.size()/double(D));
	gl = floor(mp.size()/double(D));
//	cout <<mp.size()<<endl;
//	cout <<"gb:"<<gb<<" gl:"<<gl<<endl;
	if(mp.size()<D)
		g = mp.size();
	else if((gb*D-mp.size())==gb)
		g = gl;
	else
		g = gb;
	//cout << "value of g:"<< g<<endl;
	auto it = mp.begin();
	for(int v = 0; v < D; v++)
	{
		map<Bid,int> m;
		for(int gg = 0; gg<g; gg++)
		{
			if(it != mp.end())//redundant check ?
			{
				Bid b = it->first;
				//cout<<"[["<<b<<":"<<it->second<<"]]"<<endl;
				m[b] = it->second;
				//cout <<m[b]<<" ";
				it++;
			}
		}
		//cout <<endl<<endl;
		//cout <<"m.size():"<<m.size()<<endl;
		groups.push_back(m);
		if(mp.size()<D)
			break;
	}
	return groups;
}

int BTree::create_node(int nextbid, int &leafpos, map<Bid,int> input, int minHeight)
{
	//cout <<"in createNode"<<endl;
	BTreeNode* node = newBTreeNode(nextbid,leafpos);
	node->height = minHeight;
	vector<map<Bid,int>> groups;
	groups.reserve(D);
	groups = createGroups(input);
	//cout <<"groups.size():"<<groups.size()<<endl;
	if(groups.size() ==1)
	{
		int i = 0;
		for(auto it = groups[0].begin(); it!= groups[0].end(); it++)
		{
			assert(groups[0].size()==D-1);
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
			node->keys[i]=it->first;
			node->values[i]=it->second;
			//cout <<"size before:"<<groups[i].size()<<endl;
			groups[i].erase(it);
			//cout <<"size after:"<<groups[i].size()<<endl;
	
			node->cbids[i]=nextBid();
			node->cpos[i] = RandomPath();
		}
		node->cbids[D-1]= nextBid();
		node->cpos[D-1]= RandomPath();
		//setupNodes[node->bid]= node;
		sn.push_back(node);
	for(int i=0;i<D;i++)
	  node->cbids[i]=create_node(node->cbids[i],node->cpos[i],groups[i],minHeight-1);
	}
	return node->bid;
}

void BTree::endSetup()
{
	sort(sn.begin(),sn.end(),[ ](const BTreeNode* lhs, const BTreeNode * rhs)
	{
        	return lhs->pos < rhs->pos;
	});
	
	for(auto el = sn.begin(); el != sn.end(); el++)
	{
		if(next(el) != sn.end())
		{
			assert((*el)->pos <= (*(next(el)))->pos);
			//cout <<(*el)->pos <<" "<<(*(next(el)))->pos<<endl;
		}
	}
	bram->setupInsert(sn);
}

