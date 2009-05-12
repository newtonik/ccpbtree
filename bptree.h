#include <stdio.h>
#include <pthread.h>
#include <stx/btree_multimap.h>
//sigmod server file
#include "server.h"
#include "src/btree.h"
#define ENV_DIRECTORY "ENV"
#define DEFAULT_HOMEDIR "./"

using namespace std;

template <int Slots>
struct btree_traits_debug
{
        static const bool       selfverify = true;
        static const bool       debug = true;
        static const int        leafslots = Slots;
        static const int        innerslots = Slots;     
};


struct keyless 
{
    bool operator()(const Key& s1, const Key& s2) const
    {
	if(s1.type ==  INT)
	    return s1.keyval.intkey < s2.keyval.intkey;
	if(s2.type == SHORT)
	    return s1.keyval.shortkey < s2.keyval.shortkey;
	if(s2.type == VARCHAR)
	    return strcmp(s1.keyval.charkey, s2.keyval.charkey) < 0;
	else
	    return false;
    }
};



typedef nwt::btree<int, string, 4,4,std::less<int> > nbtree;
typedef stx::btree_multimap<Key, std::string, keyless, btree_traits_debug<16> > stxbtree_type;
typedef nwt::btree<short, string, 4,4,std::less<short> > nbtree_st;
typedef nwt::btree<string, string, 4,4,std::less<string> > nbtree_ch;
typedef nwt::btree<int, string, 4,4,std::less<int> > nbtree_int;
typedef stxbtree_type::iterator btinter;

struct STXDBState
{
    stxbtree_type *dbp; //this will be the struct type stx btree
    void *nbt;
    KeyType type;
    const char* db_name;
    uint32_t tid;
    Key lastKey;
    int keyNotFound;
    int inUse;
};

struct CursorLink
{   
 //   DBC *cursor;
    struct CursorLink *cursorLink;
};
typedef struct
{
	//stxbtree_type  *dbp;
	void* nbt;
        CursorLink  *cursorLink;
	int status;
	uint32_t tid;
} TXNState;

//typedef int bool;

struct DBLink
{
    char    *name;
    void* nbt;
    stxbtree_type   *dbp;
    KeyType type;
    struct DBLink *link;
    int numOpenThreads;
    int inUse;
};
