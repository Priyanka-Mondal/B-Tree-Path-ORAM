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
  deletion(k,root);
  if (root->knum == 0) 
  {
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
  if (idx < node->knum && node->keys[idx] == k) 
  {
    if (node->isleaf)
    {
      node->removeFromLeaf(idx);
      bram->WriteBTreeNode(node->bid, node);
    }
    else
    {
      removeFromNonLeaf(idx,node);
    }
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
      fill(idx,node);
    if (flag && (idx > node->knum))
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
void BTree::removeFromNonLeaf(int idx, BTreeNode *&node)
{
  Bid k = node->keys[idx];
  BTreeNode* ci = bram->ReadBTreeNode(node->cbids[idx],node->cpos[idx],node->cpos[idx]);
  BTreeNode* ci1 = bram->ReadBTreeNode(node->cbids[idx+1],node->cpos[idx+1],node->cpos[idx+1]);
  if (ci->knum >= T) ////
  {
    Bid pred = getPredecessor(idx,node);
    node->keys[idx] = pred;
    bram->WriteBTreeNode(node->bid,node);
    deletion(pred,ci);
  }
  else if (ci1->knum >= T) 
  {
    Bid succ = getSuccessor(idx,node);
    node->keys[idx] = succ;
    bram->WriteBTreeNode(node->bid,node);
    deletion(succ,ci1);
  }
  else 
  {
    merge(idx,node);
    deletion(k,ci);
  }
  return;
}

Bid BTree::getPredecessor(int idx, BTreeNode* node) 
{
  BTreeNode *cur = bram->ReadBTreeNode(node->cbids[idx],node->cpos[idx],node->cpos[idx]);
  while (!cur->isleaf)
   cur=bram->ReadBTreeNode(cur->cbids[cur->knum],cur->cpos[cur->knum],cur->cpos[cur->knum]);

  return cur->keys[cur->knum-1];
}

Bid BTree::getSuccessor(int idx, BTreeNode* node)
{
BTreeNode *cur=bram->ReadBTreeNode(node->cbids[idx+1],node->cpos[idx+1],node->cpos[idx+1]);
    while (!cur->isleaf)
       cur=bram->ReadBTreeNode(cur->cbids[0],cur->cpos[0],cur->cpos[0]);
    return cur->keys[0];
}

void BTree::fill(int idx, BTreeNode *&node) 
{
if(idx!=0 && bram->ReadBTreeNode(node->cbids[idx-1],node->cpos[idx-1],node->cpos[idx-1])->knum >= T)
    borrowFromPrev(idx,node);

  else if (idx != node->knum && bram->ReadBTreeNode(node->cbids[idx-1],node->cpos[idx-1],node->cpos[idx-1])->knum >= T)
    borrowFromNext(idx,node);

  else {
    if (idx != node->knum)
      merge(idx,node);
    else
      merge(idx-1,node);
  }
  return;
}

// Borrow from previous
void BTree::borrowFromPrev(int idx, BTreeNode *&node) 
{
  BTreeNode *child = bram->ReadBTreeNode(node->cbids[idx],node->cpos[idx],node->cpos[idx]);
  BTreeNode *sibling = bram->ReadBTreeNode(node->cbids[idx-1],node->cpos[idx-1],node->cpos[idx-1]);

  for (int i = child->knum-1; i >= 0; --i)
    child->keys[i + 1] = child->keys[i];

  if (!child->isleaf) 
  {
    for (int i = child->knum; i >= 0; --i)
    {
      child->cbids[i + 1] = child->cbids[i];
      child->cpos[i + 1] = child->cpos[i];
    }
  }

  child->keys[0] = node->keys[idx - 1];

  if (!child->isleaf)
  {
    child->cbids[0] = sibling->cbids[sibling->knum];
    child->cpos[0] = sibling->cpos[sibling->knum];
  }

  node->keys[idx-1] = sibling->keys[sibling->knum-1];

  child->knum += 1;
  sibling->knum -= 1;
  bram->WriteBTreeNode(child->bid,child);
  bram->WriteBTreeNode(sibling->bid,sibling);
  bram->WriteBTreeNode(node->bid,node);

  return;
}


// Borrow from the next
void BTree::borrowFromNext(int idx, BTreeNode *&node) 
{
  BTreeNode *child = bram->ReadBTreeNode(node->cbids[idx],node->cpos[idx],node->cpos[idx]);
  BTreeNode *sibling = bram->ReadBTreeNode(node->cbids[idx+1],node->cpos[idx+1],node->cpos[idx+1]);

  child->keys[(child->knum)] = node->keys[idx];

  if (!(child->isleaf))
  {
    child->cbids[(child->knum) + 1] = sibling->cbids[0];
    child->cpos[(child->knum) + 1] = sibling->cpos[0];
  }

  node->keys[idx] = sibling->keys[0];

  for (int i = 1; i < sibling->knum; ++i)
  {
    sibling->keys[i - 1] = sibling->keys[i];
  }

  if (!sibling->isleaf) 
  {
    for (int i = 1; i <= sibling->knum; ++i)
    {
      sibling->cbids[i - 1] = sibling->cbids[i];
      sibling->cpos[i - 1] = sibling->cpos[i];
    }
  }

  child->knum += 1;
  sibling->knum -= 1;
  bram->WriteBTreeNode(child->bid,child);
  bram->WriteBTreeNode(sibling->bid,sibling);
  bram->WriteBTreeNode(node->bid,node);

  return;
}

void BTree::merge(int idx, BTreeNode *&node)
{
  BTreeNode *child = bram->ReadBTreeNode(node->cbids[idx],node->cpos[idx],node->cpos[idx]);
  BTreeNode *sibling = bram->ReadBTreeNode(node->cbids[idx+1],node->cpos[idx+1],node->cpos[idx+1]);

    // Pulling a key from the current node and inserting it into (t-1)th
    // position of C[idx]
    child->keys[T-1] = node->keys[idx];

    // Copying the keys from C[idx+1] to C[idx] at the end
    for (int i=0; i<sibling->knum; ++i)
        child->keys[i+T] = sibling->keys[i];

    // Copying the child pointers from C[idx+1] to C[idx]
    if (!child->isleaf)
    {
        for(int i=0; i<=sibling->knum; ++i)
	{
            child->cbids[i+T] = sibling->cbids[i];
            child->cpos[i+T] = sibling->cpos[i];
	}
    }

    // Moving all keys after idx in the current node one step before -
    // to fill the gap created by moving keys[idx] to C[idx]
    for (int i=idx+1; i<node->knum; ++i)
        node->keys[i-1] = node->keys[i];

    // Moving the child pointers after (idx+1) in the current node one
    // step before
    for (int i=idx+2; i<=node->knum; ++i)
    {
        node->cbids[i-1] = node->cbids[i];
        node->cpos[i-1] = node->cpos[i];
    }

    // Updating the key count of child and the current node
    child->knum += sibling->knum+1;
    node->knum--;

    // Freeing the memory occupied by sibling
    //delete(sibling); //??
    sibling->bid = 0;
    sibling->knum = 0;
  bram->WriteBTreeNode(child->bid,child);
  bram->WriteBTreeNode(sibling->bid,sibling);
  bram->WriteBTreeNode(node->bid,node);
    return;
}
