#include <iostream>
#include <string>
#include <stdlib.h>
#include "btree.h"

using namespace nwt;
using namespace std;
int main() {

    int all = 0;
    typedef btree<int, int, 4, 4,std::less<int> > bt_tree;
    bt_tree* mytree = new bt_tree();
 
    
    while (all == 0)
    {
    
	mytree->insert(1, 2);
	assert(mytree->size() == 1);
	mytree->insert(2, 3);
	assert(mytree->size() == 2);
	mytree->insert(3, 3);
	assert(mytree->size() == 3);
	mytree->insert(4, 12);
	assert(mytree->size() == 4);
	cout << "Main: insert #5" << endl;
	mytree->insert(5, 123);
	cout << "Main: insert complete with size as "<< mytree->size() << endl;
	assert(mytree->size() == 5);
	mytree->insert(40, 23334);
	cout << "Main: insert complete with size as "<< mytree->size() << endl;
	mytree->insert(12, 343);
	cout << "Main: insert complete with size as "<< mytree->size() << endl;
        mytree->insert(9, 2);
	assert(mytree->size() == 8);
        mytree->insert(49, 23334);
        mytree->insert(43, 233334);
        cout << "size is " << mytree->size() << endl;
        mytree->insert(44, 233344);
        cout << "size is " << mytree->size() << endl;
        mytree->insert(42, 233314);
        cout << "size is " << mytree->size() << endl;
         mytree->insert(46, 233314);
         cout << "size is " << mytree->size() << endl;
          mytree->insert(22, 233314);
          cout << "size is " << mytree->size() << endl;
           mytree->insert(24, 233314);
           cout << "size is " << mytree->size() << endl;
       mytree->insert(30, 23334);
                cout << "size is " << mytree->size() << endl;
        mytree->insert(23, 233334);
        cout << "size is " << mytree->size() << endl;
        mytree->insert(54, 233344);
        cout << "size is " << mytree->size() << endl;
        mytree->insert(92, 233314);
        cout << "size is " << mytree->size() << endl;
         mytree->insert(76, 233314);
         cout << "size is " << mytree->size() << endl;
          mytree->insert(52, 233314);
          cout << "size is " << mytree->size() << endl;
           mytree->insert(14, 233314);
           cout << "size is " << mytree->size() << endl;
	all = 1;
        
    }
    delete mytree;
    return 0;
}
