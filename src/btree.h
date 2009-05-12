#ifndef _BPTREE_H_
#define _BPTREE_H_


#include <iostream>
#include <ostream>
#include <assert.h>


#ifdef BTREE_DEBUG

#define TREE_PRINT(x)	do { if (debug) (std:://cout <<x << endl; ); } while(0)

#define BTREE_ASSERT(x)  do { assert(x); } while(0)



#endif

using namespace std;

namespace nwt {
    //class for the btree

    template <typename _Key, typename _Datatype, int _nodeslots, int _leafslots, typename _Compare = std::less<_Key> >
            class btree {
    public:
        //key type for this current instance of the btree.
        typedef _Key keytype;
        //Data type for current instance of the btree
        typedef _Datatype data_type;

        typedef std::pair<_Key, _Datatype> pair_type;

        typedef std::pair<_Key, bool> lookuppair_type;
        //Key comparison function
        typedef _Compare key_compare;

        class iterator;
        class const_iterator;
        class reverse_iterator;
        class const_reverse_iterator;

    private:
        static const unsigned short bt_nodenum;
        //Number of nodes in the tree
        short bt_numNodes;
        //Max leaf node data slots
        static const unsigned short bt_leafnodemax = _leafslots;
        //Min leaf node data slots
        static const unsigned short bt_leafnodemin = _leafslots / 2;
        //Max inner node data slots
        static const unsigned short bt_innernodemax = _nodeslots;
        //Min iiner node data slots
        static const unsigned short bt_innernodemin = _nodeslots / 2;
        //keycompare
        key_compare keyless;


    private:
        //Private data structures

        struct node {
            //total number of slots
            unsigned short slots;
            //total number of slots in use
            unsigned short slotsinuse;
            //if its a leafnode
            bool isleafnode;
	    //if its a root
            bool isrootnode;
            node* parent;

            inline void initialize() {
                slots = 0;
                slotsinuse = 0;
                parent = NULL;
                isrootnode = false;
            }

            inline bool isleaf() const {
                return isleafnode;
            }

            inline unsigned short keyCount() const {
                return slotsinuse;
            }

            inline bool isRoot() const {
                return isrootnode;
            }

            inline void setIsRoot(bool root) {
                isrootnode = root;
            }
        };

        //inherit node for inner node
        struct innerNode : public node {
            //slots for keys....inner nodes only have slots of keys and pointers
            keytype keySlots[bt_innernodemax];
            //pointer to the first child, this is the pointer to the main segment
            node * firstChild[bt_innernodemax + 1];
            //number of child nodes present, max can be node:slots + 1
            int numChildren;
            //innernode slots
            inline void initialize() {
                node::initialize();
                node::slots = bt_innernodemax;
                node::isleafnode = false;
                for (int i = 0; i < bt_innernodemax + 1; i++)
                    firstChild[i] = NULL;
                numChildren = 0;
            }
	    //quick quess that two nodes are identical...not always correct
            inline bool equal(const innerNode & n) {
                if (node::keyCount() != n->keyCount())
                    return false;
                for (int i = 0; i < node::slotsinuse; i++)
                    if (keySlots[i] != n.keySlots[i])
                        return false;
                return true;
            }
            //check for underflow
            inline bool isunderflow() const {
                return (numChildren < (bt_innernodemin+1));
            }
            //check if nodes slots are full.

            inline bool isfull() {
                //cout << "slots size is " << node::slotsinuse << endl;
                return (node::slotsinuse == bt_innernodemax);
            };
        };

	/**
	 *Struct mean't to represent inner nodes
	 *
	 * */
        struct innerngroup {
            innerNode * group[bt_innernodemax];

            inline void initialize() {
                group = new innerNode[bt_innernodemax];
                for (int i = 0; i < bt_innernodemax; i++) {
                    group[i] = NULL;
                }
            }

            innerngroup * getNodeGroup() {
                return group;
            }

            inline void free_group() {
                for (int i = 0; i < bt_leafnodemax; i++)
                    delete group[i];

            }
        };


        //inherit node for leaf node

        struct leafNode : public node {
            //keys for leaf nodes
            keytype keySlots[bt_leafnodemax];
            data_type* dataSlots;

            //keep track of the next and previous nodes
            //They will be helpful for range queries
            leafNode* nextLeaf;
            leafNode* prevLeaf;

            inline void initialize() {
                node::initialize();
                node::slots = bt_leafnodemax;
                node::isleafnode = true;
                prevLeaf = nextLeaf = NULL;
                dataSlots = new data_type[bt_leafnodemax];
                //keySlots = new keytype[node::slot];
            }
	    /**
	     *Check that the nodes are identifical
	     *
	     **/
            inline bool equal(const leafNode &l) {
                if (node::keyCount() != l->keyCount())
                    return false;
		
		for (int i = 0; i < node::slotsinuse; i++)
                    if ((dataSlots[i] != l->dataSlots[i]) || (keySlots[i] != l->keySlots[i]))
                        return false;
                return true;
            }
            //check if node is full

            inline bool isfull() const {
                //cout << "in isfull " << endl;
                //cout << "bt leaf value is " << bt_leafnodemax << endl;
                //cout << "the value of slotsinuse is " << node::slotsinuse << endl;
                return (node::slotsinuse == bt_leafnodemax);
                //return false;
            }
            //check if there are node is less than full

            inline bool isfew() const {
                return (node::slotsinuse <= bt_leafnodemin);
            }
            //check if there is an underflow

            inline bool isunderflow()const {
                return (node::slotsinuse < bt_leafnodemin);
            }
        };
    private:
        node* root;
        //D-value of the tree, leaf node will always can hold 2d keys.
        leafNode* tailleaf;
        leafNode* headleaf;
        unsigned int totalkeycount;
        //public tree methods
    public:
        //constructor

        inline btree() {
            root = NULL;
            tailleaf = NULL;
            headleaf = NULL;
            totalkeycount = 0;
        }

        inline node* getRoot() {
            return btree::root;
        }
        //size, this will return the number of data values in the treee

        inline int size() {
            return totalkeycount;

        }

        inline bool empty() {

            if (btree::root == NULL)
                return true;
            return false;
        }
        //get leaf slot size
        int keyslotsize();
        //get node slot size
        int nodeslotsize();
        //erase
        void erase();

	/**
	 *Find the location of node c, in innerNode n
	 **/
        int findInnerNodeLoc(innerNode* n, node* c) {
            //cout << "findInnerNodeLoc" << endl;
            if ((c == NULL) || (n == NULL)) {
                //cout << "findInnerNodeLoc, child is null" << endl;
                return -1;
            }
            //cout << "parent has " << n->numChildren << " children" << endl;
             //cout << "parent has " << n->keyCount() << " keys" << endl;

            if (c->isleaf()) {
                leafNode* child = static_cast<leafNode*> (c);
                //cout << "findinnernodeloc:: inner node is leaf" << endl;
                //get the first key of the child
                keytype k = child->keySlots[0];
                int i;
                //cout << "findinnernodeloc:: search key is " << k << endl;
                for (i = 0; i < n->keyCount(); i++) {
                     //cout << "findinnernodeloc:: checking loc " << i << endl;
                    if (keygreater(n->keySlots[i], k) > 0) {
                        return i;
                    }
                }
                //check node at the end
                if(i < n->numChildren) {
                    //cout << "checking last child " << endl;
                leafNode* last = static_cast<leafNode*>(n->firstChild[i]);
                //cout << "last child's first key key is " << last->keySlots[0] << endl;
                if(keyequal(last->keySlots[0], k))
                    return i;
                }
                return -1;
                

            } else {
                innerNode* child = static_cast<innerNode*> (c);

                //get the first key of the child
                int i;
                keytype k = child->keySlots[0];
                for (i = 0; i < n->keyCount(); i++) {
                    if (keygreater(n->keySlots[i], k) > 0) {
                        return i;
                    }
                }
                if(i < n->numChildren)
                {
                    //cout << "checking last child " << endl;
                //the node is at the end
                  innerNode* last = static_cast<innerNode*>(n->firstChild[n->numChildren-1]);

                    if(keyequal(last->keySlots[0], k))
                        return i;
                }
                return i;
            }
            return -1;


        }
        void printleaves()
        {

            leafNode* l;
            //cout << "printing leaves" << endl;
            l = find(0);

            while(l != NULL) {
                for(int i = 0; i < l->keyCount(); i++)
                {
                    cout << l->keySlots[i] << endl;
                }
                l = l->nextLeaf;
                //cout << "next leaf" << endl;
            }
        }
	/**
	 *Find the location of key in innderNode
	 *
	 * */
        int findInnerNodeKey(innerNode* n, keytype k) {
            if (n == NULL)
                return -1;
            //cout << "findInnerNodeKey:: locking for key " << k << endl;
            for (int i = 0; i < n->slotsinuse; i++)
                if (keyequal(n->keySlots[i], k) > 0) {
                    return i;
                }
            return -1;
        }

        inline node* getChild(innerNode* n, int i) {
            if (i < 0 || i > n->slotsinuse)
                return NULL;
            if (n->firstChild == NULL)
                return NULL;
            else {
                //cout << "returning child " << i << endl;
                return n->firstChild[i];
            }

        }
	/**
	 *insert a key in a inner Node
	 *
	 * */
        inline int insertInnerNodeKeyAt(innerNode* n, keytype k, int index) {
            //int old = node:slotsinuse;
            int max = n->keyCount();
            max = max - 1;
            if (index >= n->slots || index < 0) {
                return -1;
            }//if there are no keys or position is at end, a simple entry.
            if ((n->keyCount() == 0) || ((n->keyCount() <= index) && (index < n->slots))) {
                n->keySlots[index] = k;
                //cout << "added key " << k << " at location " << index << endl;
                n->slotsinuse++;
                return 1;
            } else if ((index) <= n->keyCount()) {
                //loop through array to move keys to create space
                //for new key in position index
                for (int i = max + 1; i > index; i--) {
                    n->keySlots[i] = n->keySlots[i - 1];
                }

                //insert the new key
                n->keySlots[index] = k;
                n->slotsinuse++;
                //cout << "added key " << k << endl;

                return 1;
            }
            return -1;
        }

        /**
         *Insert Child at index given.
         * */
        inline int insertInnerNodeChildAt(innerNode* dest, node* n, int index) {
            //check if its out of bounds
            if (index > dest->slots || index < 0) {
                printf("Array of of bounds, %d in array of size %d\n", index, dest->slots);
                return -1;
            }
            //cout << "insert child in index " << index << endl;
            if(index < dest->numChildren)
            {
                //cout << "insertInnerNodeChildAt:: shifting nodes" << endl;
                for(int i=dest->numChildren-1; i >= index; i--)
                {
                    dest->firstChild[i+1] = dest->firstChild[i];
                    //leafNode* d = static_cast<leafNode*> (dest->firstChild[index+1]);
                       ////cout << "first key in next is " << d->keySlots[i+1] << endl;
                }
            }
            dest->firstChild[index] = static_cast<node*> (n);
            n->parent = dest;
            //cout << "a child has been added" << endl;


            //increment the number of Children
            dest->numChildren++;
            ///set the next and previous leaf pointers if the node being
            //added is a leaf
            if (n->isleaf()) {
                //set the next and previous nodes
                 leafNode* l = static_cast<leafNode*> (n);
                if (index - 1 >= 0) {
                    //cout << "setting prev leaf" << endl;

                    leafNode* d = static_cast<leafNode*> (dest->firstChild[index - 1]);
                     l->prevLeaf = d;
                    d->nextLeaf = l;
                     //cout << "last key in prev is " << d->keySlots[d->keyCount()-1] << endl;
                     //cout << "first key in cur is " << l->keySlots[0] << endl;
                }
          
                if (index + 1 < dest->numChildren) {
                    //cout << "setting next leaf" << endl;
                    
                    leafNode* d = static_cast<leafNode*> (dest->firstChild[index + 1]);
                    l->nextLeaf = d;
                    d->prevLeaf = l;
                     //cout << "last key in cur is " << l->keySlots[l->keyCount()-1] << endl;
                     //cout << "first key in next is " << d->keySlots[0] << endl;
                }
             
            }
            //cout << dest->numChildren << endl;
            return 0;
            //return -1;
        }

        /**
         * insert pair into node
         * */
        inline int insertinnernodepair(innerNode* n, keytype k, node* child) {

            int loc = -1;
            if ((child == NULL)) {
                printf("Cannot insert null keys or nodes\n");
                return -1;
            }
            assert(n->slotsinuse < n->slots);
            
               for (int i = 0; i < n->slotsinuse; i++) {
                if (keygreater(n->keySlots[i], k) > 0) {
                    loc = i;
                }

               }
            if(loc == -1)
                loc = n->slotsinuse;
            if (loc < 0) {
                //cout << "cannot find location " << endl;
                return loc;
            }

            if (loc >= 0) {
                //cout << "insertinnernodepair:: loc is " << loc << endl;
                insertInnerNodeKeyAt(n, k, loc);
                insertInnerNodeChildAt(n, child, loc);
                //cout << "there are now " << n->numChildren << " children" << endl;
                return loc;
            } else {
                printf("cannot insert pair\n");
                return loc;
            }
            return -1;
        }

        /**
         *Free all keys and children in node
         * */
        void freeInnerNode(innerNode* n) {
            for (int i = 0; i < n->keyCount(); i++) {
                delete n->firstChild[i];
            }
        }
	/***
	 *Find a key great in a leafNode
	 *
	 * **/
        inline int findKeyLoc(leafNode* l, keytype k) {
            int i = 0;

            if (l->slotsinuse == 0)
                return 0;
            //cout << "In leafnode find key Loc" << endl;
            for (i = 0; i < l->slotsinuse; i++) {
                //cout << "slot key is " << l->keySlots[i] << " and key is " << k << endl;
                if (keygreaterequal(l->keySlots[i], k) > 0)
                {
                    return i;
                }
            }
            //cout << "findkeyloc:: k is greater than everything" << endl;
            return l->slotsinuse;

        }

	/***
	 *Find the location of a key equal to k
	 *
	 * */
        inline int findKeyEqual(leafNode* l, keytype k) {
            int i = 0;
	    if(l == NULL)
		return -1;
            if (l->slotsinuse == 0)
                return 0;
            //cout << "In leafnode find key Loc" << endl;
            for (i = 0; i < l->slotsinuse; i++) {
                //cout << "slot key is " << l->keySlots[i] << " and key is " << k << endl;
                if (keyequal(l->keySlots[i], k) > 0)
                    return i;
            }
            //cout << "findkeyloc:: k is greater than everything" << endl;
            return l->slotsinuse;
        }
	/**
	 *Insert data in the index of the leafNode
	 *
	 * */
        inline int insertleafDataAt(leafNode* l, data_type data, int index) {
	    if(l == NULL)
		return -1;

            assert(l->isleaf());
            if (index >= bt_leafnodemax || index < 0) {
                return -1;
            } else {

                l->dataSlots[index] = data;
            }

            //dosomething

            return index;
        }
	/**
	 *Insert key in the index of the leafNode 
	 * */
        inline int insertleafKeyAt(leafNode* l, keytype k, int index) {
            //int old = node:slotsinuse;
	    if(l == NULL)
		return -1;
            int max = l->keyCount();
            assert(l->isleaf());
            //cout << "in insertleafkey " << k << " at index " << index << endl;
            if ((index >= l->slots) || (index < 0)) {
                return -1;
            }//if there are no keys or position is at end, a simple entry.
            if ((l->keyCount() == 0) || ((l->keyCount() <= index) && (index < l->slots))) {
                l->keySlots[index] = k;
                l->slotsinuse++;
                //cout << "keys in node" << endl;
                for (int i = 0; i < l->keyCount(); i++)
                    //cout << "key is " << l->keySlots[i] << endl;
                return 1;
            } else if ((index) <= l->keyCount()) {
                //loop through array to move keys to create space
                //for new key in position index
                //cout << "moving keys to create space" << endl;
                for (int i = max; i > index; i--) {
                    l->keySlots[i] = l->keySlots[i - 1];
                }

                //insert the new key
                l->keySlots[index] = k;
                l->slotsinuse++;
                //cout << "keys in node" << endl;
                for (int i = 0; i < l->keyCount(); i++)
                    //cout << "key is " << l->keySlots[i] << endl;
                return 1;

            }
            return 1;
        }

        inline int insertleafpair(leafNode* l, keytype k, data_type data) {
            //cout << "insertleafpair:: in leafnode insertpair" << endl;
            if (l->isfull())
                return -1;
            assert(l->isleaf());

            if ((bt_leafnodemax == l->slotsinuse))
                return -1;
            //cout << "insertleafpair:: about to find key location" << endl;
            int loc = findKeyLoc(l, k);
            //cout << "loc is " << loc << endl;
            if (loc < 0)
                return loc;

            //check for duplicates
            leafNode* cur = l;
            int j = loc;
            while(cur != NULL)
            {
                if((keyequal(cur->keySlots[j], k) > 0)) {
                    while((j < cur->keyCount()) && (keyequal(cur->keySlots[j], k) > 0))
                    {
                            if(data == cur->dataSlots[j])
                            {
                                //there is a duplicate pair return a special error.
                                cout << "duplicate pair found" << endl;
                                return -2;
                            }
                            j++;
                    }

                    j = 0;
                    cur = cur->nextLeaf;
                }
                else
                {
                    cur = NULL;
                    break;
                }
            }
            insertleafKeyAt(l, k, loc);
            insertleafDataAt(l, data, loc);
            //l->slotsinuse++;
            //cout << "insertleafpair:: key count is " << l->slotsinuse << " after insert" << endl;
            return 1;
        }

        inline int deleteleafpair(leafNode* l, keytype k, data_type data) {
            //cout << "deletepair:: entering" << endl;
            if (l->keyCount() < 1)
                return -1;
            int loc = find_lowerkey(l, k);

            //cout << "key found in leaf at slot " << loc << std::endl;
            if (keyequal(k, l->keySlots[loc])) {
                for (int i = loc; i < l->keyCount() - 1; i++) {
                    l->keySlots[i] = l->keySlots[i + 1];
                    l->dataSlots[i] = l->dataSlots[i + 1];

                }
                l->slotsinuse--;
                downkeycount();
                //cout << k << " has been deleted" << endl;
                //cout << "key count is now " << l->keyCount() << endl;
            } else {
              return -1;
            }
            return loc;
        }

        inline int deleteinnerpair(innerNode* p, keytype k, node* n) {

            //cout << "deleteinnerpair:: " << endl;
            // innerNode* inner = static_cast<innerNode*>(n);
            if (p->keyCount() < 1)
                return -1;
            int j = 0;
            for (int i = 0; i < p->keyCount(); i++) {
                if (keyequal(k, p->keySlots[i]) > 0) {
                    //cout << "deleteinnerpair:: found keyes that are equal" << endl;
                    for (j = i; i < p->keyCount() - 1; j++) {
                        p->firstChild[j] = p->firstChild[j + 1];
                        p->keySlots[j] = p->keySlots[j + 1];
                    }
                    if (j + 2 == p->numChildren) {
                        p->firstChild[j + 1] = p->firstChild[j + 2];
                    }
                    p->slotsinuse--;
                    p->numChildren--;
                    return i;
                }

            }
            return -1;
        }

        /**
         *Find Key in tree.
         **/
        inline leafNode* find(keytype k) {
            int i = 0;
            node* rootNode = root;
            node* tempNode = NULL;
            keytype ki;
            int slot = -1;
            std::pair<iterator, bool> ret;
            //cout << "FIND: in btree find" << endl;
            if (empty())
                return NULL;
            //return std::pair<iterator, bool> (end(), false);
            //cout << "FIND: not empty" << endl;

            if (rootNode->isRoot()) {
                //cout << "this is root" << endl;

            }
            if (rootNode == NULL) {
                //cout << "root is null" << endl;
            }
            while (!(rootNode->isleaf())) {
                innerNode* curNode = static_cast<innerNode*> (rootNode);
                //cout << "FIND:: checking inner nodes, this node has  " << curNode->numChildren << " children." << endl;
                //cout << "FIND:: Key is " << k << " key count is " << curNode->keyCount() << endl;
                for (i = 0; i < curNode->keyCount(); i++) {
                    //cout << "checking slot, key is " << curNode->keySlots[i] << endl;
                    // TREE_PRINT("looking for key " << k << "and at index" << i);
                    if (keygreater(curNode->keySlots[i], k) > 0) {
                        //found a key that is greater than the look being searched'
                        //cout << curNode->keySlots[i] << " is greater than " << k << endl;
                        ki = (curNode->keySlots[i]);
                        slot = i;
                        break;
                    }
                    //cout << "i is " << i << endl;
                }
         
                if (slot >= 0) {
                    //cout << "FIND:: slot is " << slot << endl;
                    if (curNode->numChildren > curNode->keyCount()) {
                        tempNode = getChild(curNode, slot);
                        if (tempNode == NULL) {
                            tempNode = getChild(curNode, curNode->numChildren - 1);
                            //cout << "tempNode is NULL" << endl;
                        }

                        rootNode = static_cast<node*> (tempNode);
                        assert(rootNode != NULL);
                    }

                    //TREE_PRINT("returning node child " << curNode->numChildren - 1);
                } else {
                    tempNode = getChild(curNode, curNode->keyCount());
                    rootNode = static_cast<node*> (tempNode);
                }
                slot = -1;

            }
            //cout << "FIND:: found the leaf I hope" << endl;
            leafNode* lnode = static_cast<leafNode*> (rootNode);

            while (lnode != NULL) {
                //if we are out of the following loop it means we have reached a leaf node, return false
                //cout << "lnode key count is " << lnode->keyCount() << endl;
                for (i = 0; i < lnode->keyCount(); i++) {
                    //cout << lnode->keySlots[i] << " is current slot key and  key " << k << " is query key" << endl;
                    if (keygreaterequal(lnode->keySlots[i], k)) {
                        //cout << lnode->keySlots[i] << " is greater or equal to " << k << endl;
                        return lnode;
                        //return std::pair<iterator, bool>(iterator(lNode, i), true);
                    }


                }
                if (lnode->isfull()) {
                    //cout << "moving to nextleaf, check next leaf" << endl;
                    if (lnode->nextLeaf != NULL)
                        lnode = static_cast<leafNode*> (lnode->nextLeaf);
                    else
                        return lnode;
                } else
                    return lnode;
            }
            //cout << "returning null" << endl;
            return lnode;
            // return std::pair<iterator, bool>(iterator(lNode, i-1), false);

            // return std::pair<iterator, bool>(end(), false);
        }

        /**
         *  just calls find and returns a bool if the key exists
         **/
        inline bool exists(keytype k) {
            leafNode* ret;
            ret = find(k);

            if (ret == NULL)
                return false;
            for (int i = 0; i < ret->keyCount(); i++) {
                if (keyequal(ret->keySlots[i], k) > 0)
                    return true;
            }
            return false;


            // return false;

        }
        inline std::pair<data_type, bool> get(keytype k) {
            leafNode* ret;
            ret = find(k);
            string empty = "";

            if (ret == NULL)
                return std::pair<data_type, bool>(empty, false);
            for (int i = 0; i < ret->keyCount(); i++) {
                if (keyequal(ret->keySlots[i], k) > 0)
                    return std::pair<data_type, bool>(ret->dataSlots[i], true);
            }
            
            return std::pair<data_type, bool>(empty, false);
        }
        inline iterator getNext(keytype k)
        {
	    leafNode* l;
	    l = find(k);
	    std::pair<iterator, bool> ret;
	    
            string empty = "";

            if (ret == NULL)
                return std::pair<data_type, bool>(empty, false);
            for (int i = 0; i < ret->keyCount(); i++) {
                if (keyequal(ret->keySlots[i], k) > 0)
                    //return std::pair<data_type, bool>(ret->dataSlots[i], true);
		{
		    iterator(l, i);
		}
	    
	    }
            
	    return end();

        }
        /**
         * Insert is a pair into the tree
         *
         *
         **/
        int insert(keytype k, data_type data) {
          
            leafNode* n = NULL;

            //cout << "in global insert function" << endl;
            if (root == NULL) {
                //cout << "making new root" << endl;
                makeroot(k, data);
                //ret = std::pair<iterator, bool>(iterator(static_cast<leafNode*> (root), 0), true);
                upkeycount();
                //return ret;
                return 1;
            }

            n = find(k);

            /*this means it wasn't found and there is current leaf to place the
             * key*/
            if (n == NULL) {
                n = new leafNode();
                n->initialize();
            }
            //if(ret.second)
            //  n = (ret.first).getleafNode();

            //n =(leafNode*)  (iter.getleafNode());
            int out = insertleafpair(n, k, data);
            if(out == -2)
            {
                //duplicate was found
                //return std::pair<iterator, bool>(iterator(static_cast<leafNode*> (root), false), 0);
                return out;
            }
            if (out == -1) {
                //cout << "cannot insert in initial first node, it must be full" << endl;
                //leafnode is full
                if (n->keyCount() == bt_innernodemax) {
                    //cout << "splitting node" << endl;
                    //this will lead to a split of the leaf node.
                    leafNode* ln = new leafNode;
                    leafNode* lp = new leafNode;
                    ln->initialize();
                    lp->initialize();
                    //copy half of the keys/tuple pairs to the new leaf node
                    int j = 0;
                    int slot = 0;
                    int added = -1;
                    for (int i = 0, slot = 0; i < n->keyCount(); i++, slot++) {
                        //insert first half in l, then second half in lp
                        if ((keygreater(n->keySlots[i], k) > 0) && slot == i) {
                            if (i < (n->keyCount() / 2)) {
                                insertleafKeyAt(ln, k, slot);
                                insertleafDataAt(ln, data, slot);
                            } else {
                                insertleafKeyAt(lp, k, j);
                                insertleafDataAt(lp, data, j);
                                j++;
                            }
                            added = 1;
                            //keep i in the same pos
                            if (i > 0)
                                i--;
                            //cout << " adding middle key in slot " << j << endl;
                        } else if (i < (n->keyCount() / 2)) {

                            insertleafKeyAt(ln, n->keySlots[i], slot);
                            insertleafDataAt(ln, n->dataSlots[i], slot);
                        } else {

                            insertleafKeyAt(lp, n->keySlots[i], j);
                            insertleafDataAt(lp, n->dataSlots[i], j);
                            j++;
                        }

                    }
                    if (added != 1) {
                        //cout << " adding middle key at the end " << endl;
                        insertleafKeyAt(lp, k, j);
                        insertleafDataAt(lp, data, j);
                    }
                    slot = 1;
                    keytype kp = lp->keySlots[0];

                    //cout << "split key kp is " << kp << endl;
                    upkeycount();
                     if(ln->isleaf())
                     {
                                //cout << "connecting leaves of spliting leafs" << endl;
                                   ln->nextLeaf = lp;
                                   lp->prevLeaf = ln;
                     }
                    ///innerNode* parent = static_cast<innerNode*>(n->parent);
                    //int oldloc = findInnerNodeLoc(parent, n);
                    //assert(oldloc >= 0);
                    //key is pushed up to parent.
                    insert_in_parent(n->parent, ln, kp, lp);
                    //cout << "out of insert parent" << endl;
                    freeNode(n);
                    n = NULL;
                    //return std::pair<iterator, bool> (iterator(static_cast<leafNode*> (root), 0), true);
                    return 1;
                }

            }
            upkeycount();
            //return std::pair<iterator, bool> (iterator(static_cast<leafNode*> (btree::root), 0), true);
            return 1;
        }

        void makeroot(keytype k, data_type data) {
            leafNode* l;
            //cout << "MAKEROOT:: making root" << endl;
            l = new leafNode;
            l->initialize();
            l->setIsRoot(true);
            l->parent = NULL;
            insertleafpair(l, k, data);
	    headleaf = l;
            root = static_cast<node*> (l);
        }

        /**
         *This function is used when adding a tuple that requires splitting of the node.
         *
         * */
        void insert_in_parent(node* parent, node* N, keytype k, node* Nprime) {
            innerNode* tnode;
            innerNode* p = static_cast<innerNode*> (parent);
            //initialize node
            //tnode->initialize();
            keytype kp;
            //cout << "inserting in parent" << endl;
            if ((p != NULL)) {

                //cout << "there is a  parent " << endl;
                if ((p->numChildren < bt_innernodemax+1)) {
                    //cout << "parent is not full, inserting pair " << endl;
                    //decrement number of Children because 1 is going to be overwritten
                    p->numChildren--;

                    int loc = insertinnernodepair(p, k, N);
                    if (loc >= 1) {
                        insertInnerNodeChildAt(p, Nprime, loc + 1);
                        //p->numChildren--;
                        ////cout << "replace old N with new N" << endl;

                    }

                    //cout << "loc is " << loc << endl;

                    return;
                } else {
                    //cout << "parent is full" << endl;
                    innerNode* pNode = new innerNode;
                    pNode->initialize();
                    innerNode* ppNode = new innerNode;
                    ppNode->initialize();

                    //perform splitting of the parent
                    //then push it up.
                    int j = 0;
                    int slot = 0;
                    int added = -1;
                    int i = 0;
                    //cout << "keycount of parent is " << p->keyCount() << endl;
                    for (i = 0, slot = 0; i < p->keyCount(); i++, slot++) {
                        //insert Np in the nodes
                        //try to insert the pushed up child
                        if ((keygreater(p->keySlots[i], k) > 0) && slot == i) {
                            if (i < (p->keyCount() / 2) + 1) {

                                insertInnerNodeKeyAt(pNode, k, slot);
                                insertInnerNodeChildAt(pNode, (N), slot);
                                insertInnerNodeChildAt(pNode, (Nprime), slot + 1);
                                added = slot;
                                //cout << " adding middle key in first node slot " << slot << endl;

                            } else {

                                insertInnerNodeKeyAt(pNode, k, j);
                                insertInnerNodeChildAt(pNode, (N), j);
                                insertInnerNodeChildAt(pNode, (Nprime), j + 1);
                                //cout << " adding middle key in second node slot " << j << endl;
                                j++;
                                added = j;
                            }

                            //keep i in the same pos
                            if (i > 0)
                                i--;

                        }
                        if (i < (p->keyCount() / 2 + 1) && slot == i) {
                            //cout << "inserting in first node" << endl;
                            insertInnerNodeKeyAt(pNode, p->keySlots[i], slot);
                            insertInnerNodeChildAt(pNode, static_cast<node*> (p->firstChild[i]), slot);

                        }


                        if (i > (p->keyCount() / 2)) {
                            //cout << "inserting in second node" << endl;
                            insertInnerNodeKeyAt(ppNode, p->keySlots[i], j);
                            insertInnerNodeChildAt(ppNode, static_cast<node*> (p->firstChild[i]), j);
                            //keep chain
                           
                            j++;
                        }

                        if (i == (p->keyCount() / 2 + 1)) {
                            //cout << "delete last key in first node" << endl;
                            kp = pNode->keySlots[i-1];
                            pNode->slotsinuse--;
                            if(p->firstChild[i]->isleaf())
                            {
                                leafNode* lp = static_cast<leafNode*>(p->firstChild[i-1]);
                                leafNode* ln = static_cast<leafNode*>(p->firstChild[i]);
                                lp->nextLeaf = ln;
                                ln->prevLeaf = lp;
                            }
                          
                        }

                    }


                    if (p->firstChild[i] != NULL) {
                        //insert last pointer in ppNode
                        insertInnerNodeChildAt(ppNode, static_cast<node*> (p->firstChild[i]), j);
                        //cout << "insert last parent inner node into child, slot is " << j << endl;
                        //innerNode* temp = static_cast<innerNode*> (p->firstChild[i]);
                        //cout << "Child's first key is " << temp->keySlots[0] << endl;
                        //cout << "ppNode has last key as " << ppNode->keySlots[ppNode->keyCount() - 1] << endl;
                        //j++;

                    } else {
                        //cout << "last child is null " << endl;
                    }
                    if (added < 0) {
                        //cout << "adding new key and children at the end" << endl;

                        //decrease the number of children because it is going to be overwritten
                        ppNode->numChildren--;
                        insertInnerNodeChildAt(ppNode, N, j);
                        insertInnerNodeKeyAt(ppNode, k, j);
                        //cout << "insert child and key at slot " << j << endl;
                        insertInnerNodeChildAt(ppNode, (Nprime), j + 1);
                        //cout << "adding key " << k << " at then end " << j + 1 << endl;
                        assert(Nprime != NULL);

                    }
                  
                    //insert in parent
                    k = pNode->keySlots[pNode->keyCount()];
                    //cout << "key to push up to parent is " << kp << endl;

                    insert_in_parent(p->parent, pNode, kp, ppNode);
                }

            } else {
                //cout << "making new root to insert" << endl;
                //there is no parent
                // assert(N->isRoot());
                tnode = new innerNode;
                tnode->initialize();
                tnode->setIsRoot(true);
                tnode->parent = NULL;


                //insert key and children in top node
                insertInnerNodeKeyAt(tnode, k, 0);
                insertInnerNodeChildAt(tnode, N, 0);
                insertInnerNodeChildAt(tnode, Nprime, 1);
                root = static_cast<node*> (tnode);

            }


        }

        //Node Search Operations

        /**
         *Find lower key in a node
         **/
        template <typename node_type>
        inline int find_lowerkey(node_type* l, keytype& k) {
            int i = 0;
            if ((l == NULL))
                return -1;

            if (l->slotsinuse == 0)
                return 0;
            //cout << "In leafnode find key Loc" << endl;
            for (i = 0; i < l->slotsinuse; i++) {
                if (keylessequal(k, l->keySlots[i])) {
                    //cout << "found the key" << endl;
                    return i;
                }

            }
            return -1;


        }

        /**
         * Delete key, data pair from index
         */
        int delete_pair(keytype k, data_type data) {
            leafNode* leaf = NULL;
            leaf = find(k);
            int loc = -1;

            if (leaf == NULL) {
                //cout << "key not in tree" << endl;
                return -1;
            } else {
                if ((loc = deleteleafpair(leaf, k, data)) < 0) {
                    return -1;
                }
                balancetree(static_cast<leafNode*> (leaf), loc);
                return loc;
            }
        }

        /*
         *Delete the first key that matches k
         */
        int erase(keytype k) {
            leafNode* l = NULL;
            l = find(k);

            //cout << "in erase funtion" << endl;
            if (l == NULL) {
                //cout << " key not in tree" << endl;
                return -1;
            } else {
                int loc = findKeyEqual(l, k);
		if(loc < 0)
		    return loc;
                //cout << "key is in location " << loc << endl;
                if (keyequal(k, l->keySlots[loc]) > 0) {
                    //cout << "found a key equal, now deleting it" << endl;
                    for (int i = loc; i < l->keyCount() - 1; i++) {
                        l->keySlots[i] = l->keySlots[i + 1];
                    }
                    l->slotsinuse--;
                    downkeycount();
                    if (!(l->isRoot())) {
                        //cout << "not a root, balance it" << endl;
                        balancetree(l, loc);
                    } else {
                        //cout << "leaf is root" << endl;
                    }
                    return loc;
                }
            }
                    return -1;
            }
            /**
             *erasepair - erase a key and a data pair from the tree.
             */
            int erasepair(keytype k, data_type d)  {
            leafNode* l = NULL;
            l = find(k);

            //cout << "in erase funtion" << endl;
            if (l == NULL) {
                //cout << " key not in tree" << endl;
                return -1;
            } else {
                int loc = findKeyEqual(l, k);
		if(loc < 0)
		    return loc;
                //cout << "key is in location " << loc << endl;
                if (keyequal(k, l->keySlots[loc]) > 0 && (d==l->dataSlots[l])) {
                    //cout << "found a key equal, now deleting it" << endl;
                    for (int i = loc; i < l->keyCount() - 1; i++) {
                        l->keySlots[i] = l->keySlots[i + 1];
                    }
                    l->slotsinuse--;
                    downkeycount();
                    if (!(l->isRoot())) {
                        //cout << "not a root, balance it" << endl;
                        balancetree(l, loc);
                    } else {
                        //cout << "leaf is root" << endl;
                    }
                    return loc;
                }
            }

                    return -1;
        }

        void balancetree(leafNode* leaf, int loc) {
            int parentloc = findInnerNodeLoc(static_cast<innerNode*>(leaf->parent), static_cast<node*>(leaf));
            //cout << "attemping to  balance tree, loc is " << parentloc << endl;
            //check for underflow...we need to rebalance tree
            if (leaf->isunderflow() ) {
                //cout << "the leaf underflows" << endl;
                //get parent
                // innerNode* pparent = static_cast<innerNode*>(leaf->parent);

                //check neighbors
                if ((leaf->prevLeaf != NULL) && (parentloc > 0) ) {
                     //cout << "prev leaf" << endl;
                    leafNode* prev = static_cast<leafNode*> (leaf->prevLeaf);

                    //if they can merge add them
                    if ((prev->keyCount() + leaf->keyCount()) < bt_leafnodemax) {
                        //cout << "prev leaf merging leafs" << endl;
                        int loc = findInnerNodeLoc(static_cast<innerNode*>(leaf->parent), static_cast<node*> (leaf));
                        for (int i = 0; i < leaf->keyCount(); i++) {
                            insertleafpair(prev, leaf->keySlots[i], leaf->dataSlots[i]);
                        }
                        
                        assert(loc >= 0);
                        //cout << "loc returned is " << parentloc << endl;
                        loc = parentloc;
                        innerNode* p = static_cast<innerNode*> (leaf->parent);
                        keytype k;
                        if(loc == p->numChildren)
                        {
                            loc--;

                        }
                        else
                            k = p->keySlots[loc-1];
                        //cout << "key is " << k << endl;
                        deleteInnerNode(static_cast<innerNode*> (leaf->parent), k, static_cast<node*> (leaf));
                        

                    } else //borrow key data pair
                    {
                        //cout << "borrowing from previous neighbor" << endl;
                        assert(leaf->keyCount() < prev->keyCount());
                        //move last key in prev to leaf
                        insertleafpair(leaf, prev->keySlots[prev->keyCount() - 1], prev->dataSlots[prev->keyCount() - 1]);

                        innerNode* p = static_cast<innerNode*> (leaf->parent);
                        keytype replacee = p->keySlots[parentloc-1];


                        replacekey(leaf->parent, replacee , leaf->keySlots[0]);

                        deleteleafpair(prev, prev->keySlots[prev->keyCount() - 1], prev->dataSlots[prev->keyCount() - 1]);
                        upkeycount();
                        //change K in parent to be new key
                    }

                } else if (leaf->nextLeaf != NULL) {
                     //cout << "nextleaf" << endl;
                    leafNode* next = static_cast<leafNode*> (leaf->nextLeaf);
                    if ((next->keyCount() + leaf->keyCount()) < bt_leafnodemax) //if not full move key
                    {
                        //cout << "nextleaf, merging leafs" << endl;
                        for (int i = 0; i < leaf->keyCount(); i++) {
                            //cout << "inserting key " << leaf->keySlots[i] << " to next leaf" << endl;
                            insertleafpair(next, leaf->keySlots[i], leaf->dataSlots[i]);
                        }
                        int loc = findInnerNodeLoc(static_cast<innerNode*>(leaf->parent), static_cast<node*> (leaf));
                        
                        assert(loc >= 0);
                        //cout << "inner node loc is " << loc << endl;
                        innerNode* p = static_cast<innerNode*> (leaf->parent);
                        keytype k;
                        if (loc < p->numChildren) {
                            k = p->keySlots[loc];
                        } else {
                            k = p->keySlots[loc+1];
                        }
                        
                        deleteInnerNode(static_cast<innerNode*> (leaf->parent), k, static_cast<node*> (leaf));
                        //replacekey(next->parent, next->keySlots[1], next->keySlots[0]);
                    } else //borrow key data pair
                    {
                        //cout << "borrowing for next neighbor" << endl;
                        assert(leaf->keyCount() < next->keyCount());
                        //move last key in prev to leaf
                        insertleafpair(leaf, next->keySlots[0], next->dataSlots[0]);

                        replacekey(next->parent, next->keySlots[0], next->keySlots[1]);
                        deleteleafpair(next, next->keySlots[0], next->dataSlots[0]);
                        upkeycount();

                        //change K in parent to be new key
                    }

                    //delete leaf from parent

                }

            }
        }

    private:

        int replacekey(node* n, keytype replacee, keytype replacer) {
            innerNode* inner = static_cast<innerNode*> (n);

            if (inner == NULL) {
                //cout << "replacekey:: inner is null" << endl;
                return -1;
            }

            for (int i = 0; i < inner->keyCount(); i++) {
                if (keyequal(inner->keySlots[i], replacee) > 0) {
                    //cout << "replacing key " << replacee << " with " << replacer << endl;
                    inner->keySlots[i] = replacer;
                    return 1;
                }
            }
            //cout << "did not find key to replace" << endl;
            return -1;
        }

        int deleteInnerNode(innerNode* N, keytype kP, node* P) {
            int loc = 0;
            keytype k;
            loc = findInnerNodeLoc(N, P);
            int keyloc = findInnerNodeKey(N, kP);
            //cout << "deleteInnerNode:: key to delete is in location " << keyloc << endl;
            if (loc < 0) {
                //cout << "deleteInnerNode:: cannot find inner node" << endl;
                return loc;
            }

            k = N->keySlots[keyloc];
            //cout << "key to delete is " << k << endl;
            
            //cout << "deleteInnnerNode: N has " << N->numChildren << " children" << endl;
            //this is where actuall deletion takes place
           if (loc < N->numChildren) {
               if((loc < N->numChildren)  && (loc > 0))
               {
                     if ((N->firstChild[loc])->isleaf()) {
                          leafNode* cur = static_cast<leafNode*> (N->firstChild[loc]);
                          leafNode* prev = static_cast<leafNode*> (N->firstChild[loc-1]);
                          prev->nextLeaf = cur->nextLeaf;
                          //if(cur->nextLeaf == NULL)
                          //cout << "deleteInnnerNode: next leaf is NULL " << endl;
                     }
                   
               }
                freeNode(N->firstChild[loc]);
                
                //cout << "deleteInnerNode:: deleting node at loc " << loc << endl;
                //cout << "shift remaining nodes" << endl;
                //cout << "keycount is " << N->keyCount() << endl;
                int i;
                int kloc;
                //shift remaining nodes.
                for(kloc = keyloc; kloc < N->keyCount()-1; kloc++)
                {
                    //cout << "deleteInnerNode:: shifting key to location " << kloc << endl;

                    N->keySlots[kloc] = N->keySlots[kloc + 1];

                    
                }
                for (i = loc; i < N->keyCount() - 1; i++) {
                    assert(i + 1 < N->keyCount());
                    //cout << "deleteInnerNode:: shifting to location " << i << endl;

                    N->firstChild[i] = N->firstChild[i + 1];

                    //cout << "cur loc is  " << i << endl;
                   if ((N->firstChild[i])->isleaf()) {
                        leafNode* cur = static_cast<leafNode*> (N->firstChild[i]);
                        if (i - 1 >= 0) {
                            leafNode* prev = static_cast<leafNode*> (N->firstChild[i - 1]);
                            //cout << "setting previous node" << endl;
                            cur->prevLeaf = prev;
                            prev->nextLeaf = cur;
                             //cout << "last key in prev is " << prev->keySlots[prev->keyCount()-1] << endl;
                            //cout << "first key in cur is " << cur->keySlots[0] << endl;
                            assert(prev != NULL);
                            
                        } else {
                            cur->prevLeaf = NULL;
                            //cout << "setting prev as NULL" << endl;
                        }

                    } else {
                        //cout << "N isn't leaf" << endl;
                    }

                }

                if (N->numChildren > N->keyCount()) {
                   if(loc+1 < N->numChildren)
                    {
                         //cout << "setting the last child at loc " << i << endl;
                    
                        N->firstChild[i] = N->firstChild[i + 1];
                        N->firstChild[i + 1] = NULL;

                        if ((N->firstChild[i])->isleaf()) {
                        leafNode* cur = static_cast<leafNode*> (N->firstChild[i]);
                        if (i - 1 >= 0) {
                            leafNode* prev = static_cast<leafNode*> (N->firstChild[i - 1]);
                            //cout << "setting previous node" << endl;
                            cur->prevLeaf = prev;
                            prev->nextLeaf = cur;
                             //cout << "last key in prev is " << prev->keySlots[prev->keyCount()-1] << endl;
                            //cout << "first key in cur is " << cur->keySlots[0] << endl;
                            assert(prev != NULL);

                        }

                    } else {
                        //cout << "N isn't leaf" << endl;
                    }

                   }
               
                }
               
                N->slotsinuse--;
                N->numChildren--;
                //cout << "num of children after delete is " << N->numChildren << endl;
            }

            innerNode* child = static_cast<innerNode*> (N);
            innerNode* parent = static_cast<innerNode*> (child->parent);
            innerNode* next = NULL;
            innerNode* prev = NULL;

            if (N->isRoot()) {
                //cout << "parent is root" << endl;
                if (N->numChildren == 1) {
                    //cout << "making child root" << endl;
                    innerNode* child = static_cast<innerNode*> (N->firstChild[0]);
                    child->parent = NULL;
                    child->setIsRoot(true);
                    root = static_cast<node*> (child);
                    delete N;
                    return 1;
                }
            }
            //balance nodes
            else if(child->isunderflow()) {
                //cout << "N parent is underflow" << endl;
                loc = findInnerNodeLoc(parent, child);
                
                //cout << "child loc is " << loc << endl;
                
                if (loc - 1 >= 0) {
                    prev = static_cast<innerNode*> (parent->firstChild[loc - 1]);
                }
                if (loc + 1 < parent->numChildren) {
                    next = static_cast<innerNode*> (parent->firstChild[loc + 1]);

                }
                if(loc == parent->numChildren-1)
                    loc = loc-1;
                //cout << "key is " << parent->keySlots[loc] << endl;
                if (next != NULL) {
                    if ((next->numChildren + child->numChildren) <= (bt_innernodemax+1)) {
                        //cout << "merging parent with next neighbor" << endl;
                        //cout << "neighbor has children count of " << next->numChildren << endl;
                        //cout << "neighbor has key count of " << next->keyCount() << endl;
                           //cout << "keey in loc is " << parent->keySlots[loc] << endl;
                                //cout << "keey in loc+1 is " << parent->keySlots[loc+1] << endl;
                      
                        //leafNode* nextChild = static_cast<leafNode*>(next->firstChild[0]);
                        insertInnerNodeKeyAt(next, parent->keySlots[loc], 0);
                        insertInnerNodeChildAt(next, getChild(child, child->numChildren-1), 0);
                        for (int i = child->keyCount()-1; i >= 0; i--) {
                            //cout << "inserting node and key from loc " << i << " in next node" << endl;
                            insertInnerNodeKeyAt(next, child->keySlots[i], 0);
                            insertInnerNodeChildAt(next, child->firstChild[i], 0);

                        }
                        keytype kp =  parent->keySlots[loc];
                         //cout << "no more children delete child" << endl;
                         deleteInnerNode(static_cast<innerNode*>(child->parent), kp, child);
                        
                    } else {
                        //cout << "redistribute from next" << endl;
                        //cout << "keey in loc is " << parent->keySlots[loc] << endl;
                                //cout << "keey in loc+1 is " << parent->keySlots[loc+1] << endl;
                        insertInnerNodeKeyAt(child, parent->keySlots[loc], child->keyCount());
                        insertInnerNodeChildAt(child, next->firstChild[0], child->numChildren);

                        
                        //cout << "removing redistributed child from next node" << endl;
                        replacekey(next->parent, parent->keySlots[loc], next->keySlots[0]);
                        int i = 0;
                        for (i = 0; i < next->keyCount() - 1; i++) {
                            //cout << "shifting key and child to " << i << endl;
                            next->keySlots[i] = next->keySlots[i + 1];
                            next->firstChild[i] = next->firstChild[i + 1];

                        }
                        if (next->numChildren > next->keyCount()) {
                            //cout << "shifting last child" << endl;
                            next->firstChild[i] = next->firstChild[i + 1];
                        }
                        next->slotsinuse--;
                        next->numChildren--;


                    }
                } else if (prev != NULL) {
                    //cout << "merging parent with previous neighbor" << endl;
                    if ((prev->numChildren + child->numChildren) <= (bt_innernodemax+1)) {
                        //merge together
                        insertInnerNodeKeyAt(prev, parent->keySlots[loc], prev->keyCount());
                         insertInnerNodeChildAt(prev, child->firstChild[0], prev->numChildren);
                        for (int i = 0; i < child->keyCount(); i++) {
                            insertInnerNodeKeyAt(prev, child->keySlots[i], prev->keyCount());
                            insertInnerNodeChildAt(prev, child->firstChild[i+1], prev->numChildren);

                        }
                         keytype kp =  parent->keySlots[loc];
                         //cout << "no more children delete child" << endl;
                         deleteInnerNode(static_cast<innerNode*>(child->parent), kp , child);

                    } else {
                        //cout << "redistribute from prev" << endl;
                       
                        insertInnerNodeKeyAt(child, parent->keySlots[loc], child->keyCount());
                        insertInnerNodeChildAt(child, prev->firstChild[prev->numChildren-1], 0);


                        //cout << "removing redistributed child from next node" << endl;
                        replacekey(next->parent, parent->keySlots[loc], prev->keySlots[prev->keyCount()-1]);

                       
                        //cout << "removing redistributed child from prev node" << endl;
                        prev->firstChild[prev->numChildren - 1] = NULL;
                        prev->numChildren--;
                        prev->slotsinuse--;
                    }


                }
            }



            return loc;

        }

        void upkeycount() {
            totalkeycount++;
        }

        void downkeycount() {
            totalkeycount--;
        }

        //Free nodes
        inline void freeNode(node* free)
        {
            if(free->isleaf())
            {
                delete static_cast<leafNode*>(free);
            }
            else
            {//if node is inner node
                delete static_cast<innerNode*>(free);
            }

        }
    private:
        // *** Template Magic to Convert a pair or key/data types to a value_type

        /// \internal For sets the second pair_type is an empty struct, so the
        /// value_type should only be the first.

        template <typename value_type, typename pair_type>
        struct btree_pair_to_value {
            /// Convert a fake pair type to just the first component

            inline value_type operator()(pair_type & p) const {
                return p.first;
            }
            /// Convert a fake pair type to just the first component

            inline value_type operator()(const pair_type & p) const {
                return p.first;
            }
        };

        /// \internal For maps value_type is the same as the pair_type

        template <typename value_type>
        struct btree_pair_to_value<value_type, value_type> {
            /// Identity "convert" a real pair type to just the first component

            inline value_type operator()(pair_type & p) const {
                return p;
            }
            /// Identity "convert" a real pair type to just the first component

            inline value_type operator()(const pair_type & p) const {
                return p;
            }
        };

        /// Using template specialization select the correct converter used by the
        /// iterators
        typedef btree_pair_to_value<pair_type, pair_type> pair_to_value_type;


    public:
        // *** Iterators and Reverse Iterators

        /// STL-like iterator object for B+ tree items. The iterator points to a
        /// specific slot number in a leaf.

        class iterator {
        public:
            // *** Types

            /// The key type of the btree. Returned by key().
            typedef typename btree::keytype key_type;

            /// The data type of the btree. Returned by data().
            typedef typename btree::data_type data_type;

            /// The value type of the btree. Returned by operator*().

            /// The pair type of the btree.
            typedef typename btree::pair_type pair_type;

            /// Reference to the value_type. STL required.
            typedef pair_type& reference;

            /// Pointer to the value_type. STL required.
            typedef pair_type* pointer;

            /// STL-magic iterator category
            //typedef std::bidirectional_iterator_tag iterator_category;

            /// STL-magic
            //typedef ptrdiff_t               difference_type;

            /// Our own type
            typedef iterator self;

        private:
            // *** Members

            /// The currently referenced leaf node of the tree
            typename btree::leafNode* currnode;

            /// Current key/data slot referenced
            unsigned short currslot;

            /// Friendly to the const_iterator, so it may access the two data items directly.
            friend class const_iterator;

            /// Also friendly to the reverse_iterator, so it may access the two data items directly.
            friend class reverse_iterator;

            /// Also friendly to the const_reverse_iterator, so it may access the two data items directly.
            friend class const_reverse_iterator;

            /// Evil! A temporary value_type to STL-correctly deliver operator* and
            /// operator->
            mutable pair_type temp_value;

            // The macro BTREE_FRIENDS can be used by outside class to access the B+
            // tree internals. This was added for wxBTreeDemo to be able to draw the
            // tree.

        public:
            // *** Methods

            /// Default-Constructor of a mutable iterator

            inline iterator()
            : currnode(NULL), currslot(0) {
            }

            /// Initializing-Constructor of a mutable iterator

            inline iterator(typename btree::leafNode *l, unsigned short s)
            : currnode(l), currslot(s) {
            }

            /// Copy-constructor from a mutable iterator
            // inline iterator(iterator &it)
            //: currnode(it.currnode), currslot(it.currslot){ }
            /// Copy-constructor from a reverse iterator

            inline iterator(const reverse_iterator &it)
            : currnode(it.currnode), currslot(it.currslot) {
            }

            /// Dereference the iterator, this is not a value_type& because key and
            /// value are not stored together

            inline reference operator*() const {
                temp_value = pair_to_value_type()(pair_type(currnode->keySlots[currslot],
                        currnode->dataSlots[currslot]));
                return temp_value;
            }

            /// Dereference the iterator. Do not use this if possible, use key()
            /// and data() instead. The B+ tree does not stored key and data
            /// together.

            inline pointer operator->() const {
                temp_value = pair_to_value_type()(pair_type(currnode->keySlots[currslot],
                        currnode->dataSlots[currslot]));
                return &temp_value;
            }

            /// Key of the current slot

            inline const key_type& key() const {
                return currnode->keySlots[currslot];
            }

            /// Writable reference to the current data object

            inline data_type& data() const {
                return currnode->dataSlots[currslot];
            }

            inline leafNode* getleafNode() {
                return currnode;
            }
            /// Prefix++ advance the iterator to the next slot

            inline self & operator++() {
                if (currslot + 1 < currnode->slotsinuse) {
                    ++currslot;
                } else if (currnode->nextLeaf != NULL) {
                    currnode = currnode->nextLeaf;
                    currslot = 0;
                } else {
                    // this is end()
                    currslot = currnode->slotsinuse;
                }

                return *this;
            }

            /// Postfix++ advance the iterator to the next slot

            inline self operator++(int) {
                self tmp = *this; // copy ourselves

                if (currslot + 1 < currnode->slotsinuse) {
                    ++currslot;
                } else if (currnode->nextLeaf != NULL) {
                    currnode = currnode->nextLeaf;
                    currslot = 0;
                } else {
                    // this is end()
                    currslot = currnode->slotsinuse;
                }

                return tmp;
            }

            /// Prefix-- backstep the iterator to the last slot

            inline self & operator--() {
                if (currslot > 0) {
                    --currslot;
                } else if (currnode->prevLeaf != NULL) {
                    currnode = currnode->prevLeaf;
                    currslot = currnode->slotsinuse - 1;
                } else {
                    // this is begin()
                    currslot = 0;
                }

                return *this;
            }

            /// Postfix-- backstep the iterator to the last slot

            inline self operator--(int) {
                self tmp = *this; // copy ourselves

                if (currslot > 0) {
                    --currslot;
                } else if (currnode->prevLeaf != NULL) {
                    currnode = currnode->prevLeaf;
                    currslot = currnode->slotsinuse - 1;
                } else {
                    // this is begin()
                    currslot = 0;
                }

                return tmp;
            }

            /// Equality of iterators

            inline bool operator==(const self& x) const {
                return (x.currnode == currnode) && (x.currslot == currslot);
            }

            /// Inequality of iterators

            inline bool operator!=(const self& x) const {
                return (x.currnode != currnode) || (x.currslot != currslot);
            }
        };

        /// STL-like read-only iterator object for B+ tree items. The iterator
        /// points to a specific slot number in a leaf.
    public:
        // *** STL Iterator Construction Functions

        /// Constructs a read/data-write iterator that points to the first slot in
        /// the first leaf of the B+ tree.

        inline iterator begin() {
            return iterator(headleaf, 0);
        }

        /// Constructs a read/data-write iterator that points to the first invalid
        /// slot in the last leaf of the B+ tree.

        inline iterator end() {
            return iterator(tailleaf, tailleaf ? tailleaf->slotsinuse : 0);
        }

        /// Constructs a read-only constant iterator that points to the first slot
        /// in the first leaf of the B+ tree.

        inline const_iterator begin() const {
            return const_iterator(headleaf, 0);
        }

        /// Constructs a read-only constant iterator that points to the first
        /// invalid slot in the last leaf of the B+ tree.

        inline const_iterator end() const {
            return const_iterator(tailleaf, tailleaf ? tailleaf->slotsinuse : 0);
        }

        /// Constructs a read/data-write reverse iterator that points to the first
        /// invalid slot in the last leaf of the B+ tree. Uses STL magic.

        inline reverse_iterator rbegin() {
            return reverse_iterator(end());
        }

        /// Constructs a read/data-write reverse iterator that points to the first
        /// slot in the first leaf of the B+ tree. Uses STL magic.

        inline reverse_iterator rend() {
            return reverse_iterator(begin());
        }

        /// Constructs a read-only reverse iterator that points to the first
        /// invalid slot in the last leaf of the B+ tree. Uses STL magic.

        inline const_reverse_iterator rbegin() const {
            return const_reverse_iterator(end());
        }

        /// Constructs a read-only reverse iterator that points to the first slot
        // in the first leaf of the B+ tree. Uses STL magic.

        inline const_reverse_iterator rend() const {
            return const_reverse_iterator(begin());
        }


        /// The pair type of the btree.

        /// Reference to the value_type. STL required.
        typedef pair_type& reference;

        /// Pointer to the value_type. STL required.
        typedef pair_type* pointer;

        /// STL-magic iterator category
        typedef std::bidirectional_iterator_tag iterator_category;

        /// STL-magic
        typedef ptrdiff_t difference_type;

        /// Our own type
        typedef reverse_iterator self;

    private:
        // *** Members

        /// The currently referenced leaf node of the tree
        typename btree::leafNode* currnode;

        /// One slot past the current key/data slot referenced.
        unsigned short currslot;

        /// Friendly to the const_iterator, so it may access the two data items directly
        friend class iterator;

        /// Also friendly to the const_iterator, so it may access the two data items directly
        friend class const_iterator;

        /// Also friendly to the const_iterator, so it may access the two data items directly
        friend class const_reverse_iterator;

        /// Evil! A temporary value_type to STL-correctly deliver operator* and
        /// operator->
        mutable pair_type temp_value;

    public:
        // *** Methods

        /// Default-Constructor of a reverse iterator
        //
        //        inline reverse_iterator()
        //        : currnode(NULL), currslot(0) {
        //        }
        //
        //        /// Initializing-Constructor of a mutable reverse iterator
        //
        //        inline reverse_iterator(typename btree::leafNode *l, unsigned short s)
        //        : currnode(l), currslot(s) {
        //        }
        //
        //        /// Copy-constructor from a mutable iterator
        //
        //        inline reverse_iterator(const iterator &it)
        //        : currnode(it.currnode), currslot(it.currslot) {
        //        }

        /// Dereference the iterator, this is not a value_type& because key and
        /// value are not stored together

        inline reference operator*() const {
            assert(currslot > 0);
            temp_value = pair_to_value_type()(pair_type(currnode->keySlots[currslot - 1],
                    currnode->dataSlots[currslot - 1]));
            return temp_value;
        }

        /// Dereference the iterator. Do not use this if possible, use key()
        /// and data() instead. The B+ tree does not stored key and data
        /// together.

        inline pointer operator->() const {
            assert(currslot > 0);
            temp_value = pair_to_value_type()(pair_type(currnode->keySlots[currslot - 1],
                    currnode->dataSlots[currslot - 1]));
            return &temp_value;
        }

        /// Key of the current slot

        inline const keytype& key() const {
            assert(currslot > 0);
            return currnode->keySlots[currslot - 1];
        }

        /// Writable reference to the current data object

        inline data_type& data() const {
            assert(currslot > 0);
            return currnode->dataSlots[currslot - 1];
        }

        /// Prefix++ advance the iterator to the next slot

        inline self & operator++() {
            if (currslot > 1) {
                --currslot;
            } else if (currnode->prevLeaf != NULL) {
                currnode = currnode->prevLeaf;
                currslot = currnode->slotsinuse;
            } else {
                // this is begin() == rend()
                currslot = 0;
            }

            return *this;
        }

        /// Postfix++ advance the iterator to the next slot

        inline self operator++(int) {
            self tmp = *this; // copy ourselves

            if (currslot > 1) {
                --currslot;
            } else if (currnode->prevLeaf != NULL) {
                currnode = currnode->prevLeaf;
                currslot = currnode->slotsinuse;
            } else {
                // this is begin() == rend()
                currslot = 0;
            }

            return tmp;
        }

        /// Prefix-- backstep the iterator to the last slot

        inline self & operator--() {
            if (currslot < currnode->slotsinuse) {
                ++currslot;
            } else if (currnode->nextLeaf != NULL) {
                currnode = currnode->nextLeaf;
                currslot = 1;
            } else {
                // this is end() == rbegin()
                currslot = currnode->slotsinuse;
            }

            return *this;
        }

        /// Postfix-- backstep the iterator to the last slot

        inline self operator--(int) {
            self tmp = *this; // copy ourselves

            if (currslot < currnode->slotsinuse) {
                ++currslot;
            } else if (currnode->nextLeaf != NULL) {
                currnode = currnode->nextLeaf;
                currslot = 1;
            } else {
                // this is end() == rbegin()
                currslot = currnode->slotsinuse;
            }

            return tmp;
        }

        /// Equality of iterators

        inline bool operator==(const self& x) const {
            return (x.currnode == currnode) && (x.currslot == currslot);
        }

        /// Inequality of iterators

        inline bool operator!=(const self& x) const {
            return (x.currnode != currnode) || (x.currslot != currslot);
        }

    private:
        // *** Convenient Key Comparison Functions Generated From keyless

        /// True if a <= b ? constructed from keyless()

        inline bool keylessequal(const keytype &a, const keytype b) const {
            return !keyless(b, a);
        }

        /// True if a > b ? constructed from keyless()

        inline bool keygreater(const keytype &a, const keytype &b) const {
            return keyless(b, a);
        }

        /// True if a >= b ? constructed from keyless()

        inline bool keygreaterequal(const keytype &a, const keytype b) const {
            return !keyless(a, b);
        }

        /// True if a == b ? constructed from keyless(). This requires the <
        /// relation to be a total order, otherwise the B+ tree cannot be sorted.

        inline bool keyequal(const keytype &a, const keytype &b) const {
            return !keyless(a, b) && !keyless(b, a);
        }

        struct btree_stats {
            /// Number of items in the B+ tree
            int datacount;

            /// Number of leaves in the B+ tree
            int leaves;

            /// Number of inner nodes in the B+ tree
            int innernodes;

            static const unsigned short leafslots = bt_leafnodemax;

            static const unsigned short innerslots = bt_innernodemax;

            /// initiaze stats

            inline btree_stats() {
                datacount = 0;
                leaves = 0;
                innernodes = 0;
            }


            /// Return the all the nondes

            inline int nodes() const {
                return innernodes + leaves;
            }

        };
    };
};
#endif
