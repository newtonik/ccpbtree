#ifndef _BPTREE_H_
#define _BPTREE_H_


#include <iostream>
#include <ostream>
#include <assert.h>


#ifdef BTREE_DEBUG

#define TREE_PRINT(x)	do { if (debug) (std::cout <<x << endl; ); } while(0)

#define TREE_ASSERT(x)  do { assert(x); } while(0)


#endif

using namespace std;

//class for the btree
template <typename _Key, typename _Datatype,int _nodeslots, int _leafslots, typename _Compare = std::less<_Key> >
class btree {
public:
    //key type for this current instance of the btree.
    typedef _Key keytype;
    //Data type for current instance of the btree
    typedef _Datatype data_type;

    typedef  std::pair<_Key, _Datatype> pair_type;

    //Key comparison function
    typedef  _Compare  key_compare;

private:
    //D-value of the tree, leaf node will always can hold 2d keys.
    static const unsigned short bt_nodenum;
    //Number of nodes in the tree
    short bt_numNodes;
    //Max leaf node data slots
    static const unsigned short bt_leafnodemax = _leafslots;
    //Min leaf node data slots
    static const unsigned short bt_leafnodemin = _leafslots/2;
    //Max inner node data slots
    static const unsigned short bt_innernodemax = _nodeslots;
    //Min iiner node data slots
    static const unsigned short bt_innernodemin = _nodeslots/2;

private:
    struct innerngroup; 
    //Private data structures
    struct node
    {
	//total number of slots 
	unsigned short slots;
	//total number of slots in use
	unsigned short slotsinuse;
	//if its a leafnode
	bool leafnode;
	bool root_node;
	inline bool isleaf() const
	{
	    return leafnode;
	}
	inline unsigned short keyCount() const
	{
	    return slotsinuse;
	}
    };
    //inherit node for inner node
    struct innerNode : public node
    {
	//slots for keys....inner nodes only have slots of keys and pointers
	keytype* keySlots[node::slots];
	//pointer to the first child, this is the pointer to the main segment
	innerngroup* firstChild;
	//number of child nodes present, max can be node:slots + 1
	int numChildren;
	inline void initialize()
	{
	    node::slots = bt_innernodemax;
	    node::leafnode = false;
	    firstChild = NULL;
	    keySlots = new keytype[node::slots];
	}
	inline bool equal(const innerNode& n)
	{
	    if(node::keyCount() != n->keyCount())
		return false;
	    for(int i = 0; i < node::slotsinuse; i++)
		if(keySlots[i] != n.keySlots[i])
		    return false;
	    return true;
	}
	//check for underflow
	inline bool isunderflow() const
	{
	    return(node::slotsinuse < bt_innernodemin);
	}
	//check if nodes slots are full. 
	inline bool isfull()
	{
	    return(node::slotsinuse == bt_innernodemax);
	};
	inline bool insertChild(keytype k, node* child)
	{
	    return true;
	}
	inline bool findKey(keytype k)
	{
	    for(int i = 0; i < node::slotsinuse; i++)
		if(keySlots[i] == k )
		    return true;
	    return false;
	}
	inline innerngroup* getChild (int i)
	{
	   if(firstChild == NULL)
	    return NULL;
	   else
	       return firstChild.group[0];

	}

    };

    //inherit node for leaf node
    struct leafNode : public node
    {
	//keys for leaf nodes
	keytype* keySlots[node::slot];
	data_type* dataSlots[node::slot];

	//keep track of the next and previous nodes
	//They will be helpful for range queries
	leafNode* nextleaf;
	leafNode* prevleaf;
	inline void initialize()
	{
	    node::slots = bt_leafnodemax;
	    node::leafnode = true;
	    prevleaf=nextleaf = NULL;
	    dataSlots = new data_type[node::slot];
	    keySlots = new keytype[node::slot];
	}
	
	inline bool equal(const leafNode& n)
	{
	    if(node::keyCount() != n->keyCount())
		return false;
	    for(int i = 0; i < node::slotsinuse; i++)
		if(dataSlots[i] != n.dataSlots[i])
		    return false;
	    return true;
	}
	//check if node is full
	inline bool isfull() const
	{
	    return (node::slotsinuse == bt_leafnodemax);
	}
	//check if there are node is less than full
	inline bool isfew() const
	{
	    return (node::slotsinuse <= bt_leafnodemin);
	}
	//check if there is an underflow
	inline bool isunderflow()const
	{   
	    return (node::slotsinuse < bt_leafnodemin);
	}
	inline bool insertData(data_type* data)
	{
	    for(int i = 0; i < node::slotsinuse; i++)
	    {
		//dosomething
	    }
	    return false;
	}
    };

    struct innerngroup 
    {
	innerNode* group[bt_innernodemax];
	inline void initialize()
	{
	    group = new innerNode[bt_innernodemax];
	}
    };

    struct leafngroup
    {
    };
    //public methods
public:
    node* root;
    inline btree()
    {
	root = NULL;
    }
    inline node* getRoot()
    {
	return root;
    }
    //size, this will return the number of data values in the treee
    int size();
    //returns true if there is a least 1 data/key value in the tree
    bool empty();
    //get leaf slot size
    int keyslotsize();
    //get node slot size
    int nodeslotsize();
    //erase 
    void erase();
    /**
     * Delete key, data pair from index 
     */
    void delete_pair(keytype k, data_type data);

    /**
     *Find Key in tree.
     **/
    inline bool find(keytype k)
    {
	int i = 0;
	node* curNode = root;
	keytype* ki = NULL; 

	if(btree::empty())
	    return false;

	while(!curNode->isleaf()) {
	for(i = 0; i < curNode.keyCount(); i++)
	{
	    TREE_PRINT("looking for key " << curNode << "and at index" << i);
	    if(key_compare(curNode->keySlots[i], k) > 0)
	    {
		//found a key that is greater than the look being searched
		ki = curNode->keySlots[i]; 
		break;
	    }
	}
	    //this key is greater than all keys in node
	    if(ki == NULL)
	    {
		if(curNode->numChildren > 0)
		    curNode = curNode->getChild(curNode->numChildren - 1);
		TREE_PRINT("returning node child " << curNode->numChildren-1);
	    }
	    else
	    {
		curNode = curNode->getChild(i);

	    }


	}	
	//if we are out of the loop it means we have reached a leaf node
	for(i = 0; i < curNode.keyCount(); i++)
	{
	    if(key_compare(curNode->keySlots[i], k) == 0)
	    {
		TREE_PRINT("Found key at key slot " << i);
		return true;
	    }
	}
	

	return false;
    }

private:

    void insert_in_parent(node* N, keytype k, node* Nprime);
};


#endif
