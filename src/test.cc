#include <iostream>
#include <string>
#include <stdlib.h>
#include "btree.h"

using namespace nwt;
using namespace std;
int main() {

   
    typedef btree<int, string, 4,4,std::less<int> > bt_tree;
    bt_tree* mytree = new bt_tree();
    int i = 0;
    
    while (i < 2000 )
    {
        string value("we");
        mytree->insert(i,  value );

        i++;
         //mytree->printleaves();
    }
    for(int j = 1999; j >= 5; j--)
    {
        if(mytree->erase(j) < 0)
            cout << "deletion failed" << endl;
    }
       string valuetwo("we");
    i = 0;
     while (i < 1000 )
    {

        if(mytree->insert(i, valuetwo) == -2)
            cout << "insert failed, duplicate key found" << endl;
        i++;
     }
 mytree->printleaves();

     std::pair<string, bool> ret;
     for(int j=0; j < 100; j++)
     {
         cout << "get data for key " << j << endl;
            ret = mytree->get(j);

            if(ret.second)
            {
                cout << "key is " << ret.first << endl;
            }
            else
            {
                cout << "not found" << endl;
            }
     }
    delete mytree;
    return 0;
}
