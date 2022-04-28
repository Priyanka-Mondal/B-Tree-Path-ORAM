#include "BTreef.h"

BTreef::BTreef(int maxSize, bytes<Key> key) : rd(), mt(rd()), dis(0, (pow(2, floor(log2(maxSize/Z))+1)-1)/2)
{
    bram = new BRAMf(maxSize, key);
    totleaves = (pow(2, floor(log2(maxSize/Z))+1)-1)/2;
    cout <<"total leaves in Btree:(0.."<< totleaves<<")"<<totleaves+1<<endl;
    cout <<maxSize<<"--------------------------------------------"<<endl;
    brootKey = 0;
    brootPos = -1;

}
BTreef::~BTreef() 
{
    delete bram;
}

BTreeNodef* BTreef::newBTreeNodef(bool leaf, int nextbid, int leafpos) 
{
    BTreeNodef* node = new BTreeNodef();
    node->isleaf=leaf;
    node->bid = nextbid;
    node->pos = leafpos;
    node->knum = 0;
    node->height = 1;
    for(int i =0;i<D-1;i++)
    {
	    node->keys[i] = Bid(0); 
    	    std::fill(node->values[i].begin(), node->values[i].end(), 0);
	    node->cbids[i]=0;
	    node->cpos[i]=-1;
    }
    node->cbids[D-1]=0;
    node->cpos[D-1]=-1;
    return node;
}

int BTreef::nextBid()
{
	nextbid++;
	return nextbid;
}

int BTreef::insert(Bid kw, string blk)
{
	bram->start(false);
	brootKey = insertblk(kw,blk,brootKey,brootPos);
	bram->finalize(brootKey,brootPos);
}

int BTreef::insertblk(Bid kw, string blk, int brootKey, int &brootPos)
{
	if (brootKey == 0)
	{
		BTreeNodef *root = newBTreeNodef(true, nextBid(),RandomPath());
		root->keys[0] = kw;
    		std::copy(blk.begin(), blk.end(), root->values[0].begin());
//		root->values[0] = blk;
		root->knum = 1; 
		brootPos = root->pos;
		brootKey = root->bid;
		brootPos = bram->WriteBTreeNodef(root->bid, root);
		return brootKey;
	}
	else 
	{
		BTreeNodef *root = bram->ReadBTreeNodef(brootKey,brootPos);
		if (root->knum == 2*T-1)
		{
			BTreeNodef *s = newBTreeNodef(false,nextBid(),RandomPath());
			s->cbids[0] = root->bid;
			s->cpos[0] = root->pos;
			s->height = root->height+1;
			cout <<"BTree height is:"<<s->height<<endl;
			BTreeNodef *z = newBTreeNodef(root->isleaf, nextBid(),RandomPath());
			//s->splitChild(0, root, z);
			splitChild(s,0, root, z);
			bram->WriteBTreeNodef(z->bid,z);
			bram->WriteBTreeNodef(s->bid,s);
			bram->WriteBTreeNodef(root->bid,root);
			int i = 0;
			BTreeNodef *sc;
			if (s->keys[0] < kw)
				sc = s;
			else
				sc = root;
			insertNFull(kw,blk,sc);
			brootKey = s->bid;
			brootPos = s->pos;
		}
		else 
		{
			insertNFull(kw,blk,root);
		}
	}
return brootKey;
}
void BTreef::insertNFull(Bid kw,string blk, BTreeNodef*& node)
{
	int i = node->knum-1;
	if(node->isleaf == true)
	{
		while(i>=0 && node->keys[i]>kw)
		{
			node->keys[i+1] = node->keys[i];
std::copy(node->values[i+1].begin(), node->values[i+1].end(), node->values[i].begin());
			node->values[i+1] = node->values[i];
			i--;
		}
		node->keys[i+1] = kw;
                std::copy(blk.begin(), blk.end(), node->values[i+1].begin());
		//node->values[i+1] = blk;
		node->knum = node->knum+1;
		bram->WriteBTreeNodef(node->bid,node);
	}
	else
	{
		while(i>=0 && node->keys[i] > kw)
			i--;
      BTreeNodef *ci = bram->ReadBTreeNodef(node->cbids[i+1],node->cpos[i+1]);
		if (ci->knum == 2*T-1)
		{
			BTreeNodef *z = newBTreeNodef(ci->isleaf, nextBid(),RandomPath());
			//node->splitChild(i+1, ci, z);
			splitChild(node,i+1, ci, z);
			bram->WriteBTreeNodef(z->bid,z);
			bram->WriteBTreeNodef(ci->bid,ci);
			bram->WriteBTreeNodef(node->bid,node);
			if (node->keys[i+1] < kw)
			{
				ci = z;
			}
		}
		insertNFull(kw,blk,ci);
	}
}

void BTreef::splitChild(BTreeNodef *&par, int i, BTreeNodef *&y, BTreeNodef *&z)
{
	z->knum = T - 1;
	z->height = y->height;
	for (int j = 0; j < T-1; j++)
	{
		z->keys[j] = y->keys[j+T];
std::copy(y->values[j+T].begin(), y->values[j+T].end(), z->values[j].begin());
		//z->values[j] = y->values[j+T];
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
std::copy(par->values[j].begin(), par->values[j].end(), par->values[j+1].begin());
		//par->values[j+1] = par->values[j];
	}
	par->keys[i] = y->keys[T-1];
std::copy(par->values[T-1].begin(), par->values[T-1].end(), par->values[i].begin());
//	par->values[i] = y->values[T-1];
	par->knum = par->knum + 1;
}

string BTreef::search(Bid kw)
{
	bram->start(false);
	string res = "";
	bram->searchf_bytes = 0;
	searchkw(brootKey,brootPos,kw,res);
	bram->finalize(brootKey,brootPos);
    	searchf_bytes = bram->searchf_bytes;
	return res;
}

void BTreef::searchkw(int rootKey, int rootPos, Bid kw, string &res)
{
	if(res != "")
	     return;
	BTreeNodef* node = bram->ReadBTreeNodef(rootKey,rootPos);
        int i = 0;
        while (i < node->knum && kw > node->keys[i])
	{
                i++;
	}
        if (node->keys[i] == kw)
	{
		//res = node->values[i];
        	res.assign(node->values[i].begin(), node->values[i].end());
                res = res.c_str();
                return ;//node->values[i];
	}
	else if (node->isleaf == true)
	{
		cout <<"NOT Found in BTree"<<endl;
                return; //0;
	}
        searchkw(node->cbids[i],node->cpos[i],kw,res);
}

int BTreef::RandomPath() 
{
    int val = dis(mt);
    return val;
}

