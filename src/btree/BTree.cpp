#include "BTree.h"

BTree::BTree(int maxSize, bytes<Key> key) : rd(), mt(rd()), dis(0, (pow(2, floor(log2(maxSize/Z))+1)-1)/2)
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
/*
BTreeNode::BTreeNode(bool leaf, int nextbid, int leafpos) 
{

    bid = nextbid;
    pos = leafpos;
    knum = 0;
    isleaf=leaf;
    height = 1;
    for(int i =0;i<D-1;i++)
    {
	    array<byte_t,16> arr;
	    fill(arr.begin(),arr.end(),0);
	    keys[i] = Bid(arr); 
	    cbids[i]=0;
	    cpos[i]=-1;
    }
    cbids[D-1]=0;
    cpos[D-1]=-1;
}
*/
BTreeNode* BTree::newBTreeNode(bool leaf, int nextbid, int leafpos) 
{
    BTreeNode* node = new BTreeNode();
    node->isleaf=leaf;
    node->bid = nextbid;
    node->pos = leafpos;
    node->knum = 0;
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

int BTree::nextBid()
{
	nextbid++;
	return nextbid;
}

int BTree::insert(Bid kw, int id)
{
	bram->start(false);
	brootKey = insertkw(kw,id,brootKey,brootPos);
	bram->finalize(brootKey,brootPos);
}

int BTree::insertkw(Bid kw, int id, int brootKey, int &brootPos)
{
	if (brootKey == 0)
	{
		BTreeNode *root = newBTreeNode(true, nextBid(),RandomPath());
		root->keys[0] = kw;
		root->values[0] = id;
		root->knum = 1; 
		brootPos = root->pos;
		brootKey = root->bid;
		brootPos = bram->WriteBTreeNode(root->bid, root);
		return brootKey;
	}
	else 
	{
		BTreeNode *root = bram->ReadBTreeNode(brootKey,brootPos,brootPos);
		if (root->knum == 2*T-1)
		{
			BTreeNode *s = newBTreeNode(false,nextBid(),RandomPath());
			s->cbids[0] = root->bid;
			s->cpos[0] = root->pos;
			s->height = root->height+1;
			cout <<"BTree height is:"<<s->height<<endl;
			BTreeNode *z = newBTreeNode(root->isleaf, nextBid(),RandomPath());
			//s->splitChild(0, root, z);
			splitChild(s,0, root, z);
			bram->WriteBTreeNode(z->bid,z);
			bram->WriteBTreeNode(s->bid,s);
			bram->WriteBTreeNode(root->bid,root);
			int i = 0;
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
	int i = node->knum-1;
	if(node->isleaf == true)
	{
		while(i>=0 && node->keys[i]>kw)
		{
			node->keys[i+1] = node->keys[i];
			node->values[i+1] = node->values[i];
			i--;
		}
		node->keys[i+1] = kw;
		node->values[i+1] = id;
		node->knum = node->knum+1;
		bram->WriteBTreeNode(node->bid,node);
	}
	else
	{
		while(i>=0 && node->keys[i] > kw)
			i--;
      BTreeNode *ci = bram->ReadBTreeNode(node->cbids[i+1],node->cpos[i+1],node->cpos[i+1]);
		if (ci->knum == 2*T-1)
		{
			BTreeNode *z = newBTreeNode(ci->isleaf, nextBid(),RandomPath());
			//node->splitChild(i+1, ci, z);
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
	z->knum = T - 1;
	z->height = y->height;
	for (int j = 0; j < T-1; j++)
	{
		z->keys[j] = y->keys[j+T];
		z->values[j] = y->values[j+T];
	}
	if (y->isleaf == false)
	{
		for (int j = 0; j < T; j++)
		{
			z->cbids[j] = y->cbids[j+T];
			z->cpos[j] = y->cpos[j+T];
		}
	}
	y->knum = T - 1;
	for (int j = par->knum; j >= i+1; j--)
	{
		par->cbids[j+1] = par->cbids[j];
		par->cpos[j+1]=par->cpos[j];
	}
	par->cbids[i+1] = z->bid;
	par->cpos[i+1]=z->pos;
	for (int j = par->knum-1; j >= i; j--)
	{
		par->keys[j+1] = par->keys[j];
		par->values[j+1] = par->values[j];
	}
	par->keys[i] = y->keys[T-1];
	par->values[i] = y->values[T-1];
	par->knum = par->knum + 1;
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
	BTreeNode* node = bram->ReadBTreeNode(rootKey,rootPos,rootPos);
        int i = 0;
        while (i < node->knum && kw > node->keys[i])
	{
                i++;
	}
        if (node->keys[i] == kw)
	{
		res = node->values[i];
                return ;//node->values[i];
	}
	else if (node->isleaf == true)
	{
		cout <<"NOT Found in BTree"<<endl;
                return; //0;
	}
        searchkw(node->cbids[i],node->cpos[i],kw,res);
}

int BTree::RandomPath() 
{
    int val = dis(mt);
    return val;
}

void BTree::remove(int k) 
{
  if (brootKey==0) 
  {
    cout << "The tree is empty\n";
    return;
  }
  BTreeNode* root = bram->ReadBTreeNode(brootKey,brootPos,brootPos);
  root->deletion(k);
  if (root->knum == 0) 
  {
    //BTreeNode *tmp = root;
    if (root->isleaf)
    {
      brootKey = 0;
      brootPos = -1;
    }
    else
    {
     brootKey = root->cbids[0];
     brootPos = root->cpos[0];
    }
    //delete tmp;
    //delete root;
  }
  return;
}

int BTreeNode::findKey(Bid k) 
{
  int idx = 0;
  while (idx < knum && keys[idx] < k)
    ++idx;
  return idx;
}

void BTree::deletion(Bid k, BTreeNode *&node) 
{
  int idx = node->findKey(k);
  if (idx < knum && keys[idx] == k) 
  {
    if (node->isleaf)
      node->removeFromLeaf(idx);
    else
      removeFromNonLeaf(idx,node);
  } 
  else 
  {
    if (node->isleaf) 
    {
      cout << "The key " << k << " does not exist in the tree\n";
      return;
    }
    bool flag = ((idx == node->knum) ? true : false);
    BTreeNode* ci = bram->ReadBTreeNode(node->cbids[idx],node->cpos[idx],node->cpos[idx]);

    if (ci->knum < T)
      fill(idx);
    if (flag && (idx > knum))
    {
    BTreeNode* ci1 = bram->ReadBTreeNode(node->cbids[idx-1],node->cpos[idx-1],node->cpos[idx-1]);
      deletion(k,ci1);
    }
    else
      deletion(k,ci);
  }
  return;
}

// Remove from the leaf
void BTreeNode::removeFromLeaf(int idx) 
{
  for (int i = idx + 1; i < knum; ++i)
    keys[i - 1] = keys[i];
  knum--;
  return;
}
void BTreeNode::removeFromNonLeaf(int idx, BTreeNode *&node)
{
  int k = keys[idx];
  if (node->cbids[idx]->knum >= T) ////
  {
    int pred = getPredecessor(idx);
    keys[idx] = pred;
    C[idx]->deletion(pred);
  }
  else if (C[idx + 1]->n >= t) 
  {
    int succ = getSuccessor(idx);
    keys[idx] = succ;
    C[idx + 1]->deletion(succ);
  }
  else 
  {
    merge(idx);
    C[idx]->deletion(k);
  }
  return;
}

int BTreeNode::getPredecessor(int idx) 
{
  BTreeNode *cur = C[idx];
  while (!cur->leaf)
    cur = cur->C[cur->n];

  return cur->keys[cur->n - 1];
}

void BTreeNode::fill(int idx) 
{
  if (idx != 0 && C[idx - 1]->n >= t)
    borrowFromPrev(idx);

  else if (idx != n && C[idx + 1]->n >= t)
    borrowFromNext(idx);

  else {
    if (idx != n)
      merge(idx);
    else
      merge(idx - 1);
  }
  return;
}

// Borrow from previous
void BTreeNode::borrowFromPrev(int idx) 
{
  BTreeNode *child = C[idx];
  BTreeNode *sibling = C[idx - 1];

  for (int i = child->n - 1; i >= 0; --i)
    child->keys[i + 1] = child->keys[i];

  if (!child->leaf) 
  {
    for (int i = child->n; i >= 0; --i)
      child->C[i + 1] = child->C[i];
  }

  child->keys[0] = keys[idx - 1];

  if (!child->leaf)
    child->C[0] = sibling->C[sibling->n];

  keys[idx - 1] = sibling->keys[sibling->n - 1];

  child->n += 1;
  sibling->n -= 1;

  return;
}

void BTreeNode::borrowFromPrev(int idx) 
{
  BTreeNode *child = C[idx];
  BTreeNode *sibling = C[idx - 1];
  for (int i = child->n - 1; i >= 0; --i)
    child->keys[i + 1] = child->keys[i];
  if (!child->leaf) 
  {
    for (int i = child->n; i >= 0; --i)
      child->C[i + 1] = child->C[i];
  }
  child->keys[0] = keys[idx - 1];
  if (!child->leaf)
    child->C[0] = sibling->C[sibling->n];
  keys[idx - 1] = sibling->keys[sibling->n - 1];
  child->n += 1;
  sibling->n -= 1;
  return;
}

// Borrow from the next
void BTreeNode::borrowFromNext(int idx) 
{
  BTreeNode *child = C[idx];
  BTreeNode *sibling = C[idx + 1];

  child->keys[(child->n)] = keys[idx];

  if (!(child->leaf))
    child->C[(child->n) + 1] = sibling->C[0];

  keys[idx] = sibling->keys[0];

  for (int i = 1; i < sibling->n; ++i)
    sibling->keys[i - 1] = sibling->keys[i];

  if (!sibling->leaf) 
  {
    for (int i = 1; i <= sibling->n; ++i)
      sibling->C[i - 1] = sibling->C[i];
  }

  child->n += 1;
  sibling->n -= 1;

  return;
}
