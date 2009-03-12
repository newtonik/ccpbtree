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
template <typename _Key, typename _Datatype,typename _NodeSlots, typename _LeafSlots, typename _Compare = std::less<_Key> >
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
    static const unsigned short bt_leafnodemax;
    //Min leaf node data slots
    static const unsigned short bt_leafnodemin;
    //Max inner node data slots
    static const unsigned short bt_innernodemax;
    //Min iiner node data slots
    static const unsigned short bt_innernodemin;

private:
    //Private data structures
    struct node
    {
	//total number of slots 
	unsigned short slots;
	//total number of slots in use
	unsigned short slotsinuse;
	//if its a leafnode
	bool leafnode;
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
	keytype slots[10];
	//pointer to the first child
	node* firstChild;
	//number of slots in use in child
	int numChildren;
	inline void initialize()
	{
	    node::leafnode = false;
	}
	inline bool equal(const innerNode& n)
	{
	    if(node::keyCount() != n->keyCount())
		return false;
	    for(int i = 0; i < node::slots; i++)
		if(slots[i] != n.slots[i])
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
    };

    //inherit node for leaf node
    struct leafNode : public node
    {
	//array bounds are hardcoded for now...that will change very soon. 
	data_type dataslots[10];

	//keep track of the next and previous nodes
	//They will be helpful for range queries
	leafNode* nextleaf;
	leafNode* prevleaf;
	inline void initialize()
	{
	    node::leafnode = true;
	    prevleaf=nextleaf = NULL;
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

    };
};


#endif
