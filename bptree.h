#include <stdio.h>
#include <pthread.h>
#include <stx/btree_multiset.h>
//sigmod server file
#include "server.h"

#define ENV_DIRECTORY "ENV"
#define DEFAULT_HOMEDIR "./"





KeyType type = INT;



template <int Slots>
struct btree_traits_debug
{
        static const bool       selfverify = true;
        static const bool       debug = true;
        static const int        leafslots = Slots;
        static const int        innerslots = Slots;
};


typedef stx::btree_multimap<int32_t, std::string, std::less<int>, btree_traits_debug<16> > btreeshortint_type;
typedef stx::btree_multimap<int64_t, std::string, std::less<int>, btree_traits_debug<16> > btreeint_type;
typedef stx::btree_multimap<std::string, std::string, std::less<int>, btree_traits_debug<16> > btreechar_type;
typedef btreeint_type::const_iterator btinter;

template <typename dbtype>
struct STXDBState
{
    dbtype *dbp; //this will be the struct type stx btree
    KeyType type;
    const char db_name;
    uint32_t tid;
    btinter lasKey;
    int keyNotFound;
};


template <typename dbtype>
struct CursorLink
{   
 //   DBC *cursor;
    dbtype  *dbp;
    struct CursorLink *cursorLink;
};


//typedef int bool;

//An STX Index in a linked list
typedef struct DBIntLink
{
    char    *name;
    btreeshortint_type   *stxdb;
    KeyType type;
    struct DBIntLink *link;
} DBIntLink;


typedef struct DBLIntLink
{
    char    *name;
    btreeint_type  *stxdb;
    KeyType type;
    struct DBLIntLink *link;
} DBLIntLink;



typedef struct DBStringLink
{
    char    *name;
    btreechar_type   *stxdb;
    KeyType type;
    struct DBStringLink *link;
} DBStringLink;


