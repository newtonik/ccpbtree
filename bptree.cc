#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <stdarg.h>
#include <unistd.h>
#include <pthread.h>
#include <stx/btree_multimap.h>
#include <iostream>
#include "bptree.h"

using namespace std;

FILE *stderrfile;
pthread_mutex_t ILINK_LOCK = PTHREAD_MUTEX_INITIALIZER;
//const char NULL_PAYLOAD[MAX_PAYLOAD_LEN + 1];



//list of index DBs for  each data type
DBIntLink  *dbintLookup;
DBLIntLink *dblinitLookup;
DBStringLink *dbstringLookup;

template <typename _keytype>
ErrCode mycreate(char* name, KeyType ktype)
{
    int ret = 0;
    typedef _keytype  _ktype;
    typedef stx::btree_multimap<_ktype, std::string, std::less<int>, btree_traits_debug<16> > stxbtree_type;
    //lock the dblink
    if((ret = pthread_mutex_lock(&ILINK_LOCK)) != 0) {
	cout << "Cannot" << endl;
    }

    //get link
    switch (ktype)
    {
	case SHORT:
	    DBIntLink  *link = dbintLookup;
    while (link != NULL) {
	if(strcmp(name, link->name) == 0) {
	    break;
	} else {
	    link = link->link;
	}

    }
    if(link != NULL) {
	pthread_mutex_unlock(&ILINK_LOCK);
	return DB_EXISTS;
    }
	break;
	case INT:
	    DBLIntLink *llink = dblinitLookup;
    while (llink != NULL) {
	if(strcmp(name, llink->name) == 0) {
	    break;
	} else {
	    llink = llink->link;
	}

    }
    if(link != NULL) {
	pthread_mutex_unlock(&ILINK_LOCK);
	return DB_EXISTS;
    }
	break;
	case VARCHAR:
	  DBStringLink *slink = dbstringLookup;
    while (slink != NULL) {
	if(strcmp(name, slink->name) == 0) {
	    break;
	} else {
	    slink = link->link;
	}

    }
    if(link != NULL) {
	pthread_mutex_unlock(&ILINK_LOCK);
	return DB_EXISTS;
    }
	  break;
	default:
	return FAILURE;
    }

    if(link != NULL) {
	pthread_mutex_unlock(&ILINK_LOCK);
	return DB_EXISTS;
    }

    //create a file to store error message for database
    //(if doesn't already exist)
    if (stderrfile == NULL) {
        char errFileName[] = "error.log";
        stderrfile = fopen(errFileName, "w");
        if (stderrfile == NULL) {
            pthread_mutex_unlock(&ILINK_LOCK);
            return FAILURE;
        }
    }
    
    //if there is no environment, make one
   /* if (env == NULL) {
        ret = p_createEnv();
        if (ret != SUCCESS) {
            return ret;
        }
    }*/
    stxbtree_type* dbp = new stxbtree_type;
    if(dbp == NULL )
    {
	return FAILURE;

    }
    
    //set the error file for the DB
    //dbp->set_errfile(dbp, stderrfile);
    
    //store the DB info in our db lookup list
    
    //make a new link object
    DBLink  *newLink = new DBLink;
    memset(newLink, 0, sizeof(DBLink<stxbtree_type>));
    
    //populate it
    newLink->name = name;
    newLink->stxdb = dbp;
    newLink->type =  SHORT;
    newLink->link = NULL;
    //Consider adding errors to log file
    
    if (dbLookup == NULL) {
        dbLookup = newLink;
    } else {
        DBLink<stxbtree_type> *thisLink = dbLookup;
        while (thisLink->link != NULL) {
            thisLink = thisLink->link;
        }
        thisLink->link = newLink;
    }
    

    pthread_mutex_unlock(&ILINK_LOCK);
    return SUCCESS;
}

ErrCode create(KeyType ktype, char* name)
{   
    ErrCode ret;
    switch (ktype)
    {
	case SHORT:
	  ret =  mycreate<int32_t>(name, ktype);
	break;
	case INT:
	    ret = mycreate<int64_t>(name, ktype);
	break;
	case VARCHAR:
	    ret = mycreate<std::string>(name, ktype);
	break;
	default:
	return FAILURE;
    }

    return  ret;
}
/*
ErrCode openIndex(const char *name, IdxState **idxState)
{
    int ret;
    btree8int_type *dbp;
    //lock the dblink system
    if ((ret = pthread_mutex_lock(&ILINK_LOCK)) != 0) {
        printf("can't acquire mutex lock: %d\n", ret);
    }
    
    //look up the DBLink for the index of that name
    DBLink *link = dbLookup;
    while (link != NULL) {
        if (strcmp(name, link->name) == 0) {
            break;
        } else {
            link = link->link;
        }
    }
    
    //if no link was found, index was never create()d
    if (link == NULL) {
        pthread_mutex_unlock(&ILINK_LOCK);
        return DB_DNE;
    }
    
    //add this thread to the link's thread counter
    //link->numOpenThreads++;
    
    //if numOpenThreads == 1, we need to open the index
    if (link->numOpenThreads == 1) {
        dbp = link->dbp;
        
        //set the db to handle duplicates (flag must be set before db is opened)
        //ret = dbp->set_flags(dbp, DB_DUPSORT); 
	//multimap
        
       
    //create a BDBState variable for this thread
    BDBState *state = malloc(sizeof(BDBState));
    memset(state, 0, sizeof(BDBState));
    *idxState = (IdxState *) state;
    state->dbp = link->dbp;
    state->type = link->type;
    state->db_name = name;
    
    //unlock the dblink system
    pthread_mutex_unlock(&ILINK_LOCK);
    
    return SUCCESS;
}
*/

/**
 *Not dealing with transcations yet
 */
ErrCode beginTransaction(TxnState **txn)
{
    return SUCCESS;
}

/***
 * Not dealing with transactions yet
 */
ErrCode abortTransaction(TxnState **txn)
{

  return SUCCESS;
}

/***
 * Not dealing with transactions yet
 */
ErrCode commitTransaction(TxnState **txn)
{

    return SUCCESS;
}
/**
 * Get the first record, I am going to ignore the transaction values
 * since we only have one index
 **/
/*ErrCode get(IdxState *idxState, TxnState *txt, Record *record)
{   
    //save the key
    //Key lastKey = new Key;
    //memcpy(&lastKey, &(record->key), sizeof(key));

    Key lastKey  = record->key;
    //string data =(char*)record->payload;
    
    btinter bit;
    DBLink *link =  dbLookup;
    btree8int_type* db;
    bit = db->find(5);
    if(bit == db->end()){	
	return KEY_NOTFOUND;
    }else
    {
	//copy data to payload
	strcpy(record->payload, bit->second.c_str());
	return SUCCESS;
     }
}

*/
/**
 Retrieve the record following the previous record retrieved by get or
 getNext. If no such call has occurred since the current transaction
 began, or if this is called from outside of a transaction, this
 returns the first record in the index. Records are ordered in ascending
 order by key.  Records with the same key but different payloads
 may be returned in any order. 

 If get returned KEY_NOT_FOUND for a key k, invoking getNext will
 return the first key after k.
 
 If the index is closed and reopened, or a new transaction has begun 
 since any previous call of get or getNext, getNext returns the first 
 record in the index.
 

 @param idxState The state variable for the index whose next Record 
 is to be returned
 @param txn The transaction state to be used (or NULL if not in a transaction)
 @param record Record through which the next key/payload pair is returned
 @return ErrCode
 SUCCESS if successfully retrieved and returned the next record in the DB.
 DB_END if reached the end of the DB.
 DEADLOCK if this call could not complete because of deadlock.
 FAILURE if could not retrieve next record for some other reason.
 */
ErrCode getNext(IdxState *idxState, TxnState *txn, Record *record);

/**
 Insert a payload associated with the given key. An identical key can
 be used multiple times, but only with unique payloads.  If this is
 called from outside of a transaction, it should commit immediately.
 Records in an index are ordered in ascending order by key.  Records
 with the same key may be stored in any order.
 
 The implementation is responsible for making a copy of payload
 (e.g., it may not assume that the payload pointer continues
 to be valid after this routine returns.)

 @param idxState The state variable for this thread
 @param txn The transaction state to be used (or NULL if not in a transaction)
 @param k key value for insert
 @param payload Pointer to the beginning of the payload string
 @return ErrCode
 SUCCESS if successfully inserted record into DB.
 ENTRY_EXISTS if identical record already exists in DB.
 DEADLOCK if this call could not complete because of deadlock.
 FAILURE if could not insert entry for some other reason.
 */
/*ErrCode insertRecord(IdxState *idxState, TxnState *txn, Key *k, const char* payload)
{

	//tree allows duplicates 
	DBLink *link =  dbLookup;
	btree8int_type* db;
	string value = payload;
	db->insert2(k->keyval.shortkey, value);
	
	return SUCCESS;
}
*/

/**
 Remove the record associated with the given key from the index
 structure.  If a payload is specified in the Record, then the
 key/payload pair specified is removed. Otherwise, the payload pointer
 is a length 0 string and all records with the given key are removed from the
 database.  If this is called from outside of a transaction, it should
 commit immediately.

 @param idxState The state variable for this thread
 @param txn The transaction state to be used (or NULL if not in a transaction)
 @param record Record struct containing a Key and a char* payload 
 (or NULL pointer) describing what is to be deleted
 @return ErrCode
 SUCCESS if successfully deleted record from DB.
 ENTRY_DNE if the specified key/payload pair could not be found in the DB.
 KEY_NOTFOUND if the specified key could not be found in the DB, with only the key specified.
 DEADLOCK if this call could not complete because of deadlock.
 FAILURE if could not delete record for some other reason.
 */
ErrCode deleteRecord(IdxState *idxState, TxnState *txn, Record *record);
