#include "AVLTreef.h"

AVLTreef::AVLTreef(int maxSize, bytes<Key> key) : rd(), mt(rd()), dis(0, (pow(2, floor(log2(maxSize / Z)) + 1) - 1) / 2) {
    oram = new ORAMf(maxSize, key);
    totalleaves = (pow(2, floor(log2(maxSize/Z))+1)-1)/2;
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


int AVLTreef::max(int a, int b) {
    return (a > b) ? a : b;
}

Nodef* AVLTreef::newNodef(Bid key, string val) {
    Nodef* node = new Nodef();
    node->key = key;
    //auto val = to_string(value);
    std::fill(node->value.begin(), node->value.end(), 0);
    std::copy(val.begin(), val.end(), node->value.begin());
    node->leftID = 0;
    node->rightID = 0;
    node->pos = RandomPath();
    node->height = 1; // new node is initially added at leaf
    return node;
}

Nodef* AVLTreef::rightRotate(Nodef* y) {
    Nodef* x = oram->ReadNodef(y->leftID,y->leftPos,y->leftPos);
    Nodef* T2;
    if (x->rightID == 0) {
        T2 = newNodef(0, 0);
    } else {
        T2 = oram->ReadNodef(x->rightID,x->rightPos,x->rightPos);
    }

    x->rightID = y->key;
    x->rightPos = y->pos;
    y->leftID = T2->key;
    y->leftPos = T2->pos;

    y->height = max(height(y->leftID, y->leftPos), height(y->rightID, y->rightPos)) + 1;
    oram->maxheight = max(y->height,oram->maxheight);
    oram->WriteNodef(y->key, y);
    x->height = max(height(x->leftID, x->leftPos), height(x->rightID, x->rightPos)) + 1;
    oram->maxheight = max(x->height,oram->maxheight);
    oram->WriteNodef(x->key, x);

    return x;
}


Nodef* AVLTreef::leftRotate(Nodef* x) {
    Nodef* y = oram->ReadNodef(x->rightID,x->rightPos,x->rightPos);
    Nodef* T2;
    if (y->leftID == 0) {
        T2 = newNodef(0, 0);
    } else {
        T2 = oram->ReadNodef(y->leftID,y->leftPos,y->leftPos);
    }

    y->leftID = x->key;
    y->leftPos = x->pos;
    x->rightID = T2->key;
    x->rightPos = T2->pos;

    x->height = max(height(x->leftID, x->leftPos), height(x->rightID, x->rightPos)) + 1;
    oram->maxheight = max(x->height,oram->maxheight);
    oram->WriteNodef(x->key, x);
    y->height = max(height(y->leftID, y->leftPos), height(y->rightID, y->rightPos)) + 1;
    oram->maxheight = max(y->height,oram->maxheight);
    oram->WriteNodef(y->key, y);
    return y;
}

int AVLTreef::getBalance(Nodef* N) {
    if (N == NULL)
        return 0;
    return height(N->leftID, N->leftPos) - height(N->rightID, N->rightPos);
}




Bid AVLTreef::insert(Bid rootKey, int& pos, Bid key, string value) {
    if (rootKey == 0) {
        Nodef* nnode = newNodef(key, value);
        pos = oram->WriteNodef(key, nnode);
        return nnode->key;
    }
    Nodef* node = oram->ReadNodef(rootKey, pos, pos);
    if (key < node->key) {
        node->leftID = insert(node->leftID, node->leftPos, key, value);
    } else if (key > node->key) {
        node->rightID = insert(node->rightID, node->rightPos, key, value);
    } 
    else 
    {
        std::fill(node->value.begin(), node->value.end(), 0);
        std::copy(value.begin(), value.end(), node->value.begin());
        oram->WriteNodef(rootKey, node);
        return node->key;
    }

    /* 2. Update height of this ancestor node */
    node->height = max(height(node->leftID, node->leftPos), height(node->rightID, node->rightPos)) + 1;
    oram->maxheight = max(node->height,oram->maxheight);

    /* 3. Get the balance factor of this ancestor node to check whether
       this node became unbalanced */
    int balance = getBalance(node);

    // If this node becomes unbalanced, then there are 4 cases

    // Left Left Case
    if (balance > 1 && key < oram->ReadNodef(node->leftID,node->leftPos,node->leftPos)->key) {
        Nodef* res = rightRotate(node);
        pos = res->pos;
        return res->key;
    }

    // Right Right Case
    if (balance < -1 && key > oram->ReadNodef(node->rightID,node->rightPos,node->rightPos)->key) {
        Nodef* res = leftRotate(node);
        pos = res->pos;
        return res->key;
    }

    // Left Right Case
    if (balance > 1 && key > oram->ReadNodef(node->leftID,node->leftPos,node->leftPos)->key) {
        Nodef* res = leftRotate(oram->ReadNodef(node->leftID,node->leftPos,node->leftPos));
        node->leftID = res->key;
        node->leftPos = res->pos;
        oram->WriteNodef(node->key, node);
        Nodef* res2 = rightRotate(node);
        pos = res2->pos;
        return res2->key;
    }

    // Right Left Case
    if (balance < -1 && key < oram->ReadNodef(node->rightID,node->rightPos,node->rightPos)->key) {
        auto res = rightRotate(oram->ReadNodef(node->rightID,node->rightPos,node->rightPos));
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

Nodef* AVLTreef::search(Nodef* head, Bid key) {
    if (head == NULL || head->key == 0)
        return head;
    head = oram->ReadNodef(head->key, head->pos, head->pos);
    if (head->key > key) {
        return search(oram->ReadNodef(head->leftID, head->leftPos, head->leftPos), key);
    } else if (head->key < key) {
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
        cout << root->key << ":" << value.c_str() << ":" << root->pos << ":" << root->leftID << ":" << root->leftPos << ":" << root->rightID << ":" << root->rightPos << endl;
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
	Nodef* curNodef = oram->ReadNodef(rootKey,rootPos,rootPos);
	if(curNodef == NULL || curNodef->key ==0)
	{
		return rootroot;
	}
	else
	{
		return minValueNode(curNodef->leftID,curNodef->leftPos, curNodef);
	}
}



Nodef* AVLTreef::parentOf(Bid parentKey, int ppos, Bid childKey, int cpos, Bid key)
{
	Nodef* parNodef = oram->ReadNodef(parentKey,ppos,ppos);
	//cout <<"parent key of child key is"<<parNodef->key<<childKey<<endl;
	if(key == parNodef->key)
		return NULL;
	else
	{
		Nodef* cNodef = oram->ReadNodef(childKey,cpos,cpos);
		if(key == cNodef->key)
		{
			return parNodef;
		}
		else if(key<cNodef->key)
		{
		return parentOf(cNodef->key,cNodef->pos,cNodef->leftID,cNodef->leftPos,key);
		}
		else if(key>cNodef->key)
		{
		return parentOf(cNodef->key,cNodef->pos,cNodef->rightID,cNodef->rightPos,key);
		}
	}
}




Bid AVLTreef::balance(Nodef* node, int &pos)
{
    Bid key = node->key;
    int balance = getBalance(node);
    //cout <<"balance is:"<<balance<<endl;
    if (balance > 1 )
    {
	    Nodef* leftChild = oram->ReadNodef(node->leftID,node->leftPos,node->leftPos);
    	    if(getBalance(leftChild)>=0)
	    {
	    	//cout <<"Left Left Case" <<endl;
        	Nodef* res = rightRotate(node);
        	pos = res->pos;
        	return res->key;
	    }
	    if(getBalance(leftChild)<0)
	    {
	    	 //cout <<"Left Right Case" <<endl;
		 Nodef* res = leftRotate(leftChild);
        	 node->leftID = res->key;
        	 node->leftPos = res->pos;
        	 oram->WriteNodef(node->key, node);
		 Nodef* res2 = rightRotate(node);
		 pos = res2->pos;
		 return res2->key;
	    }
    }
    if(balance < -1)
    {
	    Nodef* rightChild=oram->ReadNodef(node->rightID,node->rightPos,node->rightPos);
	    if(getBalance(rightChild)<=0)
	    {
	    	//cout <<"Right Right Case" <<endl;
		Nodef* res = leftRotate(node);
		pos = res->pos;
		return res->key;
	    }
	    if(getBalance(rightChild)>0)
	    {
	    	    //cout <<"Right Left Case" <<endl;
		    Nodef* res = rightRotate(rightChild);
		    node->rightID = res->key;
		    node->rightPos = res->pos;
		    oram->WriteNodef(node->key,node);
		    Nodef* res2 = leftRotate(node);
		    pos = res2->pos;
		    return res2->key;
	    }
    }
    //oram->WriteNodef(node->key, node);
    return node->key;
}


int AVLTreef::deleteNode(Nodef* nodef)
{
	Nodef* free = newNodef(0,0);
	oram->DeleteNode(nodef->key,free);
	//oram->WriteNodef(nodef->key,free);
	return 0;
}


Bid AVLTreef:: removeMain(Bid rootKey,int& pos, Bid delKey)
{
	if(rootKey != delKey || rootKey <delKey || rootKey > delKey)
	{// it does not work without these 3 checks ??
		//cout<<"rootKey != delKey(removeMain)"<<endl;
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



Bid AVLTreef::removeRoot(Bid rootKey, int& pos)
{
	Nodef* delnode =oram->ReadNodef(rootKey,pos,pos); 
	Bid delKey = rootKey;
Nodef* b=oram->ReadNodef(delnode->rightID,delnode->rightPos,delnode->rightPos);
	Nodef* minnode;
	if(b == NULL || b->key == 0)
	{
		minnode = delnode;
	}
	else
	{
		Nodef* mn = minValueNode(b->key,b->pos,b);
		minnode = oram->ReadNodef(mn->key,mn->pos,mn->pos);
	}
	if(delKey == minnode->key)
	{//no right child of delKey
Nodef* lc = oram->ReadNodef(delnode->leftID,delnode->leftPos,delnode->leftPos);
		if(lc == NULL || lc->key ==0)
			lc = newNodef(0,0);
		pos = lc->pos;
		deleteNode(delnode);
		return lc->key;
	}
	else
	{

		Nodef* pm=parentOf(rootKey,pos,rootKey,pos,minnode->key);
		Nodef* parmin = oram->ReadNodef(pm->key,pm->pos,pm->pos);
		//cout << "parent of minnode is:"<< parmin->key<<endl;
		if(delKey == parmin->key)
		{//no leftchild of minnode //minnode is delKey's right child
		//	cout <<"SECOND CASE: rootKey == parmin->key"<< endl;
	Nodef* lc = oram->ReadNodef(delnode->leftID,delnode->leftPos,delnode->leftPos);
			if(lc == NULL || lc->key ==0)
				lc = newNodef(0,0);
			minnode->leftID = lc->key;
			minnode->leftPos = lc->pos;
			minnode->height =  max(height(minnode->leftID,minnode->leftPos), height(minnode->rightID, minnode->rightPos)) + 1;
			oram->WriteNodef(minnode->key,minnode);
			int minPos = minnode->pos;
			Bid minKey = balance(minnode,minnode->pos);
			minnode = oram->ReadNodef(minKey,minPos,minPos);
			pos = minnode->pos;
			deleteNode(delnode);
			return minnode->key;
		}
		else //minnode does not have leftID in general
		{//in this case minnode is parmin's left child always
	Nodef* rc = oram->ReadNodef(minnode->rightID,minnode->rightPos,minnode->rightPos);
			if(rc == NULL || rc->key ==0)
				rc = newNodef(0,0);
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
			pos = minnode->pos;
			deleteNode(delnode);
			return minnode->key;
		}	
	}
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
		node->key = realDelete(node,delKey,delPos);// int& node->pos
		//paren->key
	}
//BALANCE:
    node->height = max(height(node->leftID, node->leftPos), height(node->rightID, node->rightPos)) + 1;
    int balance = getBalance(node);
    //cout << "Balance is:"<<balance<<endl;
    Bid key = node->key;
    if (balance > 1 )
    {
	    Nodef* leftChild = oram->ReadNodef(node->leftID,node->leftPos,node->leftPos);
    	    if(getBalance(leftChild)>=0)
	    {
	    	//cout <<"Left Left Case" <<endl;
        	Nodef* res = rightRotate(node);
        	pos = res->pos;
        	return res->key;
	    }
	    if(getBalance(leftChild)<0)
	    {
	    	 //cout <<"Left Right Case" <<endl;
		 Nodef* res = leftRotate(leftChild);
        	 node->leftID = res->key;
        	 node->leftPos = res->pos;
        	 oram->WriteNodef(node->key, node);
		 Nodef* res2 = rightRotate(node);
		 pos = res2->pos;
		 return res2->key;
	    }
    }
    if(balance < -1)
    {
	    Nodef* rightChild=oram->ReadNodef(node->rightID,node->rightPos,node->rightPos);
	    if(getBalance(rightChild)<=0)
	    {
	        //cout <<"Right Right Case" <<endl;
		Nodef* res = leftRotate(node);
		pos = res->pos;
		return res->key;
	    }
	    if(getBalance(rightChild)>0)
	    {
	            //cout <<"Right Left Case" <<endl;
		    Nodef* res = rightRotate(rightChild);
		    node->rightID = res->key;
		    node->rightPos = res->pos;
		    oram->WriteNodef(node->key,node);
		    Nodef* res2 = leftRotate(node);
		    pos = res2->pos;
		    return res2->key;
	    }
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
		//cout << "the min value node is->"<< delnode->key<<endl;
	minnode = oram->ReadNodef(delnode->key,delnode->pos,delnode->pos);
	}
	else
	{
		Nodef* mn = minValueNode(b->key,b->pos,b);
		//cout << "the min value node is:"<< mn->key<<endl;
		minnode = oram->ReadNodef(mn->key,mn->pos,mn->pos);
	}
Nodef* pm=parentOf(paren->key,paren->pos,paren->key,paren->pos,minnode->key);
	Nodef* parmin = oram->ReadNodef(pm->key,pm->pos,pm->pos);
	//cout << "parent of minnode is:"<< parmin->key<<endl;
	if(delKey == minnode->key)//paren->key == parmin->key
	{//no right child of delKey
	//	cout <<"FIRST CASE:delKey == minnode->key"<< endl;
Nodef* lc = oram->ReadNodef(delnode->leftID,delnode->leftPos,delnode->leftPos);
		if(lc == NULL || lc->key ==0)
			lc = newNodef(0,0);
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
		oram->WriteNodef(paren->key,paren);
		deleteNode(delnode);
		return paren->key;
	}
	else if(delKey == parmin->key)
	{//no leftchild of minnode //minnode is delKey right child
	//	cout <<"SECOND CASE: delKey == parmin->key"<< endl;
Nodef* lc = oram->ReadNodef(delnode->leftID,delnode->leftPos,delnode->leftPos);
		if(lc == NULL || lc->key ==0)
			lc = newNodef(0,0);
		minnode->leftID = lc->key;
		minnode->leftPos = lc->pos;
		minnode->height =  max(height(minnode->leftID,minnode->leftPos), height(minnode->rightID, minnode->rightPos)) + 1;
		oram->WriteNodef(minnode->key,minnode);
		int minPos = minnode->pos;
		Bid minKey = balance(minnode,minnode->pos);
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
		oram->WriteNodef(paren->key,paren);
		deleteNode(delnode);
		return paren->key;
	}
	else //minnode does not have leftID in general
	{//in this case minnode is parmin's left child always
		//cout <<"THIRD case"<< endl;	
Nodef* rc = oram->ReadNodef(minnode->rightID,minnode->rightPos,minnode->rightPos);
		if(rc == NULL || rc->key ==0)
			rc = newNodef(0,0);
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
		oram->WriteNodef(paren->key,paren);
		deleteNode(delnode);
		return paren->key;
	}	
}


Bid AVLTreef::balanceDel(Bid key, int& pos, Nodef* parmin)
{
	Nodef* node = oram->ReadNodef(key,pos,pos);
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
    int balance = getBalance(node);
    //cout <<"balance is:"<<balance<<endl;
    if (balance > 1 )
    {
	    Nodef* leftChild = oram->ReadNodef(node->leftID,node->leftPos,node->leftPos);
    	    if(getBalance(leftChild)>=0)
	    {
	    	//cout <<"Left Left Case" <<endl;
        	Nodef* res = rightRotate(node);
        	pos = res->pos;
        	return res->key;
	    }
	    if(getBalance(leftChild)<0)
	    {
	    	 //cout <<"Left Right Case" <<endl;
		 Nodef* res = leftRotate(leftChild);
        	 node->leftID = res->key;
        	 node->leftPos = res->pos;
        	 oram->WriteNodef(node->key, node);
		 Nodef* res2 = rightRotate(node);
		 pos = res2->pos;
		 return res2->key;
	    }
    }
    if(balance < -1)
    {
	    Nodef* rightChild=oram->ReadNodef(node->rightID,node->rightPos,node->rightPos);
	    if(getBalance(rightChild)<=0)
	    {
	    	//cout <<"Right Right Case" <<endl;
		Nodef* res = leftRotate(node);
		pos = res->pos;
		return res->key;
	    }
	    if(getBalance(rightChild)>0)
	    {
	    	    //cout <<"Right Left Case" <<endl;
		    Nodef* res = rightRotate(rightChild);
		    node->rightID = res->key;
		    node->rightPos = res->pos;
		    oram->WriteNodef(node->key,node);
		    Nodef* res2 = leftRotate(node);
		    pos = res2->pos;
		    return res2->key;
	    }
    }

  //}
    oram->WriteNodef(node->key, node);
    return node->key;
}
void AVLTreef::setupInsert(Bid& rootKey, int& rootPos, map<Bid, string> pairs) {
    for (auto pair : pairs) {
        Nodef* node = newNodef(pair.first, pair.second);
        setupNodes.push_back(node);
    }
    //cout << "Creating BST" << endl;
    sortedArrayToBST(0, setupNodes.size() - 1, rootPos, rootKey);
    //cout << "Inserting in ORAM" << endl;
    oram->setupInsert(setupNodes);
}

int AVLTreef::sortedArrayToBST(int start, int end, int& pos, Bid& node) {
    setupProgress++;
    if (setupProgress % 100000 == 0) {
        cout << setupProgress << "/" << setupNodes.size()*2 << " of AVL tree constructed" << endl;
    }
    if (start > end) {
        pos = -1;
        node = 0;
        return 0;
    }

    int mid = (start + end) / 2;
    Nodef *root = setupNodes[mid];

    int leftHeight = sortedArrayToBST(start, mid - 1, root->leftPos, root->leftID);

    int rightHeight = sortedArrayToBST(mid + 1, end, root->rightPos, root->rightID);
    root->pos = RandomPath();
    root->height = max(leftHeight, rightHeight) + 1;
    pos = root->pos;
    node = root->key;
    return root->height;
}
