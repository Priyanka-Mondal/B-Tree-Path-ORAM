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
    Nodef* x = oram->ReadNodef(y->leftID);
    Nodef* T2;
    if (x->rightID == 0) {
        T2 = newNodef(0, "");
    } else {
        T2 = oram->ReadNodef(x->rightID);
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
    Nodef* y = oram->ReadNodef(x->rightID);
    Nodef* T2;
    if (y->leftID == 0) {
        T2 = newNodef(0, "");
    } else {
        T2 = oram->ReadNodef(y->leftID);
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

string AVLTreef::minValue(Bid rootKey, int rootPos, Nodef* rootroot, string v)
{
	Nodef* curNode = oram->ReadNodef(rootKey,rootPos,rootPos);
	if(curNode == NULL || curNode->key ==0)
	{
	       	v.assign(rootroot->value.begin(), rootroot->value.end());
		v = v.c_str();
		return v;
	}
	else
	{
		return minValue(curNode->leftID,curNode->leftPos, curNode,v);
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
		if(key<cNode->key)
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
    Bid key = node->key;
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
}


Bid AVLTreef::remove(Bid rootKey, int& pos, Bid delKey)
{
	cout <<"root At the beginning " << rootKey<< endl;
	Nodef* nodef = search(oram->ReadNodef(rootKey,pos,pos),delKey);
	cout << " delKey:" << nodef->key <<endl;
	Nodef* b = oram->ReadNodef(nodef->rightID,nodef->rightPos,nodef->rightPos);
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
	Nodef* parmin = parentOf(rootKey,pos,rootKey,pos,minnode->key);
	//1. parent = NULL & parmin = NULL -> delnode's right subtree is NULL
	//2. parent = NULL & parmin != NULL  ****DONE
	//4. parent != NULL & parmin != NULL
	//	4a. parmin == delkey ?/*
	if(parent == NULL && parmin == NULL)
	{//node->leftID becomes root
		cout << "parent NULL parmin NULL"<< endl;
		node=oram->ReadNodef(node->leftID,node->leftPos,node->leftPos);
	}
	else if(parent == NULL && parmin != NULL)
	{
		if(parmin->key != delKey)
		{
			Nodef* tr = oram->ReadNodef(minnode->rightID,minnode->rightPos,minnode->rightPos);
			Nodef* pm = oram->ReadNodef(parmin->key,parmin->pos,parmin->pos);
			if(tr ==NULL || tr->key ==0)
			{
				tr = newNodef(0,"");
			}
				pm->leftID = tr->key;//tr->key<parmin->key ever
				pm->leftPos = tr->pos;
				oram->WriteNodef(pm->key,pm);
			
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
			pos = node->pos;
			Nodef* free = newNodef(0,"");
			oram->WriteNodef(nodef->key,free);
			goto BALANCE; // balance the tree
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
			pos = node->pos;
                        Nodef* free = newNodef(0,"");
                        oram->WriteNodef(nodef->key,free);
                        goto BALANCE; 
		}
	}
	else if(parent!=NULL && parmin!=NULL)
	{
		if(parmin->key == delKey)
		{
			cout << "PARMIN KEY ++ DELKEY"<< endl;
		Nodef* par=oram->ReadNodef(parent->key,parent->pos,parent->pos);			if(par->key < delKey) // delkey is at right subtree
			{
				Nodef* delLeft = oram->ReadNodef(nodef->leftID, nodef->leftPos, nodef->leftPos);
				if(delLeft == NULL || delLeft->key == 0)
				{
					delLeft = newNodef(0,"");
				}
				node->leftID = delLeft->key;
				node->leftPos = delLeft->pos;
				node->height = max(height(node->leftID, node->leftPos), height(node->rightID, node->rightPos)) + 1;
				oram->WriteNodef(node->key,node);
				//Bid bk=balance(node, node->pos);
				//node = oram->ReadNodef(bk);
				par->rightID = node->key;
				par->rightPos = node->pos;
				par->height = max(height(par->leftID, par->leftPos), height(par->rightID, par->rightPos)) + 1; 
				//Bid pk = balance(par,par->pos);
				//par = oram->ReadNodef(pk);
				oram->WriteNodef(par->key, par);

				Nodef* free = newNodef(0,"");
				oram->WriteNodef(nodef->key,free);
				if(par->key!=rootKey)
				{
					return rootKey;
				}
				else
				{//WHAT ?
					pos = par->pos;
					node = oram->ReadNodef(par->key,par->pos,par->pos);
					goto BALANCE;
				}
			}
			else if(par->key >delKey)
			{
		node = oram->ReadNodef(rootKey,pos,pos);
		goto BALANCE;
			}
		}
		else if(delKey == node->key) //delkey has no right child
		{
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
			oram->WriteNodef(pm->key,pm);
			Nodef* free = newNodef(0,"");
			oram->WriteNodef(nodef->key,free);
			if(pm->key!=rootKey)
				return rootKey;
			else
				node =oram->ReadNodef(pm->key,pm->pos,pm->pos);
		}
		else // if(what condition?)
		{
			Nodef* lc = oram->ReadNodef(nodef->leftID,nodef->leftPos,nodef->leftPos);
			if(lc==NULL || lc->key == 0)
				lc= newNodef(0,"");
			Nodef* rc = oram->ReadNodef(nodef->rightID,nodef->rightPos,nodef->rightPos);
			if(rc==NULL || rc->key == 0)
				rc= newNodef(0,"");
			Nodef* mrc = oram->ReadNodef(node->rightID,node->rightPos,node->rightPos);
			if(mrc==NULL || mrc->key == 0)
				mrc= newNodef(0,"");
			Nodef* par = oram->ReadNodef(parent->key,parent->pos,parent->pos);
			Nodef* pm = oram->ReadNodef(parmin->key,parmin->pos,parmin->pos);
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
			oram->WriteNodef(pm->key,pm);
			node->leftID = lc->key;
			node->leftPos = lc->pos;
			node->rightID = rc->key;
			node->rightPos = rc->pos;
			node->height = max(height(node->leftID, node->leftPos), height(node->rightID, node->rightPos)) + 1;
			oram->WriteNodef(node->key, node);
			if(nodef->key < par->key)
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
			oram->WriteNodef(par->key,par);
			Bid n = balance(node,node->pos);
			if(node->key!=rootKey)
				return rootKey;
			else pos=node->pos;
		}
		/*else
		{
		node = oram->ReadNodef(rootKey,pos,pos);
		}*/
		//if()
	/*	else if(parmin->key != delKey)
		{
			cout << "parmin key =/= delkey"<< endl;
		node = oram->ReadNodef(rootKey,pos,pos);
		}*/
	}
	else
	{
		cout <<"PARENT of"<<nodef->key<<"is"<< parent->key<< endl;
		node = oram->ReadNodef(rootKey,pos,pos);
	}
BALANCE:	
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
    oram->WriteNodef(node->key,node);
    return node->key;	
}

Bid AVLTreef::deleteHelper(Bid rootKey, int& pos, Bid delKey,Bid key,int posk,string value) {
    if (rootKey == delKey) 
    {
	cout<<"(rootKey==delKey):"<< endl;
        //Nodef* nnode = newNodef(key, value);
	Nodef* nnode = oram->ReadNodef(key, posk, posk);
        pos = oram->WriteNodef(rootKey, nnode);
        return rootKey;
    }
    Nodef* node = oram->ReadNodef(rootKey, pos, pos);
    if (delKey < node->key) {
	    cout<<"(del key < cur node):"<<delKey << node->key<< endl;
        node->leftID = deleteHelper(node->leftID, node->leftPos, delKey,key, posk,value);
    } else if (delKey > node->key) {
	    cout<<"(del key > cur node):"<<delKey << node->key<< endl;
        node->rightID = deleteHelper(node->rightID, node->rightPos, delKey,key,posk ,value);
    } else { // this one updates exixting value
	    cout<<"(del key == cur node):"<<delKey << node->key<< endl;

	   Nodef* min = newNodef(key,value);
	   //Nodef* min = oram->ReadNodef(key,posk,posk);
	   cout << "min value:"<< min->key<< endl;
        std::fill(node->value.begin(), node->value.end(), 0);
        std::copy(value.begin(), value.end(), node->value.begin());
        pos = oram->WriteNodef(node->key,min);
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
