/*
 * STX Btree versus MultiSet Btreee
 */

#include <string>
#include <string.h>
#include <stdlib.h>
#include <sys/time.h>

#include <fstream>
#include <iostream>
#include <iomanip>

#include <set>
#include <ext/hash_set>
#include <stx/btree_multiset.h>
#include <typeinfo>
#include <assert.h>

const char* const INT = "int";
const char* const FLOAT = "float";
const char* const INT64 = "int64_t";
const char* const STRING = "string";
const char* const STL = "stl";
const char* const STX = "stx";

// *** Settings

/// starting number of items to insert
const unsigned int mininsertnum = 1024000*32;

/// maximum number of items to insert
const unsigned int maxinsertnum = 1024000*32;

const int randseed = 34234235;

/// b+ tree slot range to test, choose 36 because it's found to be optimal
const int min_nodeslots = 36;
const int max_nodeslots = 36;

/// Time is measured using gettimeofday()
inline double timestamp()
{
    struct timeval tv;
    gettimeofday(&tv, NULL);
    return tv.tv_sec + tv.tv_usec * 0.000001;
}

using std::string;
/**
**function to create random string
**
**/
std::string randString(int seed)
{
   int len, num ;
    //using std::string;
    std::string randstring = ""; 
    srand(seed);
    len = rand()%32;
    if(len < 16) 
	len += 16;
    for(int i = 0; i < len; i++)
    {
	len = rand()%122;
	//makes the string in the regular character set.
	if(len < 32)
	    len += 32;

	char cCh = char(num);	
	randstring+=cCh;
    }
   
    return randstring;
}

/// Traits used for the speed tests, BTREE_DEBUG is not defined.
template <int _innerslots, int _leafslots>
struct btree_traits_speed
{
    static const bool   selfverify = false;
    static const bool   debug = false;

    static const int    leafslots = _innerslots;
    static const int    innerslots = _leafslots;
};

/// Test the B+ tree with a specific leaf/inner slots (only insert)
//INSERT X
template <typename type, int _slots>
struct Test_Btree_Insert
{
    typedef stx::btree_multiset<type, std::less<type>,
            struct btree_traits_speed<_slots, _slots> > btree_type;

    Test_Btree_Insert(unsigned int)
    {
    }

    void run(unsigned int insertnum)
    {
   btree_type bt;
   
   srand(randseed);
  
   if(strcmp(typeid(type).name(), "a") == 0)
   {
      for(unsigned int i = 0; i < insertnum; i++)
	  bt.insert((type)rand());

   }
   else
   {
      for(unsigned int i = 0; i < insertnum; i++)
	  bt.insert((type)rand() );

    }

      assert( bt.size() == insertnum );
    }
};
//INSERT X AND FIND Y
template <typename type, int Slots>
struct Test_Btree_Find
{
    typedef stx::btree_multiset<type, std::less<type>,
				struct btree_traits_speed<Slots, Slots> > btree_type;

    btree_type bt;

    Test_Btree_Find(unsigned int insertnum)
    {
	srand(randseed);
	for(unsigned int i = 0; i < insertnum; i++)
	    bt.insert((type)rand());

	assert( bt.size() == insertnum );
    }

    void run(unsigned int insertnum)
    {
	srand(randseed);
	//find half of what inserted
	for(unsigned int i = 0; i < insertnum/2; i++)
	    bt.exists(rand());
    }
};
unsigned int repeatuntil;
/// Test the B+ tree with a specific leaf/inner slots (insert, find and delete)
//INSERT X, DELETE Y
template <typename type, int Slots>
struct Test_Btree_InsertDelete
{
    typedef stx::btree_multiset<type, std::less<type>,
				struct btree_traits_speed<Slots, Slots> > btree_type;

    Test_Btree_InsertDelete(unsigned int)
    {
    }

    void run(unsigned int insertnum)
    {
	btree_type bt;

	srand(randseed);
	for(unsigned int i = 0; i < insertnum; i++)
	    bt.insert((type)rand());

	assert( bt.size() == insertnum );
	srand(randseed);
	for(unsigned int i = 0; i < insertnum*0.25; i++)
	    bt.erase_one((type)rand());

	//assert(bt.empty());
    }
};


/// Test the B+ tree with a specific leaf/inner slots (insert, find, delete)
//INSERT X, FIND Y, DELETE Z 
template <typename type, int Slots>
struct Test_Btree_InsertFindDelete
{
    typedef stx::btree_multiset<type, std::less<type>,
				struct btree_traits_speed<Slots, Slots> > btree_type;

    Test_Btree_InsertFindDelete(unsigned int)
    {
    }

    void run(unsigned int insertnum)
    {
	btree_type bt;

	srand(randseed);
	for(unsigned int i = 0; i < insertnum; i++)
	    bt.insert((type)rand());

	assert( bt.size() == insertnum );

	srand(randseed);
	for(unsigned int i = 0; i < insertnum*0.5; i++)
	    bt.exists((type)rand());

	srand(randseed);
	for(unsigned int i = 0; i < insertnum*0.25; i++)
	    bt.erase_one((type)rand());

	//assert(bt.empty());
    }
};


/// Repeat (short) tests until enough time elapsed and divide by the runs.
template <typename TestClass>
void testrunner_loop(std::ostream& os, unsigned int insertnum)
{
    unsigned int runs = 0;
    double ts1, ts2;
    
    do
    {
   runs = 0;

   {
       TestClass test(insertnum);   // initialize test structures

       ts1 = timestamp();
	  test.run(insertnum);
       ts2 = timestamp();
   }

   //if ((ts2 - ts1) < 1.0) repeatuntil *= 2;
    }
    while ((ts2 - ts1) < .000003);

    os << std::fixed << std::setprecision(10) << ((ts2 - ts1) ) << " " << std::flush;
}

// Template magic to emulate a for_each slots. These templates will roll-out
// btree instantiations for each of the Low-High leaf/inner slot numbers.
template< template<typename type, int Slots> class functional, typename type, int Low, int High>
struct btree_range
{
    inline void operator()(std::ostream& os, unsigned int insertnum)
    {
	testrunner_loop< functional<type, Low> >(os, insertnum);
        btree_range<functional, type, Low+4, High>()(os, insertnum);
    }
};
template< template<typename type, int Slots> class functional, typename type, int Low>
struct btree_range<functional,type, Low, Low>
{
    inline void operator()(std::ostream& os, unsigned int insertnum)
    {
      testrunner_loop< functional<type, Low> >(os, insertnum);
    }
};

/// Speed test them!
int main(int argc, char* argv[]) {
   using namespace std;
   repeatuntil = mininsertnum;
    int i = 1; 
    std::ofstream os("speedtestwithint.txt");
    if(argc < 2)
    {
	cout << "Enter a test type, e.g insert, insertfind, insertdelete, insertfinddelete" << "\n";
    }
    string cmdline = argv[i];
	//need to clean this up, probably use a map and switch statement.
	//
	//INSERT X
	if(cmdline.compare("insert") == 0)
	{
	    cout << "Testing the btree with int, insert only" << endl;
	    for(unsigned int insertnum = mininsertnum; insertnum <= maxinsertnum; insertnum *= 2) {
		std::cerr << "Insert " << insertnum << "\n";
		os << insertnum << " " << std::flush;
		btree_range<Test_Btree_Insert, unsigned int64_t, min_nodeslots, max_nodeslots>()(os, insertnum);
		os << "\n" << std::flush;
	    }
	
	}
	if(cmdline.compare("insertfind") == 0)
	{
	
	    
	    //INSERT X, FIND Y
	    for(unsigned int insertnum = mininsertnum; insertnum <= maxinsertnum; insertnum *= 2)
	    {
		std::cerr << "Insert, Find" << insertnum << "\n";
		os << insertnum << " " << std::flush;
		btree_range<Test_Btree_Find, unsigned int64_t, min_nodeslots, max_nodeslots>()(os, insertnum);
	    }
	}

	//INSERT DELETE Y
	if(cmdline.compare("insertdelete") == 0)
	{
	    for(unsigned int insertnum = mininsertnum; insertnum <= maxinsertnum; insertnum *= 2)
	    {
		std::cerr << "Insert, Delete" << insertnum << "\n";
		os << insertnum << " " << std::flush;
		btree_range<Test_Btree_InsertDelete, unsigned int64_t, min_nodeslots, max_nodeslots>()(os, insertnum);
	    }
	}
	//INSERT X FIND Y DELETE Z
	if(cmdline.compare("insertfinddelete") == 0)
	{

	    
	    for(unsigned int insertnum = mininsertnum; insertnum <= maxinsertnum; insertnum *= 2)
	    {
		std::cerr << "Insert, Delete" << insertnum << "\n";
		os << insertnum << " " << std::flush;
		btree_range<Test_Btree_InsertFindDelete, unsigned int64_t, min_nodeslots, max_nodeslots>()(os, insertnum);
	    }

	}





    return 0;
}
