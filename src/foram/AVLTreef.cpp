#include "AVLTreef.h"

AVLTreef::AVLTreef(int maxSize, bytes<Key> key) : rd(), mt(rd()), dis(0, (pow(2, floor(log2(maxSize / Z)) + 1) - 1) / 2) {
    oram = new ORAMf(maxSize, key);
}

AVLTreef::~AVLTreef() {
    delete oram;
}

// A utility function to get height of the tree

int AVLTreef::height(Bid key, int& leaf) {
    if (key == 0)
        return 0;
    Nodef* node = oram->ReadNodef(key, leaf, leaf);
    return node->height;
}

// A utility function to get maximum of two integers

int AVLTreef::max(int a, int b) {
    return (a > b) ? a : b;
}

/* Helper function that allocates a new node with the given key and
   NULL left and right pointers. */
Nodef* AVLTreef::newNodef(Bid key, string value) {
    Nodef* node = new Nodef();
    node->key = key;
    std::fill(node->value.begin(), node->value.end(), 0);
    std::copy(value.begin(), value.end(), node->value.begin());
    node->leftID = 0;
    node->rightID = 0;
    node->pos = RandomPath();
    node->height = 1; // new node is initially added at leaf
    return node;
}

// A utility function to right rotate subtree rooted with y
// See the diagram given above.

Nodef* AVLTreef::rightRotate(Nodef* y) {
cout <<"ONE"<<endl;
    Nodef* x = oram->ReadNodef(y->leftID,y->leftPos,y->leftPos);
    Nodef* T2;
    if (x->rightID == 0) {
        T2 = newNodef(0, "");
    } else {
        T2 = oram->ReadNodef(x->rightID,x->rightPos,x->rightPos);
    }
    // Perform rotation
    x->rightID = y->key;
    x->rightPos = y->pos;
    y->leftID = T2->key;
    y->leftPos = T2->pos;

    // Update heights
    y->height = max(height(y->leftID, y->leftPos), height(y->rightID, y->rightPos)) + 1;
    oram->WriteNodef(y->key, y);
    x->height = max(height(x->leftID, x->leftPos), height(x->rightID, x->rightPos)) + 1;
    oram->WriteNodef(x->key, x);
    // Return new root
    return x;
}

// A utility function to left rotate subtree rooted with x
// See the diagram given above.

Nodef* AVLTreef::leftRotate(Nodef* x) {
    Nodef* y = oram->ReadNodef(x->rightID,x->rightPos,x->rightPos);
    Nodef* T2;
    if (y->leftID == 0) {
        T2 = newNodef(0, "");
    } else {
        T2 = oram->ReadNodef(y->leftID,y->leftPos,y->leftPos);
    }


    // Perform rotation
    y->leftID = x->key;
    y->leftPos = x->pos;
    x->rightID = T2->key;
    x->rightPos = T2->pos;

    // Update heights
    x->height = max(height(x->leftID, x->leftPos), height(x->rightID, x->rightPos)) + 1;
    oram->WriteNodef(x->key, x);
    y->height = max(height(y->leftID, y->leftPos), height(y->rightID, y->rightPos)) + 1;
    oram->WriteNodef(y->key, y);
    // Return new root
    return y;
}

// Get Balance factor of node N

int AVLTreef::getBalance(Nodef* N) {
    if (N == NULL)
        return 0;
    return height(N->leftID, N->leftPos) - height(N->rightID, N->rightPos);
}




Bid AVLTreef::insert(Bid rootKey, int& pos, Bid key, string value) {
    /* 1. Perform the normal BST rotation */
    if (rootKey == 0) {
        Nodef* nnode = newNodef(key, value);
        pos = oram->WriteNodef(key, nnode);
	    //cout<<"(writing key at root):"<<key << endl;
        return nnode->key;
    }
    Nodef* node = oram->ReadNodef(rootKey, pos, pos);
    if (key < node->key) {
	    //cout<<"(insert key < cur node):"<<key << node->key<< endl;
        node->leftID = insert(node->leftID, node->leftPos, key, value);
    } else if (key > node->key) {
	    //cout<<"(insert key > cur node):"<<key << node->key<< endl;
        node->rightID = insert(node->rightID, node->rightPos, key, value);
    } else { // this one updates exixting value
	    //cout<<"(insert key == cur node):"<<key << node->key<< endl;
        std::fill(node->value.begin(), node->value.end(), 0);
        std::copy(value.begin(), value.end(), node->value.begin());
        oram->WriteNodef(rootKey, node);
        return node->key;
    }

    /* 2. Update height of this ancestor node */
    node->height = max(height(node->leftID, node->leftPos), height(node->rightID, node->rightPos)) + 1;

    /* 3. Get the balance factor of this ancestor node to check whether
       this node became unbalanced */
    int balance = getBalance(node);

    // If this node becomes unbalanced, then there are 4 cases

    // Left Left Case
    if (balance > 1 && key < oram->ReadNodef(node->leftID)->key) {
        Nodef* res = rightRotate(node);
        pos = res->pos;
        return res->key;
    }

    // Right Right Case
    if (balance < -1 && key > oram->ReadNodef(node->rightID)->key) {
        Nodef* res = leftRotate(node);
        pos = res->pos;
        return res->key;
    }

    // Left Right Case
    if (balance > 1 && key > oram->ReadNodef(node->leftID)->key) {
        Nodef* res = leftRotate(oram->ReadNodef(node->leftID));
        node->leftID = res->key;
        node->leftPos = res->pos;
        oram->WriteNodef(node->key, node);
        Nodef* res2 = rightRotate(node);
        pos = res2->pos;
        return res2->key;
    }

    // Right Left Case
    if (balance < -1 && key < oram->ReadNodef(node->rightID)->key) {
        auto res = rightRotate(oram->ReadNodef(node->rightID));
        node->rightID = res->key;
        node->rightPos = res->pos;
        oram->WriteNodef(node->key, node);
        auto res2 = leftRotate(node);
        pos = res2->pos;
        return res2->key;
    }

    /* return the (unchanged) node pointer */
    oram->WriteNodef(node->key, node);
    return node->key;
}

/**
 * a recursive search function which traverse binary tree to find the target node
 */
Nodef* AVLTreef::search(Nodef* head, Bid key) {
    if (head == NULL || head->key == 0)
        return head;
    head = oram->ReadNodef(head->key, head->pos, head->pos);
    if (head->key > key) {
	    //cout << "in search:"<< head->key << endl;
        return search(oram->ReadNodef(head->leftID, head->leftPos, head->leftPos), key);
    } else if (head->key < key) {
	    //cout << "in search:"<< head->key << endl;
        return search(oram->ReadNodef(head->rightID, head->rightPos, head->rightPos), key);
    } else
        return head;
}

/**
 * a recursive search function which traverse binary tree to find the target node
 */
void AVLTreef::batchSearch(Nodef* head, vector<Bid> keys, vector<Nodef*>* results) {
    if (head == NULL || head->key == 0) {
        return;
    }
    head = oram->ReadNodef(head->key, head->pos, head->pos);
    bool getLeft = false, getRight = false;
    vector<Bid> leftkeys,rightkeys;
    for (Bid bid : keys) {
        if (head->key > bid) {
            getLeft = true;
            leftkeys.push_back(bid);
        }
        if (head->key < bid) {
            getRight = true;
            rightkeys.push_back(bid);
        }
        if (head->key == bid) {
            results->push_back(head);
        }
    }
    if (getLeft) {
        batchSearch(oram->ReadNodef(head->leftID, head->leftPos, head->leftPos), leftkeys, results);
    }
    if (getRight) {
        batchSearch(oram->ReadNodef(head->rightID, head->rightPos, head->rightPos), rightkeys, results);
    }
}

void AVLTreef::printTree(Nodef* root, int indent) {
    if (root != 0 && root->key != 0) {
        root = oram->ReadNodef(root->key, root->pos, root->pos);
	//cout <<"root"<<root->key<<endl;
        if (root->leftID != 0)
            printTree(oram->ReadNodef(root->leftID, root->leftPos, root->leftPos), indent + 4);
        if (indent > 0)
            cout << setw(indent) << " ";
        string value;
        value.assign(root->value.begin(), root->value.end());
        cout << root->key << ":" << value.c_str() << endl;
        if (root->rightID != 0)
            printTree(oram->ReadNodef(root->rightID, root->rightPos, root->rightPos), indent + 4);

    }
}

/*
 * before executing each operation, this function should be called with proper arguments
 */
void AVLTreef::startOperation(bool batchWrite) {
    oram->start(batchWrite);
}

/*
 * after executing each operation, this function should be called with proper arguments
 */
void AVLTreef::finishOperation(bool find, Bid& rootKey, int& rootPos) {
    oram->finilize(find, rootKey, rootPos);
}

int AVLTreef::RandomPath() {
    int val = dis(mt);
    return val;
}

Nodef* AVLTreef::minValueNode(Bid rootKey, int rootPos, Nodef* rootroot)
{
	Nodef* curNode = oram->ReadNodef(rootKey,rootPos,rootPos);
	if(curNode == NULL || curNode->key ==0)
	{
		return rootroot;
	}
	else
	{
		return minValueNode(curNode->leftID,curNode->leftPos, curNode);
	}

}


Nodef* AVLTreef::parentOf(Bid parentKey, int ppos, Bid childKey, int cpos, Bid key)
{
	Nodef* parNode = oram->ReadNodef(parentKey,ppos,ppos);
	if(key == parNode->key)
		return NULL;
	else
	{
		Nodef* cNode = oram->ReadNodef(childKey,cpos,cpos);
		if(key == cNode->key)
		{
			return parNode;
		}
		else if(key<cNode->key)
		{
		return parentOf(cNode->key,cNode->pos,cNode->leftID,cNode->leftPos,key);
		}
		else if(key>cNode->key)
		{
		return parentOf(cNode->key,cNode->pos,cNode->rightID,cNode->rightPos,key);
		}
	}
}

Bid AVLTreef::balance(Nodef* node, int &pos)
{
    int balance = getBalance(node);
    if(balance<-1 || balance >1)
    {
    cout <<"balance of"<<node->key<<"is:"<< balance<< endl;
    Bid key = node->key;
    Nodef* nl = oram->ReadNodef(node->leftID,node->leftPos,node->leftPos);
    Nodef* nr = oram->ReadNodef(node->rightID,node->rightPos,node->rightPos);
    if(nl!=NULL)
	    cout <<"nl:" <<nl->key<<endl;
    if(nr!=NULL)
	    cout<<"nr:"<< nr->key<<endl;
    // If this node becomes unbalanced, then there are 4 cases

    // Left Left Case
    if (balance > 1 && key < nl->key) {
        Nodef* res = rightRotate(node);
        pos = res->pos;
        return res->key;
    }

    // Right Right Case
    if (balance < -1 && key > nr->key) {
        Nodef* res = leftRotate(node);
        pos = res->pos;
        return res->key;
    }

    // Left Right Case
    if (balance > 1 && key > nl->key) {
        Nodef* res = leftRotate(nl);
        node->leftID = res->key;
        node->leftPos = res->pos;
        oram->WriteNodef(node->key, node);
        Nodef* res2 = rightRotate(node);
        pos = res2->pos;
        return res2->key;
    }

    // Right Left Case
    if (balance < -1 && key < nr->key) {
        auto res = rightRotate(nr);
	cout <<"res->key:"<< res->key<<endl;
        node->rightID = res->key;
        node->rightPos = res->pos;
        oram->WriteNodef(node->key, node);
        auto res2 = leftRotate(node);
	cout << "res2->key:"<< res2->key<<endl;
        pos = res2->pos;
        return res2->key;
    }
    }
   return node->key;
}

Bid AVLTreef::balanceRec(Bid rootKey, int &rootPos, Nodef* node, int &pos)
{
    int balance = getBalance(node);
    Bid key = node->key;
    Nodef* res2;
    // If this node becomes unbalanced, then there are 4 cases
    // Left Left Case
    if (balance > 1 && key < oram->ReadNodef(node->leftID)->key) {
        res2 = rightRotate(node);
        pos = res2->pos;
        //return res2->key;
    }

    // Right Right Case
    else if (balance < -1 && key > oram->ReadNodef(node->rightID)->key) {
        res2 = leftRotate(node);
        pos = res2->pos;
        //return res2->key;
    }

    // Left Right Case
    else if (balance > 1 && key > oram->ReadNodef(node->leftID)->key) {
        Nodef* res = leftRotate(oram->ReadNodef(node->leftID));
        node->leftID = res->key;
        node->leftPos = res->pos;
        oram->WriteNodef(node->key, node);
        res2 = rightRotate(node);
        pos = res2->pos;
        //return res2->key;
    }

    // Right Left Case
    else if (balance < -1 && key < oram->ReadNodef(node->rightID)->key) {
        auto res = rightRotate(oram->ReadNodef(node->rightID));
        node->rightID = res->key;
        node->rightPos = res->pos;
        oram->WriteNodef(node->key, node);
        res2 = leftRotate(node);
        pos = res2->pos;
        //return res2->key;
    }

    if(res2->key == rootKey)// everything except rootKey is balanced
	    return res2->key;
    else
    {
	    Nodef* parent = parentOf(rootKey,rootPos,rootKey,rootPos,res2->key);
	    balanceRec(rootKey, rootPos, parent, parent->pos);
    }
    return res2->key;
}
Nodef* AVLTreef::balanceAndAttachtoParent(Nodef* parpm, Nodef* pm)
{
	int pmpos = pm->pos;
	Bid bpm = balance(pm,pmpos);
	pm=oram->ReadNodef(bpm,pmpos,pmpos);
	oram->WriteNodef(pm->key,pm);
	if(parpm != NULL)
	{
		if(pm->key<parpm->key)
		{
			parpm->leftID = pm->key;
			parpm->leftPos = pm->pos;
		}
		else if(pm->key > parpm->key)
		{
			parpm->rightID = pm->key;
			parpm->rightPos = pm->pos;
		}
		oram->WriteNodef(parpm->key,parpm);
	}
	return pm;
}


void AVLTreef::deleteNode(Nodef* nodef)
{
	Nodef* free = newNodef(0,"");
	oram->WriteNodef(nodef->key,free);
}

Bid AVLTreef::remove(Bid rootKey, int& pos, Bid delKey)
{
	cout <<"root At the beginning " << rootKey<< endl;
	Nodef* nodef = search(oram->ReadNodef(rootKey,pos,pos),delKey);
	cout << " delKey:" << nodef->key <<endl;
       Nodef* b=oram->ReadNodef(nodef->rightID,nodef->rightPos,nodef->rightPos);
	Nodef* minnode;
	Nodef* node ;//= oram->ReadNodef(rootKey,pos,pos);
	Nodef* c;
	string value ="";
	if(b == NULL || b->key == 0)
	{
		cout << "the min value node is->"<< nodef->key<<endl;
		minnode = newNodef(nodef->key,value);
		node = oram->ReadNodef(nodef->key,nodef->pos,nodef->pos);
	}
	else
	{
		minnode = minValueNode(b->key,b->pos,b);
		cout << "the min value node is:"<< minnode->key<<endl;
		node = oram->ReadNodef(minnode->key,minnode->pos,minnode->pos);
	}
	Nodef* parent = parentOf(rootKey, pos, rootKey, pos, nodef->key);
	Nodef* parmin = parentOf(rootKey,pos,rootKey,pos,node->key);
	//1. parent = NULL & parmin = NULL -> delnode's right subtree is NULL
	//2. parent = NULL & parmin != NULL  
	//4. parent != NULL & parmin != NULL
	//	4a. parmin == delkey ?/*
	if(parent == NULL && parmin == NULL)
	{//node->leftID becomes root
		cout << "parent == NULL && parmin == NULL"<< endl;
		node=oram->ReadNodef(node->leftID,node->leftPos,node->leftPos);
	}
	else if(parent == NULL && parmin != NULL)
	{
		cout <<"parent==NULL && parmin!=NULL"<<endl;
		if(parmin->key != delKey)
		{
			Nodef* tr = oram->ReadNodef(node->rightID,node->rightPos,node->rightPos);//minnode->rightID
			Nodef* pm = oram->ReadNodef(parmin->key,parmin->pos,parmin->pos);
			if(tr ==NULL || tr->key ==0)
			{
				tr = newNodef(0,"");
			}
				pm->leftID = tr->key;//tr->key<parmin->key ever
				pm->leftPos = tr->pos;
			pm->height = max(height(pm->leftID, pm->leftPos), height(pm->rightID, pm->rightPos)) + 1;
			Nodef* parpm =parentOf(rootKey,pos,rootKey,pos,pm->key);
			pm=balanceAndAttachtoParent(parpm,pm);
			
			Nodef* nr = oram->ReadNodef(nodef->rightID,nodef->rightPos,nodef->rightPos);
			if(nr ==NULL || nr->key==0)
			{
				nr = newNodef(0,"");
			}
			node->rightID = nr->key;
			node->rightPos = nr->pos;
			Nodef* nl = oram->ReadNodef(nodef->leftID,nodef->leftPos,nodef->leftPos);
			if(nl ==NULL || nl->key == 0) // && before 
			{
				nl = newNodef(0,"");
			}
			node->leftID = nl->key;
			node->leftPos = nl->pos; 
			node->height = max(height(node->leftID, node->leftPos), height(node->rightID, node->rightPos)) + 1;
			int pos = node->pos; // As parent = NULL
			//balanceAndAttachtoParent(pm,node); //Not required
			Bid nodekey = balance(node,node->pos);
			//node = oram->ReadNode(nodekey,no)
			//oram->WriteNodef(node->,node);
			deleteNode(nodef);
			return nodekey;

		}
		else if (parmin->key == delKey)
		{
			Nodef* nl=oram->ReadNodef(nodef->leftID,nodef->leftPos,nodef->leftPos);
			Nodef* nr=oram->ReadNodef(nodef->rightID,nodef->rightPos,nodef->rightPos);
			if(node->key > delKey)
			{
				node->leftID = nl->key;
				node->leftPos = nl->pos;
			}
			else if(node->key < delKey)//not required
			{
				node->rightID = nr->key;
				node->rightPos = nr->pos;
			}
			node->height = max(height(node->leftID, node->leftPos), height(node->rightID, node->rightPos)) + 1;
			//pos = node->pos;
			Bid nodekey = balance(node,node->pos);
			oram->WriteNodef(node->key,node);
			deleteNode(nodef);
			return nodekey;
		}
		else
		{//*? need code here
			cout << "Not sure why I am here"<< endl;
			//node = oram->ReadNodef(rootKey,pos,pos);
		}
	}
	else if(parent!=NULL && parmin!=NULL)
	{
		cout <<"parent!=NULL && parmin!=NULL"<<endl;
		if(parmin->key == delKey)
		{
			cout << "parmin->key == delKey"<<parmin->key<<nodef->key<<node->key<<endl;
			Nodef* par=oram->ReadNodef(parent->key,parent->pos,parent->pos);			
			Nodef* delLeft = oram->ReadNodef(nodef->leftID, nodef->leftPos, nodef->leftPos);
			if(delLeft == NULL || delLeft->key == 0)
			{
				delLeft = newNodef(0,"");
			}
			if(par->key < delKey) // delkey is at right subtree
			{
				node->leftID = delLeft->key;
				node->leftPos = delLeft->pos;
				node->height = max(height(node->leftID, node->leftPos), height(node->rightID, node->rightPos)) + 1;
				//oram->WriteNodef(node->key,node);
				node = balanceAndAttachtoParent(parmin,node);
				cout << "Node key:"<< node->key;
				if(nodef->key > par->key)
				{	
					par->rightID = node->key;
					par->rightPos = node->pos;
				}
				else if(nodef->key < par->key)
				{
					par->leftID = node->key;
					par->leftPos = node->pos;
				}
			cout << "par node left"<<par->key<<par->leftID<<endl;
				par->height = max(height(par->leftID, par->leftPos), height(par->rightID, par->rightPos)) + 1; 
				Nodef* ppar = parentOf(rootKey,pos,rootKey,pos,par->key);
				oram->WriteNodef(par->key,par);
				//par = balanceAndAttachtoParent(ppar,par);
				//*? Gives error
				deleteNode(nodef);
				if(par->key!=rootKey)
				{
					return rootKey;
				}
				else if(par->key==rootKey)
				{
					pos = par->pos;
					node = oram->ReadNodef(par->key,par->pos,par->pos);
					return node->key;
				}
			}
			else if(par->key >delKey) //seg fault happens after adding this block
			{ 	cout <<"par->key>delKey"<< par->key<< delKey;
				node->leftID = delLeft->key;
				node->leftPos = delLeft->pos;
				node->height = max(height(node->leftID, node->leftPos), height(node->rightID, node->rightPos)) + 1;
				//oram->WriteNodef(node->key,node);
				node = balanceAndAttachtoParent(parmin,node);
				cout << "Node key:"<< node->key;
				if(nodef->key > par->key)
				{	
					par->rightID = node->key;
					par->rightPos = node->pos;
				}
				else if(nodef->key < par->key)
				{
					par->leftID = node->key;
					par->leftPos = node->pos;
				}
			cout << "par node left"<<par->key<<par->leftID<<endl;
				par->height = max(height(par->leftID, par->leftPos), height(par->rightID, par->rightPos)) + 1; 
				Nodef* ppar = parentOf(rootKey,pos,rootKey,pos,par->key);
				//oram->WriteNodef(par->key,par);
				par = balanceAndAttachtoParent(ppar,par);
				//*? Gives error
				deleteNode(nodef);
				if(par->key!=rootKey)
				{
					return rootKey;
				}
				else if(par->key==rootKey)
				{
					pos = par->pos;
					node = oram->ReadNodef(par->key,par->pos,par->pos);
					return node->key;
				}
			}
		}
		else if(delKey == node->key) //delkey has no right child
		{
			cout << "delKey == node->key"<<endl;
			Nodef* pm = oram->ReadNodef(parmin->key,parmin->pos,parmin->pos);
			Nodef* lc = oram->ReadNodef(node->leftID,node->leftPos,node->leftPos);
			if(lc==NULL || lc->key == 0)
				lc = newNodef(0,"");
			if(pm->key < delKey)
			{
				pm->rightID = lc->key;
				pm->rightPos = lc->pos;
			}
			else if(pm->key > delKey)
			{
				pm->leftID = lc->key;
				pm->leftPos = lc->pos;
			}
			pm->height = max(height(pm->leftID, pm->leftPos), height(pm->rightID, pm->rightPos)) + 1;
			//oram->WriteNodef(pm->key,pm);
			Nodef* ppm = parentOf(rootKey,pos,rootKey,pos,pm->key);	
			pm = balanceAndAttachtoParent(ppm,pm);
			deleteNode(nodef);
			if(pm->key!=rootKey)
				return rootKey;
			else if(pm->key==rootKey)
			{
				node =oram->ReadNodef(pm->key,pm->pos,pm->pos);
				return node->key;
			}
		}
		else // if(what condition?)
		{
			Nodef* mrc = oram->ReadNodef(node->rightID,node->rightPos,node->rightPos);
			if(mrc==NULL || mrc->key == 0)
				mrc= newNodef(0,"");
			Nodef* par = oram->ReadNodef(parent->key,parent->pos,parent->pos);
			Nodef* pm = oram->ReadNodef(parmin->key,parmin->pos,parmin->pos);
			cout <<"node right key is:"<<node->rightID<<endl;
			if(node->key > pm->key)
			{
				pm->rightID = mrc->key;
				pm->rightPos = mrc->pos;
			}
			else if(node->key < pm->key)
			{
				pm->leftID = mrc->key;
				pm->leftPos = mrc->pos;
			}
			pm->height = max(height(pm->leftID, pm->leftPos), height(pm->rightID, pm->rightPos)) + 1;
		//	oram->WriteNodef(pm->key,pm);
			//**HERE
			Nodef* parpm =parentOf(rootKey,pos,rootKey,pos,pm->key);
			pm = balanceAndAttachtoParent(parpm, pm);
			
			Nodef* lc = oram->ReadNodef(nodef->leftID,nodef->leftPos,nodef->leftPos);
			if(lc==NULL || lc->key == 0)
				lc= newNodef(0,"");
			Nodef* rc = oram->ReadNodef(nodef->rightID,nodef->rightPos,nodef->rightPos);
			if(rc==NULL || rc->key == 0)
				rc= newNodef(0,"");
			
			node->leftID = lc->key;
			node->leftPos = lc->pos;
			node->rightID = rc->key;
			node->rightPos = rc->pos;
			node->height = max(height(node->leftID, node->leftPos), height(node->rightID, node->rightPos)) + 1;
			oram->WriteNodef(node->key, node);
			int nodepos = node->pos;
			Bid bnode = balance(node,nodepos);
			node = oram->ReadNodef(bnode,nodepos,nodepos);
			//here
			if(nodef->key < par->key)//replacing nodef with node
			{
				par->leftID = node->key;
				par->leftPos = node->pos;
			}
			else if(nodef->key > par->key)
			{
				par->rightID = node->key;
				par->rightPos = node->pos;
			}
			par->height = max(height(par->leftID, par->leftPos), height(par->rightID, par->rightPos)) + 1;
			//oram->WriteNodef(par->key,par);
			int parpos = par->pos;
			Bid bpar = balance(par,parpos); //balanceRec
			node = oram->ReadNodef(bpar,parpos,parpos);
			deleteNode(nodef);
			if(node->key !=rootKey)
				return rootKey;
			else 
				return node->key;
		}
	}
	else
	{
		cout <<"This LAST case of remove should never execute"<< endl;
		//node = oram->ReadNodef(rootKey,pos,pos);
	}
return rootKey;	
}

Bid AVLTreef::balanceParmin(Bid key, int& pos, Nodef* parmin, Nodef* minnode)
{
	if(parmin == NULL || parmin->key == 0)
		return 0;
	if(key == parmin->leftID || key == parmin->rightID)
	{
		Nodef* mlc = oram->ReadNodef(minnode->leftID,minnode->leftPos,minnode->leftPos);
		if(mlc == NULL || mlc->key == 0)
		     mlc = newNodef(0,"");
		return mlc->key;
	}

	Nodef* node = oram->ReadNodef(key, pos, pos);
	if(parmin->key < node->key)
	{
	  node->leftID=balanceParmin(node->leftID,node->leftPos,parmin,minnode);
	}
	else if(parmin->key > node->key)
	{
	  node->rightID=balanceParmin(node->rightID,node->rightPos,parmin,minnode);
	}
	else if (parmin->key == node->key)
	{
		Nodef* mrc = oram->ReadNodef(minnode->rightID,minnode->rightPos,minnode->rightPos);
			
		if(mrc == NULL || mrc->key == 0)
				mrc = newNodef(0,"");
		
		if(minnode->key < parmin->key)
		{
			parmin->leftID = mrc->key;
			parmin->leftPos = mrc->pos;
		}
		else if(minnode->key > parmin->key)
		{
			parmin->rightID = mrc->key;
			parmin->rightPos = mrc->pos;
		}
		parmin->height =  max(height(parmin->leftID, parmin->leftPos), height(parmin->rightID, parmin->rightPos)) + 1;
		int parminpos = parmin->pos;
		Bid bparmin = balance(parmin,parminpos);
		parmin = oram->ReadNodef(bparmin,parminpos,parminpos);
		oram->WriteNodef(parmin->key,parmin);
		return parmin->key; //node
	}
    
	node->height = max(height(node->leftID, node->leftPos), height(node->rightID, node->rightPos)) + 1;

    int balance = getBalance(node);

    // Left Left Case
    if (balance > 1 && key < oram->ReadNodef(node->leftID)->key) {
        Nodef* res = rightRotate(node);
        pos = res->pos;
        return res->key;
    }

    // Right Right Case
    if (balance < -1 && key > oram->ReadNodef(node->rightID)->key) {
        Nodef* res = leftRotate(node);
        pos = res->pos;
        return res->key;
    }

    // Left Right Case
    if (balance > 1 && key > oram->ReadNodef(node->leftID)->key) {
        Nodef* res = leftRotate(oram->ReadNodef(node->leftID));
        node->leftID = res->key;
        node->leftPos = res->pos;
        oram->WriteNodef(node->key, node);
        Nodef* res2 = rightRotate(node);
        pos = res2->pos;
        return res2->key;
    }

    // Right Left Case
    if (balance < -1 && key < oram->ReadNodef(node->rightID)->key) {
        auto res = rightRotate(oram->ReadNodef(node->rightID));
        node->rightID = res->key;
        node->rightPos = res->pos;
        oram->WriteNodef(node->key, node);
        auto res2 = leftRotate(node);
        pos = res2->pos;
        return res2->key;
    }

    /* return the (unchanged) node pointer */
    oram->WriteNodef(node->key, node);
    return node->key;
}

Bid AVLTreef::removenaive(Bid rootKey, int& pos,Bid delKey)
{
	Nodef* node = oram->ReadNodef(rootKey,pos,pos);
	if(node->key < delKey)
	{
		cout <<"node->key < delKey"<< node->key<< delKey<<endl;
	      node->rightID = removenaive(node->rightID,node->rightPos,delKey);
	}
	else if(node->key > delKey)
	{
		cout <<"node->key > delKey"<< node->key<< delKey<<endl;
	      node->leftID = removenaive(node->leftID,node->leftPos,delKey);
	}
	else if (node->key == delKey)
	{
		cout <<"node->key == delKey"<< node->key << delKey<<endl;
		node = newNodef(0,"");
		oram->DeleteNodef(node->key,node);
		return node->key;
	}
	return node->key;
}
Bid AVLTreef::callRemove(Bid rootkey, int& pos, int rootpos,Bid delKey)
{
	cout <<"root is:"<< rootkey<<endl;
	Bid root = removenew(rootkey,pos,rootkey,rootpos,delKey);
	return root;
}

Bid AVLTreef::removenew(Bid key, int& pos,Bid rootKey, int rootPos, Bid delKey) {

    if (key == 0) {
	    cout << "nothing to delete"<<endl;
        return key;
    }
    Nodef* node = oram->ReadNodef(key, pos, pos);
    if (key < delKey) 
    {
        cout<<"( key < delKey):"<<key << delKey<< endl;
        node->rightID=removenew(node->rightID, node->rightPos, rootKey,rootPos, delKey);
    } 
    else if (key > delKey) 
    {
        cout<<"(key > delKey):"<<key << delKey<< endl;
        node->leftID = removenew(node->leftID, node->leftPos, rootKey, rootPos, delKey);
    } 
    else if(key == delKey)
    { 
	cout<<"(key == delKey):"<<key << delKey << endl;
        Nodef* rc=oram->ReadNodef(node->rightID,node->rightPos,node->rightPos);
	Nodef* minnode;

	if(rc == NULL || rc->key == 0)
	{
		rc = newNodef(0,"");
		cout << "the min value node is->"<< node->key<<endl;
		minnode = oram->ReadNodef(key, pos, pos);
	}
	else
	{
		Nodef* temp = minValueNode(rc->key,rc->pos,rc);
		cout << "the min value node is:"<< temp->key<<endl;
		minnode = oram->ReadNodef(temp->key,temp->pos,temp->pos);
	}
	Nodef* parmin = parentOf(rootKey,rootPos,rootKey,rootPos,minnode->key);

	if (delKey == parmin->leftID || delKey== parmin->rightID)
	{
		Nodef* mlc = oram->ReadNodef(minnode->leftID,minnode->leftPos,minnode->leftPos);
		if(mlc == NULL || mlc->key == 0)
		     mlc = newNodef(0,"");
		return mlc->key ;
	}
	else
	{
		if(parmin != NULL)
		cout <<"parent of minnode is :"<< parmin->key<<endl;
		else 
		cout <<"parent is NULL"<<endl;

		int npos = node->pos;
		Bid bnode = balanceParmin(node->key,npos,parmin,minnode);
		cout << "node was:"<<node->key<< delKey<<endl;
		node = oram->ReadNodef(bnode,npos,npos);
		cout << "node is:"<<node->key<< delKey<<endl;
	       // if this returns a different node then ? 
	 Nodef* lc=oram->ReadNodef(node->leftID,node->leftPos,node->leftPos);
	       	rc=oram->ReadNodef(node->rightID,node->rightPos,node->rightPos);
		if(lc == NULL || lc->key == 0)
			lc = newNodef(0,"");
		if(rc == NULL || rc->key == 0)
			rc = newNodef(0,"");
		
		minnode->leftID = lc->key;
		minnode->leftPos = lc->pos;
		minnode->rightID = rc->key;
		minnode->rightPos = rc->pos;
		minnode->height = max(height(minnode->leftID, minnode->leftPos), height(minnode->rightID, minnode->rightPos)) + 1;
		oram->WriteNodef(minnode->key, minnode);
		return minnode->key;
	}
    }

    node->height = max(height(node->leftID, node->leftPos), height(node->rightID, node->rightPos)) + 1;

    int balance = getBalance(node);

    // Left Left Case
    if (balance > 1 && key < oram->ReadNodef(node->leftID)->key) {
        Nodef* res = rightRotate(node);
        pos = res->pos;
        return res->key;
    }

    // Right Right Case
    if (balance < -1 && key > oram->ReadNodef(node->rightID)->key) {
        Nodef* res = leftRotate(node);
        pos = res->pos;
        return res->key;
    }

    // Left Right Case
    if (balance > 1 && key > oram->ReadNodef(node->leftID)->key) {
        Nodef* res = leftRotate(oram->ReadNodef(node->leftID));
        node->leftID = res->key;
        node->leftPos = res->pos;
        oram->WriteNodef(node->key, node);
        Nodef* res2 = rightRotate(node);
        pos = res2->pos;
        return res2->key;
    }

    // Right Left Case
    if (balance < -1 && key < oram->ReadNodef(node->rightID)->key) {
        auto res = rightRotate(oram->ReadNodef(node->rightID));
        node->rightID = res->key;
        node->rightPos = res->pos;
        oram->WriteNodef(node->key, node);
        auto res2 = leftRotate(node);
        pos = res2->pos;
        return res2->key;
    }

    /* return the (unchanged) node pointer */
    oram->WriteNodef(node->key, node);
    return node->key;
}

Bid AVLTreef:: removeMain(Bid rootKey,int& pos, Bid delKey)
{
	if(rootKey != delKey)
	{
		Nodef* paren = parentOf(rootKey,pos,rootKey,pos,delKey);
		int delPos;
		if(delKey == paren->leftID)
			delPos = paren->leftPos;
		else if(delKey == paren->rightID)
			delPos = paren->rightPos;

		rootKey = removeDel(rootKey,pos,delKey,delPos,paren);
		return rootKey;
	}
	else if(rootKey == delKey)
		return rootKey;
}


Bid AVLTreef::removeDel(Bid rootKey,int& pos,Bid delKey,int delPos,Nodef* paren)
{
	Nodef* node = oram->ReadNodef(rootKey, pos, pos);
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
		node->key = realDelete(node,delKey,delPos);
		//paren->key
	}
//BALANCE:
    node->height = max(height(node->leftID, node->leftPos), height(node->rightID, node->rightPos)) + 1;
    int balance = getBalance(node);
    Bid key = node->key;
    // Left Left Case
    if (balance > 1 && key < oram->ReadNodef(node->leftID)->key) {
        Nodef* res = rightRotate(node);
        pos = res->pos;
        return res->key;
    }

    // Right Right Case
    if (balance < -1 && key > oram->ReadNodef(node->rightID)->key) {
        Nodef* res = leftRotate(node);
        pos = res->pos;
        return res->key;
    }

    // Left Right Case
    if (balance > 1 && key > oram->ReadNodef(node->leftID)->key) {
        Nodef* res = leftRotate(oram->ReadNodef(node->leftID));
        node->leftID = res->key;
        node->leftPos = res->pos;
        oram->WriteNodef(node->key, node);
        Nodef* res2 = rightRotate(node);
        pos = res2->pos;
        return res2->key;
    }

    // Right Left Case
    if (balance < -1 && key < oram->ReadNodef(node->rightID)->key) {
        auto res = rightRotate(oram->ReadNodef(node->rightID));
        node->rightID = res->key;
        node->rightPos = res->pos;
        oram->WriteNodef(node->key, node);
        auto res2 = leftRotate(node);
        pos = res2->pos;
        return res2->key;
    }

    oram->WriteNodef(node->key, node);
    return node->key;
}

Bid AVLTreef::realDelete(Nodef* paren,Bid delKey,int delPos)
{
	Nodef* delnode =oram->ReadNodef(delKey,delPos,delPos); 
Nodef* b=oram->ReadNodef(delnode->rightID,delnode->rightPos,delnode->rightPos);
	Nodef* minnode;
	if(b == NULL || b->key == 0)
	{
		cout << "the min value node is->"<< delnode->key<<endl;
	minnode = oram->ReadNodef(delnode->key,delnode->pos,delnode->pos);
	}
	else
	{
		Nodef* mn = minValueNode(b->key,b->pos,b);
		cout << "the min value node is:"<< mn->key<<endl;
		minnode = oram->ReadNodef(mn->key,mn->pos,mn->pos);
	}
Nodef* parmin=parentOf(paren->key,paren->pos,paren->key,paren->pos,minnode->key);
	if(delKey == minnode->key)//paren->key == parmin->key
	{//no right child of delKey
Nodef* lc = oram->ReadNodef(delnode->leftID,delnode->leftPos,delnode->leftPos);
		if(lc == NULL || lc->key ==0)
			lc = newNodef(0,"");

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
		//oram->WriteNodef(paren->key,paren);
		return paren->key;
	}
	else if(delKey == parmin->key)
	{//no leftchild of minnode
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
		//oram->WriteNodef(paren->key,paren);
		return paren->key;
	}
	else //minnode does not have leftID in general
	{//in this case minnode is parmin's left child always
Nodef* rc = oram->ReadNodef(minnode->rightID,minnode->rightPos,minnode->rightPos);
		if(rc == NULL || rc->key ==0)
			rc = newNodef(0,"");
	
		parmin->leftID = rc->key;
		parmin->leftPos = rc->pos;
		parmin->height = max(height(parmin->leftID,parmin->leftPos), height(parmin->rightID, parmin->rightPos)) + 1;
		oram->WriteNodef(parmin->key,parmin);

		minnode->leftID = delnode->leftID;
		minnode->leftPos = delnode->leftPos;
		minnode->rightID = delnode->rightID;
		minnode->rightPos = delnode->rightPos;
		minnode->height = max(height(minnode->leftID,minnode->leftPos), height(minnode->rightID, minnode->rightPos)) + 1;
		oram->WriteNodef(minnode->key,minnode);
		int minPos = minnode->pos;
		Bid minKey = balanceDel(minnode->key,minnode->pos, parmin);
		minnode = oram->ReadNodef(minKey,minPos,minPos);
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
		//oram->WriteNodef(paren->key,paren);
		return paren->key;
	}	
}

Bid AVLTreef::balanceDel(Bid key, int& pos, Nodef* parmin)
{
	Nodef* node = oram->ReadNodef(key,pos,pos);
	if(node->key < parmin->key)
	{
		node->rightID = balanceDel(node->rightID,node->rightPos,parmin);
	}
	else if(node->key > parmin->key)
	{
		node->leftID = balanceDel(node->leftID,node->leftPos,parmin);
	}
	else if(node->key == parmin->key)
	{

    node->height = max(height(node->leftID, node->leftPos), height(node->rightID, node->rightPos)) + 1;
    int balance = getBalance(node);

    // Left Left Case
    if (balance > 1 && key < oram->ReadNodef(node->leftID)->key) {
        Nodef* res = rightRotate(node);
        pos = res->pos;
        return res->key;
    }

    // Right Right Case
    if (balance < -1 && key > oram->ReadNodef(node->rightID)->key) {
        Nodef* res = leftRotate(node);
        pos = res->pos;
        return res->key;
    }

    // Left Right Case
    if (balance > 1 && key > oram->ReadNodef(node->leftID)->key) {
        Nodef* res = leftRotate(oram->ReadNodef(node->leftID));
        node->leftID = res->key;
        node->leftPos = res->pos;
        oram->WriteNodef(node->key, node);
        Nodef* res2 = rightRotate(node);
        pos = res2->pos;
        return res2->key;
    }

    // Right Left Case
    if (balance < -1 && key < oram->ReadNodef(node->rightID)->key) {
        auto res = rightRotate(oram->ReadNodef(node->rightID));
        node->rightID = res->key;
        node->rightPos = res->pos;
        oram->WriteNodef(node->key, node);
        auto res2 = leftRotate(node);
        pos = res2->pos;
        return res2->key;
    }

    oram->WriteNodef(node->key, node);
    return node->key;

	}
}
