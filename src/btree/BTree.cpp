#include "BTree.h"

BTree::BTree(int maxSize, bytes<Key> key) : rd(), mt(rd()), dis(0, (pow(2, floor(log2(maxSize/Z))+1)-1)/2)
{
    bram = new BRAM(maxSize, key);
    totleaves = (pow(2, floor(log2(maxSize/Z))+1)-1)/2;
    cout <<"total leaves in Btree:(0.."<< totleaves<<")"<<totleaves+1<<endl;
    cout <<maxSize<<"--------------------------------------------"<<endl;
    brootKey = 0;
    brootPos = 0;

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


/*
//delete functions
BTreeNode* BTree::minValueBTreeNode(int rootKey, int rootPos, BTreeNode* rootroot)
{
	BTreeNode* curBTreeNode = oram->ReadBTreeNode(rootKey,rootPos,rootPos);
	if(curBTreeNode == NULL || curBTreeNode->key ==0)
		return rootroot;
	else
		return minValueBTreeNode(curBTreeNode->leftID,curBTreeNode->leftPos, curBTreeNode);
}


BTreeNode* BTree::parentOf(int parentKey, int ppos, int childKey, int cpos, int key)
{
	BTreeNode* parBTreeNode = oram->ReadBTreeNode(parentKey,ppos,ppos);
	//cout <<"parent key of child key is"<<parBTreeNode->key<<childKey<<endl;
	if(key == parBTreeNode->key)
		return NULL;
	else
	{
		BTreeNode* cBTreeNode = oram->ReadBTreeNode(childKey,cpos,cpos);
		if(key == cBTreeNode->key)
		{
			return parBTreeNode;
		}
		else if(key<cBTreeNode->key)
		{
		return parentOf(cBTreeNode->key,cBTreeNode->pos,cBTreeNode->leftID,cBTreeNode->leftPos,key);
		}
		else if(key>cBTreeNode->key)
		{
		return parentOf(cBTreeNode->key,cBTreeNode->pos,cBTreeNode->rightID,cBTreeNode->rightPos,key);
		}
	}
}




int BTree::balance(BTreeNode* node, int &pos)
{
    int key = node->key;
    int balance = getBalance(node);
    //cout <<"balance is:"<<balance<<endl;
    if (balance > 1 )
    {
	    BTreeNode* leftChild = oram->ReadBTreeNode(node->leftID,node->leftPos,node->leftPos);
    	    if(getBalance(leftChild)>=0)
	    {
	    	//cout <<"Left Left Case" <<endl;
        	BTreeNode* res = rightRotate(node);
        	pos = res->pos;
        	return res->key;
	    }
	    if(getBalance(leftChild)<0)
	    {
	    	 //cout <<"Left Right Case" <<endl;
		 BTreeNode* res = leftRotate(leftChild);
        	 node->leftID = res->key;
        	 node->leftPos = res->pos;
        	 oram->WriteBTreeNode(node->key, node);
		 BTreeNode* res2 = rightRotate(node);
		 pos = res2->pos;
		 return res2->key;
	    }
    }
    if(balance < -1)
    {
	    BTreeNode* rightChild=oram->ReadBTreeNode(node->rightID,node->rightPos,node->rightPos);
	    if(getBalance(rightChild)<=0)
	    {
	    	//cout <<"Right Right Case" <<endl;
		BTreeNode* res = leftRotate(node);
		pos = res->pos;
		return res->key;
	    }
	    if(getBalance(rightChild)>0)
	    {
	    	    //cout <<"Right Left Case" <<endl;
		    BTreeNode* res = rightRotate(rightChild);
		    node->rightID = res->key;
		    node->rightPos = res->pos;
		    oram->WriteBTreeNode(node->key,node);
		    BTreeNode* res2 = leftRotate(node);
		    pos = res2->pos;
		    return res2->key;
	    }
    }
    //oram->WriteBTreeNode(node->key, node);
    return node->key;
}



int BTree::deleteBTreeNode(BTreeNode* nodef)
{
	BTreeNode* free = newBTreeNode(0,"");
	oram->DeleteBTreeNode(nodef->key,free);
	return 0;
}


int BTree:: removeMain(int rootKey,int& pos, int delKey)
{
	if(rootKey != delKey || rootKey <delKey || rootKey > delKey)
	{// it does not work without these 3 checks ??
		//cout<<"rootKey != delKey(removeMain)"<<endl;
		BTreeNode* paren = parentOf(rootKey,pos,rootKey,pos,delKey);
		int delPos;
		if(delKey == paren->leftID)
			delPos = paren->leftPos;
		else if(delKey == paren->rightID)
			delPos = paren->rightPos;
		rootKey = removeDel(rootKey,pos,delKey,delPos,paren);
		return rootKey;
	}
	else if(rootKey == delKey)
	{
		//cout<<"rootKey == delKey (removeMain)"<<endl;
		rootKey = removeRoot(rootKey, pos);
		return rootKey;
	}
	else
	{
		cout<<"WHY this (removeMain)"<<endl;
		return rootKey;
	}
}


int BTree::removeRoot(int rootKey, int& pos)
{
	BTreeNode* delnode =oram->ReadBTreeNode(rootKey,pos,pos); 
	int delKey = rootKey;
BTreeNode* b=oram->ReadBTreeNode(delnode->rightID,delnode->rightPos,delnode->rightPos);
	BTreeNode* minnode;
	if(b == NULL || b->key == 0)
	{
		//cout << "the min value node is->"<< delnode->key<<endl;
		minnode = delnode;
	}
	else
	{
		BTreeNode* mn = minValueBTreeNode(b->key,b->pos,b);
		//cout << "the min value node is:"<< mn->key<<endl;
		minnode = oram->ReadBTreeNode(mn->key,mn->pos,mn->pos);
	}
	
	if(delKey == minnode->key)
	{//no right child of delKey
		//cout <<"FIRST CASE:rootKey == minnode->key"<< endl;
BTreeNode* lc = oram->ReadBTreeNode(delnode->leftID,delnode->leftPos,delnode->leftPos);
		if(lc == NULL || lc->key ==0)
			lc = newBTreeNode(0,"");
		pos = lc->pos;
		deleteBTreeNode(delnode);
		return lc->key;
	}
	else
	{

		BTreeNode* pm=parentOf(rootKey,pos,rootKey,pos,minnode->key);
		BTreeNode* parmin = oram->ReadBTreeNode(pm->key,pm->pos,pm->pos);
		//cout << "parent of minnode is:"<< parmin->key<<endl;
		if(delKey == parmin->key)
		{//no leftchild of minnode //minnode is delKey's right child
		//	cout <<"SECOND CASE: rootKey == parmin->key"<< endl;
	BTreeNode* lc = oram->ReadBTreeNode(delnode->leftID,delnode->leftPos,delnode->leftPos);
			if(lc == NULL || lc->key ==0)
				lc = newBTreeNode(0,"");
			minnode->leftID = lc->key;
			minnode->leftPos = lc->pos;
			minnode->height =  max(height(minnode->leftID,minnode->leftPos), height(minnode->rightID, minnode->rightPos)) + 1;
    			//oram->maxheight = max(minnode->height,oram->maxheight);
			oram->WriteBTreeNode(minnode->key,minnode);
			int minPos = minnode->pos;
			int minKey = balance(minnode,minnode->pos);
			minnode = oram->ReadBTreeNode(minKey,minPos,minPos);
			pos = minnode->pos;
			deleteBTreeNode(delnode);
			return minnode->key;
		}
		else //minnode does not have leftID in general
		{//in this case minnode is parmin's left child always
		//	cout <<"THIRD case"<< endl;	
	BTreeNode* rc = oram->ReadBTreeNode(minnode->rightID,minnode->rightPos,minnode->rightPos);
			if(rc == NULL || rc->key ==0)
				rc = newBTreeNode(0,"");
			parmin->leftID = rc->key;
			parmin->leftPos = rc->pos;
			parmin->height = max(height(parmin->leftID,parmin->leftPos), height(parmin->rightID, parmin->rightPos)) + 1;
    			//oram->maxheight = max(parmin->height,oram->maxheight);
			oram->WriteBTreeNode(parmin->key,parmin);
			minnode->leftID = delnode->leftID;
			minnode->leftPos = delnode->leftPos;
			minnode->rightID = delnode->rightID;
			minnode->rightPos = delnode->rightPos;
			minnode->height = max(height(minnode->leftID,minnode->leftPos), height(minnode->rightID, minnode->rightPos)) + 1;
    			//oram->maxheight = max(minnode->height,oram->maxheight);
			oram->WriteBTreeNode(minnode->key,minnode);
			int minPos = minnode->pos;
			int minKey = balanceDel(minnode->key,minnode->pos, parmin);
			minnode = oram->ReadBTreeNode(minKey,minPos,minPos);
			pos = minnode->pos;
			deleteBTreeNode(delnode);
			return minnode->key;
		}	
	}
}


int BTree::removeDel(int rootKey,int& pos,int delKey,int delPos,BTreeNode* paren)
{
	BTreeNode* node = oram->ReadBTreeNode(rootKey, pos, pos);
	if(node->key > paren->key)
	{
		node->leftID=removeDel(node->leftID,node->leftPos,delKey,delPos,paren);
	}
	else if(node->key < paren->key)
	{
	     node->rightID=removeDel(node->rightID,node->rightPos,delKey,delPos,paren);
	}
	else if(node->key == paren->key)
	{
		node->key = realDelete(node,delKey,delPos);// int& node->pos
		//paren->key
	}
//BALANCE:
    node->height = max(height(node->leftID, node->leftPos), height(node->rightID, node->rightPos)) + 1;
    //oram->maxheight = max(node->height,oram->maxheight);
    int balance = getBalance(node);
    //cout << "Balance is:"<<balance<<endl;
    int key = node->key;
    if (balance > 1 )
    {
	    BTreeNode* leftChild = oram->ReadBTreeNode(node->leftID,node->leftPos,node->leftPos);
    	    if(getBalance(leftChild)>=0)
	    {
	    	//cout <<"Left Left Case" <<endl;
        	BTreeNode* res = rightRotate(node);
        	pos = res->pos;
        	return res->key;
	    }
	    if(getBalance(leftChild)<0)
	    {
	    	 //cout <<"Left Right Case" <<endl;
		 BTreeNode* res = leftRotate(leftChild);
        	 node->leftID = res->key;
        	 node->leftPos = res->pos;
        	 oram->WriteBTreeNode(node->key, node);
		 BTreeNode* res2 = rightRotate(node);
		 pos = res2->pos;
		 return res2->key;
	    }
    }
    if(balance < -1)
    {
	    BTreeNode* rightChild=oram->ReadBTreeNode(node->rightID,node->rightPos,node->rightPos);
	    if(getBalance(rightChild)<=0)
	    {
	        //cout <<"Right Right Case" <<endl;
		BTreeNode* res = leftRotate(node);
		pos = res->pos;
		return res->key;
	    }
	    if(getBalance(rightChild)>0)
	    {
	            //cout <<"Right Left Case" <<endl;
		    BTreeNode* res = rightRotate(rightChild);
		    node->rightID = res->key;
		    node->rightPos = res->pos;
		    oram->WriteBTreeNode(node->key,node);
		    BTreeNode* res2 = leftRotate(node);
		    pos = res2->pos;
		    return res2->key;
	    }
    }

    oram->WriteBTreeNode(node->key, node);
    return node->key;
}



int BTree::realDelete(BTreeNode* paren,int delKey,int delPos)
{
	BTreeNode* delnode =oram->ReadBTreeNode(delKey,delPos,delPos); 
BTreeNode* b=oram->ReadBTreeNode(delnode->rightID,delnode->rightPos,delnode->rightPos);
	BTreeNode* minnode;
	if(b == NULL || b->key == 0)
	{
		//cout << "the min value node is->"<< delnode->key<<endl;
	minnode = oram->ReadBTreeNode(delnode->key,delnode->pos,delnode->pos);
	}
	else
	{
		BTreeNode* mn = minValueBTreeNode(b->key,b->pos,b);
		//cout << "the min value node is:"<< mn->key<<endl;
		minnode = oram->ReadBTreeNode(mn->key,mn->pos,mn->pos);
	}
BTreeNode* pm=parentOf(paren->key,paren->pos,paren->key,paren->pos,minnode->key);
	BTreeNode* parmin = oram->ReadBTreeNode(pm->key,pm->pos,pm->pos);
	//cout << "parent of minnode is:"<< parmin->key<<endl;
	if(delKey == minnode->key)//paren->key == parmin->key
	{//no right child of delKey
	//	cout <<"FIRST CASE:delKey == minnode->key"<< endl;
BTreeNode* lc = oram->ReadBTreeNode(delnode->leftID,delnode->leftPos,delnode->leftPos);
		if(lc == NULL || lc->key ==0)
			lc = newBTreeNode(0,"");
		if(paren->leftID == minnode->key)
		{
			paren->leftID = lc->key;
			paren->leftPos = lc->pos;
		}
		else if(paren->rightID == minnode->key)
		{
			paren->rightID = lc->key;
			paren->rightPos = lc->pos;
		}
		paren->height=max(height(paren->leftID,paren->leftPos), height(paren->rightID, paren->rightPos)) + 1;
    		//oram->maxheight = max(paren->height,oram->maxheight);
		oram->WriteBTreeNode(paren->key,paren);
		deleteBTreeNode(delnode);
		return paren->key;
	}
	else if(delKey == parmin->key)
	{//no leftchild of minnode //minnode is delKey right child
	//	cout <<"SECOND CASE: delKey == parmin->key"<< endl;
BTreeNode* lc = oram->ReadBTreeNode(delnode->leftID,delnode->leftPos,delnode->leftPos);
		if(lc == NULL || lc->key ==0)
			lc = newBTreeNode(0,"");
		minnode->leftID = lc->key;
		minnode->leftPos = lc->pos;
		minnode->height =  max(height(minnode->leftID,minnode->leftPos), height(minnode->rightID, minnode->rightPos)) + 1;
    		//oram->maxheight = max(minnode->height,oram->maxheight);
		oram->WriteBTreeNode(minnode->key,minnode);
		int minPos = minnode->pos;
		int minKey = balance(minnode,minnode->pos);
		minnode = oram->ReadBTreeNode(minKey,minPos,minPos);
		if(paren->leftID == delKey)
		{
			paren->leftID = minnode->key;
			paren->leftPos = minnode->pos;
		}
		else if(paren->rightID == delKey)
		{
			paren->rightID = minnode->key;
			paren->rightPos = minnode->pos;
		}
		paren->height=max(height(paren->leftID,paren->leftPos), height(paren->rightID, paren->rightPos)) + 1;
    		//oram->maxheight = max(paren->height,oram->maxheight);
		oram->WriteBTreeNode(paren->key,paren);
		deleteBTreeNode(delnode);
		return paren->key;
	}
	else //minnode does not have leftID in general
	{//in this case minnode is parmin's left child always
		//cout <<"THIRD case"<< endl;	
BTreeNode* rc = oram->ReadBTreeNode(minnode->rightID,minnode->rightPos,minnode->rightPos);
		if(rc == NULL || rc->key ==0)
			rc = newBTreeNode(0,"");
		parmin->leftID = rc->key;
		parmin->leftPos = rc->pos;
		parmin->height = max(height(parmin->leftID,parmin->leftPos), height(parmin->rightID, parmin->rightPos)) + 1;
    		//oram->maxheight = max(parmin->height,oram->maxheight);
		oram->WriteBTreeNode(parmin->key,parmin);
		minnode->leftID = delnode->leftID;
		minnode->leftPos = delnode->leftPos;
		minnode->rightID = delnode->rightID;
		minnode->rightPos = delnode->rightPos;
		minnode->height = max(height(minnode->leftID,minnode->leftPos), height(minnode->rightID, minnode->rightPos)) + 1;
    		//oram->maxheight = max(minnode->height,oram->maxheight);
		oram->WriteBTreeNode(minnode->key,minnode);
		int minPos = minnode->pos;
		int minKey = balanceDel(minnode->key,minnode->pos, parmin);
		minnode = oram->ReadBTreeNode(minKey,minPos,minPos);
		if(paren->leftID == delKey)
		{
			paren->leftID = minnode->key;
			paren->leftPos = minnode->pos;
		}
		else if(paren->rightID == delKey)
		{
			paren->rightID = minnode->key;
			paren->rightPos = minnode->pos;
		}
		paren->height=max(height(paren->leftID,paren->leftPos), height(paren->rightID, paren->rightPos)) + 1;
    		//oram->maxheight = max(paren->height,oram->maxheight);
		oram->WriteBTreeNode(paren->key,paren);
		deleteBTreeNode(delnode);
		return paren->key;
	}	
}


int BTree::balanceDel(int key, int& pos, BTreeNode* parmin)
{
	BTreeNode* node = oram->ReadBTreeNode(key,pos,pos);
	//cout <<"IN BALANCEDEL:"<< node->key<<endl;
	if(node->key < parmin->key)
	{
		node->rightID = balanceDel(node->rightID,node->rightPos,parmin);
	}
	else if(node->key > parmin->key)
	{
		node->leftID = balanceDel(node->leftID,node->leftPos,parmin);
	}
	//else if(node->key == parmin->key)
	//{

    node->height = max(height(node->leftID, node->leftPos), height(node->rightID, node->rightPos)) + 1;
    //oram->maxheight = max(node->height,oram->maxheight);
    int balance = getBalance(node);
    //cout <<"balance is:"<<balance<<endl;
    if (balance > 1 )
    {
	    BTreeNode* leftChild = oram->ReadBTreeNode(node->leftID,node->leftPos,node->leftPos);
    	    if(getBalance(leftChild)>=0)
	    {
	    	//cout <<"Left Left Case" <<endl;
        	BTreeNode* res = rightRotate(node);
        	pos = res->pos;
        	return res->key;
	    }
	    if(getBalance(leftChild)<0)
	    {
	    	 //cout <<"Left Right Case" <<endl;
		 BTreeNode* res = leftRotate(leftChild);
        	 node->leftID = res->key;
        	 node->leftPos = res->pos;
        	 oram->WriteBTreeNode(node->key, node);
		 BTreeNode* res2 = rightRotate(node);
		 pos = res2->pos;
		 return res2->key;
	    }
    }
    if(balance < -1)
    {
	    BTreeNode* rightChild=oram->ReadBTreeNode(node->rightID,node->rightPos,node->rightPos);
	    if(getBalance(rightChild)<=0)
	    {
	    	//cout <<"Right Right Case" <<endl;
		BTreeNode* res = leftRotate(node);
		pos = res->pos;
		return res->key;
	    }
	    if(getBalance(rightChild)>0)
	    {
	    	    //cout <<"Right Left Case" <<endl;
		    BTreeNode* res = rightRotate(rightChild);
		    node->rightID = res->key;
		    node->rightPos = res->pos;
		    oram->WriteBTreeNode(node->key,node);
		    BTreeNode* res2 = leftRotate(node);
		    pos = res2->pos;
		    return res2->key;
	    }
    }

  //}
    oram->WriteBTreeNode(node->key, node);
    return node->key;
}
void BTree::setupInsert(int& rootKey, int& rootPos, map<int, string> pairs) {
    for (auto pair : pairs) {
        BTreeNode* node = newBTreeNode(pair.first, pair.second);
        setupBTreeNodes.push_back(node);
    }
    cout << "Creating BST" << endl;
    sortedArrayToBST(0, setupBTreeNodes.size() - 1, rootPos, rootKey);
    cout << "Inserting in ORAM" << endl;
    oram->setupInsert(setupBTreeNodes);
}

int BTree::sortedArrayToBST(int start, int end, int& pos, int& node) {
    setupProgress++;
    if (setupProgress % 100000 == 0) {
        cout << setupProgress << "/" << setupBTreeNodes.size()*2 << " of AVL tree constructed" << endl;
    }
    if (start > end) {
        pos = -1;
        node = 0;
        return 0;
    }

    int mid = (start + end) / 2;
    BTreeNode *root = setupBTreeNodes[mid];

    int leftHeight = sortedArrayToBST(start, mid - 1, root->leftPos, root->leftID);

    int rightHeight = sortedArrayToBST(mid + 1, end, root->rightPos, root->rightID);
    root->pos = RandomPath();
    root->height = max(leftHeight, rightHeight) + 1;
    pos = root->pos;
    node = root->key;
    return root->height;
}
*/