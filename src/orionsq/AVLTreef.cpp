#include "AVLTreef.h"

AVLTreef::AVLTreef(int maxSize, bytes<Key> key) : rd(), mt(rd()), dis(0, (pow(2, floor(log2(maxSize / Z)) + 1) - 1) / 2) {
    oram = new ORAMf(maxSize, key);
    totalleaves = (pow(2, floor(log2(maxSize/Z))+1)-1)/2;
    cout <<"total leaves in treef:(0.."<< totalleaves<<")"<<totalleaves+1<<endl;
    cout <<"--------------------------------------------"<<endl;
    this->insertread = oram->insertread;
}

AVLTreef::~AVLTreef() {
    delete oram;
}


int AVLTreef::setupheight(Bid key, int& leaf) {
    if (key == 0)
        return 0;
    Nodef* node = new Nodef();
    oram->setupReadNf(node,key,leaf);
    //Nodef* node = oram->setupReadNf(key, leaf);
    //return node->height;
    int hei = node->height;
    delete node;
    return hei;
}
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
Nodef* AVLTreef::newNodef(Bid key, int value) {
    Nodef* node = new Nodef();
    node->key = key;
    auto val = to_string(value);
    std::fill(node->value.begin(), node->value.end(), 0);
    std::copy(val.begin(), val.end(), node->value.begin());
    node->leftID = 0;
    node->rightID = 0;
    node->pos = RandomPath();
    node->height = 1; // new node is initially added at leaf
    return node;
}

Nodef* AVLTreef::setupnewNodef(Bid key, int value) {
    Nodef* node = new Nodef();
    node->key = key;
    auto val = to_string(value);
    std::fill(node->value.begin(), node->value.end(), 0);
    std::copy(val.begin(), val.end(), node->value.begin());
    node->leftID = 0;
    node->rightID = 0;
    node->pos = notsoRandomPath();
    node->height = 1; // new node is initially added at leaf
    return node;
}

Nodef* AVLTreef::setuprightRotate(Nodef* y, Bid rootKey, int& pos) {
    Nodef* x = oram->setupReadNf(y->leftID,y->leftPos);
    Nodef* T2;
    if (x->rightID == 0) {
		//setupleaf--;
        T2 = setupnewNodef(0, 0);
    } else {
        T2 = oram->setupReadNf(x->rightID,x->rightPos);
    }

    x->rightID = y->key;
    x->rightPos = y->pos;
    y->leftID = T2->key;
    y->leftPos = T2->pos;

    y->height = max(setupheight(y->leftID, y->leftPos), setupheight(y->rightID, y->rightPos)) + 1;
    oram->maxheight = max(y->height,oram->maxheight);
    oram->setupWriteNf(y->key, y, rootKey,pos);
    x->height = max(setupheight(x->leftID, x->leftPos), setupheight(x->rightID, x->rightPos)) + 1;
    oram->maxheight = max(x->height,oram->maxheight);
    oram->setupWriteNf(x->key, x, rootKey,  pos);
delete T2;
    return x;
}
Nodef* AVLTreef::rightRotate(Nodef* y) {
    Nodef* x = oram->ReadNodef(y->leftID,y->leftPos,y->leftPos);
    Nodef* T2;
    if (x->rightID == 0) {
        T2 = newNodef(0, 0);
    } else {
        //T2 = oram->ReadNodef(x->rightID,x->rightPos,x->rightPos);
	T2 = new Nodef();
        oram->setupReadNf(T2,x->rightID,x->rightPos);
    }

    // Perform rotation
    x->rightID = y->key;
    x->rightPos = y->pos;
    y->leftID = T2->key;
    y->leftPos = T2->pos;

    // Update heights
    y->height = max(height(y->leftID, y->leftPos), height(y->rightID, y->rightPos)) + 1;
    oram->maxheight = max(y->height,oram->maxheight);
    oram->WriteNodef(y->key, y);
    x->height = max(height(x->leftID, x->leftPos), height(x->rightID, x->rightPos)) + 1;
    oram->maxheight = max(x->height,oram->maxheight);
    oram->WriteNodef(x->key, x);
    // Return new root

    return x;
}


Nodef* AVLTreef::setupleftRotate(Nodef* x, Bid rootKey, int& pos) {
    Nodef* y = oram->setupReadNf(x->rightID,x->rightPos);
    Nodef* T2;
    if (y->leftID == 0) {
		//setupleaf--;
        T2 = setupnewNodef(0, 0);
    } else {
        //T2 = oram->setupReadNf(y->leftID,y->leftPos);
	T2 = new Nodef();
        oram->setupReadNf(T2,y->leftID,y->leftPos);
    }

    y->leftID = x->key;
    y->leftPos = x->pos;
    x->rightID = T2->key;
    x->rightPos = T2->pos;

    x->height = max(setupheight(x->leftID, x->leftPos), setupheight(x->rightID, x->rightPos)) + 1;
    oram->maxheight = max(x->height,oram->maxheight);
    oram->setupWriteNf(x->key, x, rootKey, pos);
    y->height = max(setupheight(y->leftID, y->leftPos), setupheight(y->rightID, y->rightPos)) + 1;
    oram->maxheight = max(y->height,oram->maxheight);
    oram->setupWriteNf(y->key, y, rootKey, pos);
delete T2;
    return y;
}
Nodef* AVLTreef::leftRotate(Nodef* x) {
    Nodef* y = oram->ReadNodef(x->rightID,x->rightPos,x->rightPos);
    Nodef* T2;
    if (y->leftID == 0) {
        T2 = newNodef(0, 0);
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
    oram->maxheight = max(x->height,oram->maxheight);
    oram->WriteNodef(x->key, x);
    y->height = max(height(y->leftID, y->leftPos), height(y->rightID, y->rightPos)) + 1;
    oram->maxheight = max(y->height,oram->maxheight);
    oram->WriteNodef(y->key, y);
    // Return new root
    return y;
}


int AVLTreef::setupgetBalance(Nodef* N) {
    if (N == NULL)
        return 0;
    return setupheight(N->leftID, N->leftPos) - setupheight(N->rightID, N->rightPos);
}

int AVLTreef::getBalance(Nodef* N) {
    if (N == NULL)
        return 0;
    return height(N->leftID, N->leftPos) - height(N->rightID, N->rightPos);
}

Bid AVLTreef::setupinsert(Bid rootKey, int& pos, Bid key, int value) 
{
    if (rootKey == 0) {
        Nodef* nnode = setupnewNodef(key, value);
        pos = oram->setupWriteNf(key, nnode,key,pos);
        //return nnode->key;
        
	Bid nnk = nnode->key;
	delete nnode;
	return nnk;
    }
    Nodef* node = new Nodef();
    oram->setupReadNf(node, rootKey, pos);
    //Nodef* node = oram->setupReadNf(rootKey, pos);
    //if(node ==NULL ) cout<<rootKey<<"rootKey/"<<pos<<":rootPos /got NULL while inserting"<<key<<endl;
    if (key < node->key) {
	    //cout <<"key<nodef"<<key<<node->key<<node->leftPos<<endl;
        node->leftID = setupinsert(node->leftID, node->leftPos, key, value);
    } else if (key > node->key) {
	    //cout <<"key>nodef"<<key<<node->key<<node->rightPos<<endl;
        node->rightID = setupinsert(node->rightID, node->rightPos, key, value);
//	cout <<node->rightPos<<"the RIGHT ID IS -----"<< node->rightID<<endl;
    } 
    else 
    {
	auto val = to_string(value);
        std::fill(node->value.begin(), node->value.end(), 0);
        std::copy(val.begin(), val.end(), node->value.begin());
        oram->setupWriteNf(rootKey, node,rootKey, pos);
        return node->key;
    }

    node->height = max(setupheight(node->leftID, node->leftPos), setupheight(node->rightID, node->rightPos)) + 1;
    oram->maxheight= max(node->height,oram->maxheight);

    int balance = setupgetBalance(node);
    
    if (balance > 1 && key < node->leftID){//oram->setupReadNf(node->leftID,node->leftPos)->key) {
    //cout <<" Left Left Case-----------------------------------"<<endl;
        Nodef* res = setuprightRotate(node, rootKey, pos);
        pos = res->pos;
       // return res->key;
        
	Bid nnk = res->key;
	delete res;
	return nnk;
    }

    // Right Right Case
    if (balance < -1 && key > node->rightID){//oram->setupReadNf(node->rightID,node->rightPos)->key) {
    //cout <<" Right Right Case-----------------------------------"<<endl;
        Nodef* res = setupleftRotate(node, rootKey, pos);
        pos = res->pos;
       // return res->key;
        
	Bid nnk = res->key;
	delete res;
	return nnk;
    }
    // Left Right Case
    if (balance > 1 && key > node->leftID){//oram->setupReadNf(node->leftID,node->leftPos)->key) {
//    cout <<" Left Right Case-----------------------------------"<<endl;
        Nodef* nodel = new Nodef();
        oram->setupReadNf(nodel,node->leftID,node->leftPos);
        Nodef* res = setupleftRotate(nodel,rootKey,pos);//oram->setupReadNf(node->leftID,node->leftPos),rootKey, pos);
        node->leftID = res->key;
        node->leftPos = res->pos;
        oram->setupWriteNf(node->key, node, rootKey,pos);
        Nodef* res2 = setuprightRotate(node, rootKey, pos);
        pos = res2->pos;
        //return res2->key;
        
	Bid nnk = res2->key;
	delete res2;
	delete res;
	delete nodel;
	return nnk;
    }

    // Right Left Case
    if (balance < -1 && key < node->rightID){//oram->setupReadNf(node->rightID,node->rightPos)->key) {
//    cout <<" Right Left Case-----------------------------------"<<endl;
        Nodef* noder = new Nodef();
        oram->setupReadNf(noder,node->rightID,node->rightPos);
        Nodef* res = setuprightRotate(noder,rootKey,pos);//oram->setupReadNf(node->rightID,node->rightPos), rootKey, pos);
        node->rightID = res->key;
        node->rightPos = res->pos;
        oram->setupWriteNf(node->key, node, rootKey,pos);
        auto res2 = setupleftRotate(node, rootKey, pos);
        pos = res2->pos;
       // return res2->key;
        
	Bid nnk = res2->key;
	delete res2;
	delete res;
	delete noder;
	return nnk;
    }

    /* return the (unchanged) node pointer */
    oram->setupWriteNf(node->key, node, rootKey,pos);
    //return node->key;
        
	Bid nnk = node->key;
	delete node;
	return nnk;
}


Bid AVLTreef::insert(Bid rootKey, int& pos, Bid key, int value) {
    /* 1. Perform the normal BST rotation */
    if (rootKey == 0) {
        Nodef* nnode = newNodef(key, value);
        pos = oram->WriteNodef(key, nnode);
        return nnode->key;
    }
    Nodef* node = oram->ReadNodef(rootKey, pos, pos);
    if (key < node->key) 
    {
        node->leftID = insert(node->leftID, node->leftPos, key, value);
    } 
    else if (key > node->key) 
    {
        node->rightID = insert(node->rightID, node->rightPos, key, value);
    } 
    else 
    {
	auto val = to_string(value);
        std::fill(node->value.begin(), node->value.end(), 0);
        std::copy(val.begin(), val.end(), node->value.begin());
        oram->WriteNodef(rootKey, node);
        return node->key;
    }

    node->height = max(height(node->leftID, node->leftPos), height(node->rightID, node->rightPos)) + 1;
    oram->maxheight= max(node->height,oram->maxheight);

    int balance = getBalance(node);


    // Left Left Case
    if (balance > 1 && key < oram->ReadNodef(node->leftID)->key) {
	//    cout <<"left left"<<endl;
        Nodef* res = rightRotate(node);
        pos = res->pos;
        return res->key;
    }

    // Right Right Case
    if (balance < -1 && key > oram->ReadNodef(node->rightID)->key) {
	  //  cout <<"right right"<<endl;
        Nodef* res = leftRotate(node);
        pos = res->pos;
        return res->key;
    }

    // Left Right Case
    if (balance > 1 && key > oram->ReadNodef(node->leftID)->key) {
	   // cout <<"left right"<<endl;
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
	    //cout <<"right left"<<endl;
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

string AVLTreef::setupsimplesearch(Bid rkey, int rpos, Bid key) {
    if (rkey == 0)
        return "";
    Nodef *head = new Nodef();
    oram->setupReadNf(head,rkey, rpos);
    if (head->key > key) 
    {
        Bid hl = head->leftID;
	int lp = head->leftPos;
	delete head;
        return setupsimplesearch(hl,lp, key);
    } 
    else if (head->key < key) 
    {
        Bid hr = head->rightID;
	int rp = head->rightPos;
	delete head;
        return setupsimplesearch(hr, rp, key);
    } 
    else 
    {
	string val;
        val.assign(head->value.begin(), head->value.end());
	delete head;
        return val;
    }
}
Nodef* AVLTreef::setupsearch(Nodef* head, Bid key) {
    if (head == NULL || head->key == 0)
        return head;
    head = oram->setupReadNf(head->key, head->pos);
    if (head->key > key) 
    {
        return setupsearch(oram->setupReadNf(head->leftID, head->leftPos), key);
    } 
    else if (head->key < key) 
    {
        return setupsearch(oram->setupReadNf(head->rightID, head->rightPos), key);
    } 
    else 
        return head;
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
void AVLTreef::batchSearch(Nodef* head, vector<Bid> keys, vector<Nodef*>* results) 
{
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
	searchi_bytes = oram->searchi_bytes;
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
void AVLTreef::startOperation(bool batchWrite) 
{
    oram->insertread = 0;
    searchi_bytes = 0;
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
int AVLTreef::notsoRandomPath() 
{
    if(setupleaf != totalleaves)
	    setupleaf++;
    else
	    setupleaf = 0;
    return setupleaf;
}



//delete functions
Nodef* AVLTreef::minValueNode(Bid rootKey, int rootPos, Nodef* rootroot)
{
	Nodef* curNodef = oram->ReadNodef(rootKey,rootPos,rootPos);
	if(curNodef == NULL || curNodef->key ==0)
		return rootroot;
	else
		return minValueNode(curNodef->leftID,curNodef->leftPos, curNodef);
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
	oram->DeleteNodef(nodef->key,free);
	return 0;
}


Bid AVLTreef:: removeMain(Bid rootKey,int& pos, Bid delKey)
{
	if(rootKey != delKey || rootKey <delKey || rootKey > delKey)
	{
		// it does not work without these 3 checks ??
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
		//cout << "the min value node is->"<< delnode->key<<endl;
		minnode = delnode;
	}
	else
	{
		Nodef* mn = minValueNode(b->key,b->pos,b);
		//cout << "the min value node is:"<< mn->key<<endl;
		minnode = oram->ReadNodef(mn->key,mn->pos,mn->pos);
	}
	
	if(delKey == minnode->key)
	{//no right child of delKey
		//cout <<"FIRST CASE:rootKey == minnode->key"<< endl;
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
    			//oram->maxheight= max(minnode->height,oram->maxheight);
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
		//	cout <<"THIRD case"<< endl;	
	Nodef* rc = oram->ReadNodef(minnode->rightID,minnode->rightPos,minnode->rightPos);
			if(rc == NULL || rc->key ==0)
				rc = newNodef(0,0);
			parmin->leftID = rc->key;
			parmin->leftPos = rc->pos;
			parmin->height = max(height(parmin->leftID,parmin->leftPos), height(parmin->rightID, parmin->rightPos)) + 1;
    			//oram->maxheight= max(parmin->height,oram->maxheight);
			oram->WriteNodef(parmin->key,parmin);
			minnode->leftID = delnode->leftID;
			minnode->leftPos = delnode->leftPos;
			minnode->rightID = delnode->rightID;
			minnode->rightPos = delnode->rightPos;
			minnode->height = max(height(minnode->leftID,minnode->leftPos), height(minnode->rightID, minnode->rightPos)) + 1;
                        //oram->maxheight= max(minnode->height,oram->maxheight);
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
    //oram->maxheight= max(node->height,oram->maxheight);
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
                //oram->maxheight= max(paren->height,oram->maxheight);
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
    		//oram->maxheight= max(minnode->height,oram->maxheight);
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
    		//oram->maxheight= max(paren->height,oram->maxheight);
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
    		//oram->maxheight= max(parmin->height,oram->maxheight);
		oram->WriteNodef(parmin->key,parmin);
		minnode->leftID = delnode->leftID;
		minnode->leftPos = delnode->leftPos;
		minnode->rightID = delnode->rightID;
		minnode->rightPos = delnode->rightPos;
		minnode->height = max(height(minnode->leftID,minnode->leftPos), height(minnode->rightID, minnode->rightPos)) + 1;
    		//oram->maxheight= max(minnode->height,oram->maxheight);
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
    		//oram->maxheight= max(paren->height,oram->maxheight);
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
    //oram->maxheight= max(node->height,oram->maxheight);
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
void AVLTreef::setupInsert(Bid& rootKey, int& rootPos, map<Bid, int> pairs) {
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
