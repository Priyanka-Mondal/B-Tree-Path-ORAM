#include "AVLTree.h"

AVLTree::AVLTree(int maxSize, bytes<Key> key) : rd(), mt(rd()), dis(0, (pow(2, floor(log2(maxSize / Z)) + 1) - 1) / 2) {
    oram = new ORAM(maxSize, key);
    totalleaves = (pow(2, floor(log2(maxSize/Z))+1)-1)/2;
    cout <<"total leaves in treef:(0.."<< totalleaves<<")"<<totalleaves+1<<endl;
    cout <<"--------------------------------------------"<<endl;

}

AVLTree::~AVLTree() {
    delete oram;
}


int AVLTree::setupheight(Bid key, int& leaf) {
    if (key == 0)
        return 0;
    Node* node = new Node();
    oram->setupReadN(node,key,leaf);
    //Node* node = oram->setupReadN(key, leaf);
    //return node->height;
    int hei = node->height;
    delete node;
    return hei;
}
int AVLTree::height(Bid key, int& leaf) {
    if (key == 0)
        return 0;
    Node* node = oram->ReadNode(key, leaf, leaf);
    return node->height;
}

// A utility function to get maximum of two integers

int AVLTree::max(int a, int b) {
    return (a > b) ? a : b;
}

/* Helper function that allocates a new node with the given key and
   NULL left and right pointers. */
Node* AVLTree::newNode(Bid key, string value) {
    Node* node = new Node();
    node->key = key;
    std::fill(node->value.begin(), node->value.end(), 0);
    std::copy(value.begin(), value.end(), node->value.begin());
    node->leftID = 0;
    node->rightID = 0;
    node->pos = RandomPath();
    node->height = 1; // new node is initially added at leaf
    return node;
}

Node* AVLTree::setupnewNode(Bid key, string value) {
    Node* node = new Node();
    node->key = key;
    std::fill(node->value.begin(), node->value.end(), 0);
    std::copy(value.begin(),value.end(), node->value.begin());
    node->leftID = 0;
    node->rightID = 0;
    node->pos = notsoRandomPath();
    //cout <<"not so random node->pos"<< node->pos<<endl;
    node->height = 1; // new node is initially added at leaf
    return node;
}

Node* AVLTree::setuprightRotate(Node* y,Bid rootKey, int& pos) {
    Node* x = oram->setupReadN(y->leftID,y->leftPos);
    Node* T2;
    if (x->rightID == 0) {
        T2 = setupnewNode(0, "");
    } else {
        //T2 = oram->setupReadN(x->rightID,x->rightPos);
	T2 = new Node();
        oram->setupReadN(T2,x->rightID,x->rightPos);
    }

    // Perform rotation
    x->rightID = y->key;
    x->rightPos = y->pos;
    y->leftID = T2->key;
    y->leftPos = T2->pos;

    // Update heights
    y->height = max(setupheight(y->leftID, y->leftPos), setupheight(y->rightID, y->rightPos)) + 1;
    oram->maxheight = max(y->height,oram->maxheight);
    oram->setupWriteN(y->key, y, rootKey,  pos);
    x->height = max(setupheight(x->leftID, x->leftPos), setupheight(x->rightID, x->rightPos)) + 1;
    oram->maxheight = max(x->height,oram->maxheight);
    oram->setupWriteN(x->key, x, rootKey,  pos);
    // Return new root
delete T2;
    return x;
}
Node* AVLTree::rightRotate(Node* y) {
    Node* x = oram->ReadNode(y->leftID,y->leftPos,y->leftPos);
    Node* T2;
    if (x->rightID == 0) {
        T2 = newNode(0, "");
    } else {
        T2 = oram->ReadNode(x->rightID,x->rightPos,x->rightPos);
    }

    // Perform rotation
    x->rightID = y->key;
    x->rightPos = y->pos;
    y->leftID = T2->key;
    y->leftPos = T2->pos;

    // Update heights
    y->height = max(height(y->leftID, y->leftPos), height(y->rightID, y->rightPos)) + 1;
    oram->maxheight = max(y->height,oram->maxheight);
    oram->WriteNode(y->key, y);
    x->height = max(height(x->leftID, x->leftPos), height(x->rightID, x->rightPos)) + 1;
    oram->maxheight = max(x->height,oram->maxheight);
    oram->WriteNode(x->key, x);
    // Return new root

delete T2;
    return x;
}


Node* AVLTree::setupleftRotate(Node* x,Bid rootKey, int& pos) {
    Node* y = oram->setupReadN(x->rightID,x->rightPos);
    Node* T2;
    if (y->leftID == 0) {
        T2 = setupnewNode(0, "");
    } else {
        //T2 = oram->setupReadN(y->leftID,y->leftPos);
	T2 = new Node();
        oram->setupReadN(T2,y->leftID,y->leftPos);
    }


    // Perform rotation
    y->leftID = x->key;
    y->leftPos = x->pos;
    x->rightID = T2->key;
    x->rightPos = T2->pos;

    // Update heights
    x->height = max(setupheight(x->leftID, x->leftPos), setupheight(x->rightID, x->rightPos)) + 1;
    oram->maxheight = max(x->height,oram->maxheight);
    oram->setupWriteN(x->key, x, rootKey,  pos);
    y->height = max(setupheight(y->leftID, y->leftPos), setupheight(y->rightID, y->rightPos)) + 1;
    oram->maxheight = max(y->height,oram->maxheight);
    oram->setupWriteN(y->key, y, rootKey,  pos);
    // Return new root
    return y;
}
Node* AVLTree::leftRotate(Node* x) {
    Node* y = oram->ReadNode(x->rightID,x->rightPos,x->rightPos);
    Node* T2;
    if (y->leftID == 0) {
        T2 = newNode(0, "");
    } else {
        T2 = oram->ReadNode(y->leftID,y->leftPos,y->leftPos);
    }


    // Perform rotation
    y->leftID = x->key;
    y->leftPos = x->pos;
    x->rightID = T2->key;
    x->rightPos = T2->pos;

    // Update heights
    x->height = max(height(x->leftID, x->leftPos), height(x->rightID, x->rightPos)) + 1;
    oram->maxheight = max(x->height,oram->maxheight);
    oram->WriteNode(x->key, x);
    y->height = max(height(y->leftID, y->leftPos), height(y->rightID, y->rightPos)) + 1;
    oram->maxheight = max(y->height,oram->maxheight);
    oram->WriteNode(y->key, y);
    // Return new root
    return y;
}

// Get Balance factor of node N

int AVLTree::setupgetBalance(Node* N) {
    if (N == NULL)
        return 0;
    return setupheight(N->leftID, N->leftPos) - setupheight(N->rightID, N->rightPos);
}
int AVLTree::getBalance(Node* N) {
    if (N == NULL)
        return 0;
    return height(N->leftID, N->leftPos) - height(N->rightID, N->rightPos);
}

Bid AVLTree::setupinsert(Bid rootKey, int& pos, Bid key, string value) 
{
    if (rootKey == 0) {
        Node* nnode = setupnewNode(key, value);
        pos = oram->setupWriteN(key, nnode,key,pos);
        //return nnode->key;
	
	Bid nnk = nnode->key;
	delete nnode;
	return nnk;
    }
    Node* node = new Node();
    oram->setupReadN(node, rootKey, pos);
    //Node* node = oram->setupReadN(rootKey, pos);
    if (key < node->key) {

        node->leftID = setupinsert(node->leftID, node->leftPos, key, value);
    } else if (key > node->key) {
        node->rightID = setupinsert(node->rightID, node->rightPos, key, value);
    } else {
        std::fill(node->value.begin(), node->value.end(), 0);
        std::copy(value.begin(), value.end(), node->value.begin());
        oram->setupWriteN(rootKey, node,rootKey, pos);
        //return node->key;
	Bid nodekey = node->key;
	return nodekey;
    }

    node->height = max(setupheight(node->leftID, node->leftPos), setupheight(node->rightID, node->rightPos)) + 1;
    oram->maxheight = max(node->height,oram->maxheight);

    int balance = setupgetBalance(node);
    
    if (balance > 1 && key < node->leftID){// oram->setupReadN(node->leftID,node->leftPos)->key) {
    //cout <<" Left Left Case-----------------------------------"<<endl;
        Node* res = setuprightRotate(node, rootKey, pos);
        pos = res->pos;
	//return res->key;
	
	Bid nnk = res->key;
	delete res;
	return nnk;
        
    }

    if (balance < -1 && key > node->rightID){//oram->setupReadN(node->rightID,node->rightPos)->key) {
    //cout <<" Right Right Case-----------------------------------"<<endl;
        Node* res = setupleftRotate(node, rootKey, pos);
        pos = res->pos;
        //return res->key;
	
	Bid nnk = res->key;
	delete res;
	return nnk;
    }
    if (balance > 1 && key > node->leftID){//oram->setupReadN(node->leftID,node->leftPos)->key) {
//    cout <<" Left Right Case-----------------------------------"<<endl;
        Node* nodel = new Node();
        oram->setupReadN(nodel,node->leftID,node->leftPos);
        Node* res = setupleftRotate(nodel,rootKey,pos);//oram->setupReadN(node->leftID,node->leftPos),rootKey, pos);
        node->leftID = res->key;
        node->leftPos = res->pos;
        oram->setupWriteN(node->key, node, rootKey,pos);
        Node* res2 = setuprightRotate(node, rootKey, pos);
        pos = res2->pos;
       // return res2->key;
	
	Bid nnk = res2->key;
	delete res2;
	delete res;
	delete nodel;
	return nnk;
    }

    if (balance < -1 && key < node->rightID){//oram->setupReadN(node->rightID,node->rightPos)->key) {
//    cout <<" Right Left Case-----------------------------------"<<endl;
        Node* noder = new Node();
        oram->setupReadN(noder,node->rightID,node->rightPos);
        auto res = setuprightRotate(noder,rootKey,pos);//oram->setupReadN(node->rightID,node->rightPos),rootKey, pos);
        node->rightID = res->key;
        node->rightPos = res->pos;
        oram->setupWriteN(node->key, node, rootKey,pos);
        Node* res2 = setupleftRotate(node, rootKey, pos);
        pos = res2->pos;
        //return res2->key;
	
	Bid nnk = res2->key;
	delete res2;
	delete res;
	delete noder;
	return nnk;
    }

    /* return the (unchanged) node pointer */
    oram->setupWriteN(node->key, node, rootKey,pos);
    //return node->key;

        Bid nnk = node->key;
	delete node;
	return nnk;
}



Bid AVLTree::insert(Bid rootKey, int& pos, Bid key, string value) 
{
    /* 1. Perform the normal BST rotation */
    if (rootKey == 0) {
        Node* nnode = newNode(key, value);
        pos = oram->WriteNode(key, nnode);
        return nnode->key;
    }
    Node* node = oram->ReadNode(rootKey, pos, pos);
    if (key < node->key) {
        node->leftID = insert(node->leftID, node->leftPos, key, value);
    } else if (key > node->key) {
        node->rightID = insert(node->rightID, node->rightPos, key, value);
    } else {
        std::fill(node->value.begin(), node->value.end(), 0);
        std::copy(value.begin(), value.end(), node->value.begin());
        oram->WriteNode(rootKey, node);
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
    if (balance > 1 && key < oram->ReadNode(node->leftID)->key) {
	    //cout <<"Left left "<<endl;
        Node* res = rightRotate(node);
        pos = res->pos;
        return res->key;
    }

    // Right Right Case
    if (balance < -1 && key > oram->ReadNode(node->rightID)->key) {
	    //cout <<"right right "<<endl;
        Node* res = leftRotate(node);
        pos = res->pos;
        return res->key;
    }

    // Left Right Case
    if (balance > 1 && key > oram->ReadNode(node->leftID)->key) {
	    //cout <<"left right "<<endl;
        Node* res = leftRotate(oram->ReadNode(node->leftID));
        node->leftID = res->key;
        node->leftPos = res->pos;
        oram->WriteNode(node->key, node);
        Node* res2 = rightRotate(node);
        pos = res2->pos;
        return res2->key;
    }

    // Right Left Case
    if (balance < -1 && key < oram->ReadNode(node->rightID)->key) {
	    //cout <<"right left"<<endl;
        auto res = rightRotate(oram->ReadNode(node->rightID));
        node->rightID = res->key;
        node->rightPos = res->pos;
        oram->WriteNode(node->key, node);
        auto res2 = leftRotate(node);
        pos = res2->pos;
        return res2->key;
    }

    /* return the (unchanged) node pointer */
    oram->WriteNode(node->key, node);
    return node->key;
}

/**
 * a recursive search function which traverse binary tree to find the target node
 */
Node* AVLTree::setupsearch(Node* head, Bid key) {
    if (head == NULL || head->key == 0)
        return head;
    head = oram->setupReadN(head->key, head->pos);
    if (head->key > key) {
        return setupsearch(oram->setupReadN(head->leftID, head->leftPos), key);
    } else if (head->key < key) {
        return setupsearch(oram->setupReadN(head->rightID, head->rightPos), key);
    } else if(head->key == key)
        return head;
}
string AVLTree::setupsimplesearch(Bid rkey, int rpos, Bid key) {
    if (rkey == 0)
        return "";
    Node *head = new Node();
    oram->setupReadN(head,rkey, rpos);
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
Node* AVLTree::search(Node* head, Bid key) {
    if (head == NULL || head->key == 0)
    {

    	searchf_bytes = oram->searchf_bytes;
        return head;
    }
    head = oram->ReadNode(head->key, head->pos, head->pos);
    if (head->key > key) {
    	searchf_bytes = oram->searchf_bytes;
        return search(oram->ReadNode(head->leftID, head->leftPos, head->leftPos), key);
    } else if (head->key < key) {
    	searchf_bytes = oram->searchf_bytes;
        return search(oram->ReadNode(head->rightID, head->rightPos, head->rightPos), key);
    } else
    {
    	searchf_bytes = oram->searchf_bytes;
        return head;
    }
}

/**
 * a recursive search function which traverse binary tree to find the target node
 */
void AVLTree::batchSearch(Node* head, vector<Bid> keys, vector<Node*>* results) {
    if (head == NULL || head->key == 0) {
        return;
    }
    head = oram->ReadNode(head->key, head->pos, head->pos);
    bool getLeft = false, getRight = false;
    vector<Bid> leftkeys,rightkeys;
    for (Bid bid : keys) {
        if (head->key > bid) {
            getLeft = true;
            leftkeys.push_back(bid);
        }
	else if (head->key < bid) {
            getRight = true;
            rightkeys.push_back(bid);
        }
	else if (head->key == bid) {
            results->push_back(head);
        }
    }
    if (getLeft) {
        batchSearch(oram->ReadNode(head->leftID, head->leftPos, head->leftPos), leftkeys, results);
    }
    if (getRight) {
        batchSearch(oram->ReadNode(head->rightID, head->rightPos, head->rightPos), rightkeys, results);
    }
    searchf_bytes = oram->searchf_bytes;
}

void AVLTree::printTree(Node* root, int indent) {
    if (root != 0 && root->key != 0) {
        root = oram->ReadNode(root->key, root->pos, root->pos);
        if (root->leftID != 0)
            printTree(oram->ReadNode(root->leftID, root->leftPos, root->leftPos), indent + 4);
        if (indent > 0)
            cout << setw(indent) << " ";
        string value;
        value.assign(root->value.begin(), root->value.end());
        cout << root->key << ":" << value.c_str() << ":" << root->pos << ":" << root->leftID << ":" << root->leftPos << ":" << root->rightID << ":" << root->rightPos << endl;
        if (root->rightID != 0)
            printTree(oram->ReadNode(root->rightID, root->rightPos, root->rightPos), indent + 4);

    }
}

/*
 * before executing each operation, this function should be called with proper arguments
 */
void AVLTree::startOperation(bool batchWrite) {
    oram->start(batchWrite);
    searchf_bytes = 0;
}

/*
 * after executing each operation, this function should be called with proper arguments
 */
void AVLTree::finishOperation(bool find, Bid& rootKey, int& rootPos) {
    searchf_bytes = 0;
    oram->finilize(find, rootKey, rootPos);
}

int AVLTree::RandomPath() {
    int val = dis(mt);
    return val;
}


int AVLTree::notsoRandomPath() 
{
    if(setupleaf != totalleaves)
	    setupleaf++;
    else
	    setupleaf = 0;
    return setupleaf;
}

//delete functions
Node* AVLTree::minValueNode(Bid rootKey, int rootPos, Node* rootroot)
{
	Node* curNode = oram->ReadNode(rootKey,rootPos,rootPos);
	if(curNode == NULL || curNode->key ==0)
		return rootroot;
	else
		return minValueNode(curNode->leftID,curNode->leftPos, curNode);
}


Node* AVLTree::parentOf(Bid parentKey, int ppos, Bid childKey, int cpos, Bid key)
{
	Node* parNode = oram->ReadNode(parentKey,ppos,ppos);
	//cout <<"parent key of child key is"<<parNode->key<<childKey<<endl;
	if(key == parNode->key)
		return NULL;
	else
	{
		Node* cNode = oram->ReadNode(childKey,cpos,cpos);
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




Bid AVLTree::balance(Node* node, int &pos)
{
    Bid key = node->key;
    int balance = getBalance(node);
    //cout <<"balance is:"<<balance<<endl;
    if (balance > 1 )
    {
	    Node* leftChild = oram->ReadNode(node->leftID,node->leftPos,node->leftPos);
    	    if(getBalance(leftChild)>=0)
	    {
	    	//cout <<"Left Left Case" <<endl;
        	Node* res = rightRotate(node);
        	pos = res->pos;
        	return res->key;
	    }
	    if(getBalance(leftChild)<0)
	    {
	    	 //cout <<"Left Right Case" <<endl;
		 Node* res = leftRotate(leftChild);
        	 node->leftID = res->key;
        	 node->leftPos = res->pos;
        	 oram->WriteNode(node->key, node);
		 Node* res2 = rightRotate(node);
		 pos = res2->pos;
		 return res2->key;
	    }
    }
    if(balance < -1)
    {
	    Node* rightChild=oram->ReadNode(node->rightID,node->rightPos,node->rightPos);
	    if(getBalance(rightChild)<=0)
	    {
	    	//cout <<"Right Right Case" <<endl;
		Node* res = leftRotate(node);
		pos = res->pos;
		return res->key;
	    }
	    if(getBalance(rightChild)>0)
	    {
	    	    //cout <<"Right Left Case" <<endl;
		    Node* res = rightRotate(rightChild);
		    node->rightID = res->key;
		    node->rightPos = res->pos;
		    oram->WriteNode(node->key,node);
		    Node* res2 = leftRotate(node);
		    pos = res2->pos;
		    return res2->key;
	    }
    }
    //oram->WriteNode(node->key, node);
    return node->key;
}



int AVLTree::deleteNode(Node* nodef)
{
	Node* free = newNode(0,"");
	oram->DeleteNode(nodef->key,free);
	return 0;
}


Bid AVLTree:: removeMain(Bid rootKey,int& pos, Bid delKey)
{
	if(rootKey != delKey || rootKey <delKey || rootKey > delKey)
	{// it does not work without these 3 checks ??
		//cout<<"rootKey != delKey(removeMain)"<<endl;
		Node* paren = parentOf(rootKey,pos,rootKey,pos,delKey);
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


Bid AVLTree::removeRoot(Bid rootKey, int& pos)
{
	Node* delnode =oram->ReadNode(rootKey,pos,pos); 
	Bid delKey = rootKey;
Node* b=oram->ReadNode(delnode->rightID,delnode->rightPos,delnode->rightPos);
	Node* minnode;
	if(b == NULL || b->key == 0)
	{
		//cout << "the min value node is->"<< delnode->key<<endl;
		minnode = delnode;
	}
	else
	{
		Node* mn = minValueNode(b->key,b->pos,b);
		//cout << "the min value node is:"<< mn->key<<endl;
		minnode = oram->ReadNode(mn->key,mn->pos,mn->pos);
	}
	
	if(delKey == minnode->key)
	{//no right child of delKey
		//cout <<"FIRST CASE:rootKey == minnode->key"<< endl;
Node* lc = oram->ReadNode(delnode->leftID,delnode->leftPos,delnode->leftPos);
		if(lc == NULL || lc->key ==0)
			lc = newNode(0,"");
		pos = lc->pos;
		deleteNode(delnode);
		return lc->key;
	}
	else
	{

		Node* pm=parentOf(rootKey,pos,rootKey,pos,minnode->key);
		Node* parmin = oram->ReadNode(pm->key,pm->pos,pm->pos);
		//cout << "parent of minnode is:"<< parmin->key<<endl;
		if(delKey == parmin->key)
		{//no leftchild of minnode //minnode is delKey's right child
		//	cout <<"SECOND CASE: rootKey == parmin->key"<< endl;
	Node* lc = oram->ReadNode(delnode->leftID,delnode->leftPos,delnode->leftPos);
			if(lc == NULL || lc->key ==0)
				lc = newNode(0,"");
			minnode->leftID = lc->key;
			minnode->leftPos = lc->pos;
			minnode->height =  max(height(minnode->leftID,minnode->leftPos), height(minnode->rightID, minnode->rightPos)) + 1;
    			//oram->maxheight = max(minnode->height,oram->maxheight);
			oram->WriteNode(minnode->key,minnode);
			int minPos = minnode->pos;
			Bid minKey = balance(minnode,minnode->pos);
			minnode = oram->ReadNode(minKey,minPos,minPos);
			pos = minnode->pos;
			deleteNode(delnode);
			return minnode->key;
		}
		else //minnode does not have leftID in general
		{//in this case minnode is parmin's left child always
		//	cout <<"THIRD case"<< endl;	
	Node* rc = oram->ReadNode(minnode->rightID,minnode->rightPos,minnode->rightPos);
			if(rc == NULL || rc->key ==0)
				rc = newNode(0,"");
			parmin->leftID = rc->key;
			parmin->leftPos = rc->pos;
			parmin->height = max(height(parmin->leftID,parmin->leftPos), height(parmin->rightID, parmin->rightPos)) + 1;
    			//oram->maxheight = max(parmin->height,oram->maxheight);
			oram->WriteNode(parmin->key,parmin);
			minnode->leftID = delnode->leftID;
			minnode->leftPos = delnode->leftPos;
			minnode->rightID = delnode->rightID;
			minnode->rightPos = delnode->rightPos;
			minnode->height = max(height(minnode->leftID,minnode->leftPos), height(minnode->rightID, minnode->rightPos)) + 1;
    			//oram->maxheight = max(minnode->height,oram->maxheight);
			oram->WriteNode(minnode->key,minnode);
			int minPos = minnode->pos;
			Bid minKey = balanceDel(minnode->key,minnode->pos, parmin);
			minnode = oram->ReadNode(minKey,minPos,minPos);
			pos = minnode->pos;
			deleteNode(delnode);
			return minnode->key;
		}	
	}
}


Bid AVLTree::removeDel(Bid rootKey,int& pos,Bid delKey,int delPos,Node* paren)
{
	Node* node = oram->ReadNode(rootKey, pos, pos);
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
    Bid key = node->key;
    if (balance > 1 )
    {
	    Node* leftChild = oram->ReadNode(node->leftID,node->leftPos,node->leftPos);
    	    if(getBalance(leftChild)>=0)
	    {
	    	//cout <<"Left Left Case" <<endl;
        	Node* res = rightRotate(node);
        	pos = res->pos;
        	return res->key;
	    }
	    if(getBalance(leftChild)<0)
	    {
	    	 //cout <<"Left Right Case" <<endl;
		 Node* res = leftRotate(leftChild);
        	 node->leftID = res->key;
        	 node->leftPos = res->pos;
        	 oram->WriteNode(node->key, node);
		 Node* res2 = rightRotate(node);
		 pos = res2->pos;
		 return res2->key;
	    }
    }
    if(balance < -1)
    {
	    Node* rightChild=oram->ReadNode(node->rightID,node->rightPos,node->rightPos);
	    if(getBalance(rightChild)<=0)
	    {
	        //cout <<"Right Right Case" <<endl;
		Node* res = leftRotate(node);
		pos = res->pos;
		return res->key;
	    }
	    if(getBalance(rightChild)>0)
	    {
	            //cout <<"Right Left Case" <<endl;
		    Node* res = rightRotate(rightChild);
		    node->rightID = res->key;
		    node->rightPos = res->pos;
		    oram->WriteNode(node->key,node);
		    Node* res2 = leftRotate(node);
		    pos = res2->pos;
		    return res2->key;
	    }
    }

    oram->WriteNode(node->key, node);
    return node->key;
}



Bid AVLTree::realDelete(Node* paren,Bid delKey,int delPos)
{
	Node* delnode =oram->ReadNode(delKey,delPos,delPos); 
Node* b=oram->ReadNode(delnode->rightID,delnode->rightPos,delnode->rightPos);
	Node* minnode;
	if(b == NULL || b->key == 0)
	{
		//cout << "the min value node is->"<< delnode->key<<endl;
	minnode = oram->ReadNode(delnode->key,delnode->pos,delnode->pos);
	}
	else
	{
		Node* mn = minValueNode(b->key,b->pos,b);
		//cout << "the min value node is:"<< mn->key<<endl;
		minnode = oram->ReadNode(mn->key,mn->pos,mn->pos);
	}
Node* pm=parentOf(paren->key,paren->pos,paren->key,paren->pos,minnode->key);
	Node* parmin = oram->ReadNode(pm->key,pm->pos,pm->pos);
	//cout << "parent of minnode is:"<< parmin->key<<endl;
	if(delKey == minnode->key)//paren->key == parmin->key
	{//no right child of delKey
	//	cout <<"FIRST CASE:delKey == minnode->key"<< endl;
Node* lc = oram->ReadNode(delnode->leftID,delnode->leftPos,delnode->leftPos);
		if(lc == NULL || lc->key ==0)
			lc = newNode(0,"");
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
		oram->WriteNode(paren->key,paren);
		deleteNode(delnode);
		return paren->key;
	}
	else if(delKey == parmin->key)
	{//no leftchild of minnode //minnode is delKey right child
	//	cout <<"SECOND CASE: delKey == parmin->key"<< endl;
Node* lc = oram->ReadNode(delnode->leftID,delnode->leftPos,delnode->leftPos);
		if(lc == NULL || lc->key ==0)
			lc = newNode(0,"");
		minnode->leftID = lc->key;
		minnode->leftPos = lc->pos;
		minnode->height =  max(height(minnode->leftID,minnode->leftPos), height(minnode->rightID, minnode->rightPos)) + 1;
    		//oram->maxheight = max(minnode->height,oram->maxheight);
		oram->WriteNode(minnode->key,minnode);
		int minPos = minnode->pos;
		Bid minKey = balance(minnode,minnode->pos);
		minnode = oram->ReadNode(minKey,minPos,minPos);
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
		oram->WriteNode(paren->key,paren);
		deleteNode(delnode);
		return paren->key;
	}
	else //minnode does not have leftID in general
	{//in this case minnode is parmin's left child always
		//cout <<"THIRD case"<< endl;	
Node* rc = oram->ReadNode(minnode->rightID,minnode->rightPos,minnode->rightPos);
		if(rc == NULL || rc->key ==0)
			rc = newNode(0,"");
		parmin->leftID = rc->key;
		parmin->leftPos = rc->pos;
		parmin->height = max(height(parmin->leftID,parmin->leftPos), height(parmin->rightID, parmin->rightPos)) + 1;
    		//oram->maxheight = max(parmin->height,oram->maxheight);
		oram->WriteNode(parmin->key,parmin);
		minnode->leftID = delnode->leftID;
		minnode->leftPos = delnode->leftPos;
		minnode->rightID = delnode->rightID;
		minnode->rightPos = delnode->rightPos;
		minnode->height = max(height(minnode->leftID,minnode->leftPos), height(minnode->rightID, minnode->rightPos)) + 1;
    		//oram->maxheight = max(minnode->height,oram->maxheight);
		oram->WriteNode(minnode->key,minnode);
		int minPos = minnode->pos;
		Bid minKey = balanceDel(minnode->key,minnode->pos, parmin);
		minnode = oram->ReadNode(minKey,minPos,minPos);
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
		oram->WriteNode(paren->key,paren);
		deleteNode(delnode);
		return paren->key;
	}	
}


Bid AVLTree::balanceDel(Bid key, int& pos, Node* parmin)
{
	Node* node = oram->ReadNode(key,pos,pos);
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
	    Node* leftChild = oram->ReadNode(node->leftID,node->leftPos,node->leftPos);
    	    if(getBalance(leftChild)>=0)
	    {
	    	//cout <<"Left Left Case" <<endl;
        	Node* res = rightRotate(node);
        	pos = res->pos;
        	return res->key;
	    }
	    if(getBalance(leftChild)<0)
	    {
	    	 //cout <<"Left Right Case" <<endl;
		 Node* res = leftRotate(leftChild);
        	 node->leftID = res->key;
        	 node->leftPos = res->pos;
        	 oram->WriteNode(node->key, node);
		 Node* res2 = rightRotate(node);
		 pos = res2->pos;
		 return res2->key;
	    }
    }
    if(balance < -1)
    {
	    Node* rightChild=oram->ReadNode(node->rightID,node->rightPos,node->rightPos);
	    if(getBalance(rightChild)<=0)
	    {
	    	//cout <<"Right Right Case" <<endl;
		Node* res = leftRotate(node);
		pos = res->pos;
		return res->key;
	    }
	    if(getBalance(rightChild)>0)
	    {
	    	    //cout <<"Right Left Case" <<endl;
		    Node* res = rightRotate(rightChild);
		    node->rightID = res->key;
		    node->rightPos = res->pos;
		    oram->WriteNode(node->key,node);
		    Node* res2 = leftRotate(node);
		    pos = res2->pos;
		    return res2->key;
	    }
    }

  //}
    oram->WriteNode(node->key, node);
    return node->key;
}
void AVLTree::setupInsert(Bid& rootKey, int& rootPos, map<Bid, string> pairs) {
    for (auto pair : pairs) {
        Node* node = newNode(pair.first, pair.second);
        setupNodes.push_back(node);
    }
    cout << "Creating BST" << endl;
    sortedArrayToBST(0, setupNodes.size() - 1, rootPos, rootKey);
    cout << "Inserting in ORAM" << endl;
    oram->setupInsert(setupNodes);
}

int AVLTree::sortedArrayToBST(int start, int end, int& pos, Bid& node) {
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
    Node *root = setupNodes[mid];

    int leftHeight = sortedArrayToBST(start, mid - 1, root->leftPos, root->leftID);

    int rightHeight = sortedArrayToBST(mid + 1, end, root->rightPos, root->rightID);
    root->pos = RandomPath();
    root->height = max(leftHeight, rightHeight) + 1;
    pos = root->pos;
    node = root->key;
    return root->height;
}
