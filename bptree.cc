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



//list of index for  each data type
DBLink  *dbLookup;

/**
 *get key, from the bdbimpl *
 * */
int p_setKeyDataFromKey(Key *k, void *key)
{

    switch (k->type) {
        case SHORT:
        {
            //the key is a 32-bit integer
            //hack: put the converted int after the plain int
            uint8_t *data = ((uint8_t *) k->keyval.charkey) + 4;
            uint32_t i = k->keyval.shortkey;
            data[3] = (i & 0xFF);
            data[2] = (i & 0xFF00) >> 8;
            data[1] = (i & 0xFF0000) >> 16;
            data[0] = (i & 0xFF000000) >> 24;
            data[0] ^= 0x80;
	    key = new short;
            key = data;
	    return 1;
            break;  
        }
        case INT:
        {
            //the key is a 64-bit integer
            //hack: put the converted int after the plain int
            uint8_t *data = ((uint8_t *) k->keyval.charkey) + 8;
            uint64_t i = k->keyval.intkey;
            data[7] = (i & 0xFFLL);
            data[6] = (i & 0xFF00LL) >> 8;
            data[5] = (i & 0xFF0000LL) >> 16;
            data[4] = (i & 0xFF000000LL) >> 24;
            data[3] = (i & 0xFF00000000LL) >> 32;
            data[2] = (i & 0xFF0000000000LL) >> 40;
            data[1] = (i & 0xFF000000000000LL) >> 48;
            data[0] = (i & 0xFF00000000000000LL) >> 56;
            data[0] ^= 0x80;
            key = new int;
	    key = data;
	    return 2;
            break;
        }
        case VARCHAR:
            //the key is a <128-byte string
	    key = new char[strlen(k->keyval.charkey)+1];
	    strcpy((char*)key, k->keyval.charkey);
            //key->data = k->keyval.charkey;
            //key->size = strlen(k->keyval.charkey);
            //key->ulen = key->size;
	    return 3;
            break;
        default:
            return -1;
    }
    return 0;
}


ErrCode create( KeyType type, char* name)
{
    int ret = 0;
   
    //lock the dblink
    if((ret = pthread_mutex_lock(&ILINK_LOCK)) != 0) {
	cout << "Cannot" << endl;
    }

    DBLink *link  = dbLookup;

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
    //stxbtree_type* dbp = new stxbtree_type;
    void* nbt = NULL;
    switch(type)
    {
	case SHORT:
	 nbt  =(nbtree_st*) new nbtree_st;
	break;
	case INT:
	nbt  = (nbtree_int*) new nbtree_int;
	break;
	case VARCHAR:
	 nbt  = (nbtree_ch*) new nbtree_ch;
	break;
	default:
	return FAILURE;
    }
    //nbtree* nbt = new nbtree;
    if(nbt == NULL)
    {
	return FAILURE;

    }
    
    //set the error file for the DB
    //dbp->set_errfile(dbp, stderrfile);
    
    //store the DB info in our db lookup list
    
    //make a new link object
    DBLink  *newLink = new DBLink;
    memset(newLink, 0, sizeof(DBLink));
    
    //populate it
    newLink->name = name;
    newLink->nbt = nbt;
    newLink->type =  type;
    newLink->numOpenThreads = 0;
    newLink->link = NULL;
    newLink->inUse = 0;
    
    //Consider adding errors to log file
    
    if (dbLookup == NULL) {
        dbLookup = newLink;
    } else {
        DBLink *thisLink = dbLookup;
        while (thisLink->link != NULL) {
            thisLink = thisLink->link;
        }
        thisLink->link = newLink;
    }
    

    pthread_mutex_unlock(&ILINK_LOCK);
    return SUCCESS;
}


ErrCode openIndex(const char *name, IdxState **idxState)
{
    int ret;
    //stxbtree_type *dbp;
    void* nbt;
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
    link->numOpenThreads++;
    
    //if numOpenThreads == 1, we need to open the index
    if (link->numOpenThreads == 1) {
        nbt = link->nbt;
        
        //set the db to handle duplicates (flag must be set before db is opened)
        //ret = dbp->set_flags(dbp, DB_DUPSORT); 
	//multimap
        
       
    //create a BDBState variable for this thread
	STXDBState *state =(STXDBState*)  new STXDBState;
	memset(state, 0, sizeof(STXDBState));
	*idxState = (IdxState *) state;
	state->nbt = link->nbt;
	state->type = link->type;
	state->db_name = name;
    
    //unlock the dblink system
	pthread_mutex_unlock(&ILINK_LOCK);
    
	return SUCCESS;
    }
    pthread_mutex_unlock(&ILINK_LOCK);
    return FAILURE;
}



ErrCode closeIndex(IdxState *ident)
{
    int ret;
    STXDBState *state = (STXDBState*)ident;
    //stxbtree_type *dbp = state->dbp;
    
    //lock the dblink system
    if ((ret = pthread_mutex_lock(&ILINK_LOCK)) != 0) {
        printf("can't acquire mutex lock: %d\n", ret);
    }
    
    //check to see if the DB currently exists
    DBLink *prevLink = NULL;
    DBLink *link = dbLookup;
    while (link != NULL) {
        if (strcmp(state->db_name, link->name) == 0) {
            break;
        } else {
            prevLink = link;
            link = link->link;
        }
    }
    
    //if the DB isn't in our linked list, it never existed
    if (link == NULL) {
        fprintf(stderrfile, "closeIndex called on an index that does not exist\n");
        pthread_mutex_unlock(&ILINK_LOCK);
        return DB_DNE;
    }
    
    //decrement the number of threads for whom this index is open
    link->numOpenThreads--;
    //remove this DBP from this thread's state
    state->nbt = NULL;
    
    // if there are still threads using this index, don't close it
    if (link->numOpenThreads > 0) {
        pthread_mutex_unlock(&ILINK_LOCK);
        return SUCCESS;
    }
    /*
    printf("closing index %s====================\n",state->db_name); 
    if ((ret = dbp->close(dbp, 0)) != 0) {
        fprintf(stderrfile, "could not close index. errno %d\n", ret);
        pthread_mutex_unlock(&DBLINK_LOCK);
        return FAILURE;
    }
    */
    if (link->numOpenThreads < 0) {
        printf("link->numOpenThreads somehow got to < 0. Resetting to 0.\n");
        link->numOpenThreads = 0;
    }
    
    pthread_mutex_unlock(&ILINK_LOCK);
    return SUCCESS;    
}


ErrCode beginTransaction(TxnState **txn)
{
    //int ret;
    //create the state variable for this transaction
    TXNState *txne = new TXNState;
    
   
    txne->cursorLink = NULL;
    
    txne->tid = rand();
    txne->status = 1;
    *txn = (TxnState*) txne;
    printf("Transaction started\n");
    //    DB_TXN *tid = NULL;a
      return SUCCESS;
}

    
/***
 * Not dealing with transactions yet
 */
ErrCode abortTransaction(TxnState *txn)
{
  TXNState* txne = (TXNState*) txn;

  txne->status = -1;
  
  printf("Transaction Aborted\n");
  return SUCCESS;
}

/***
 * Not dealing with transactions yet
 */
ErrCode commitTransaction(TxnState *txn)
{
    
    TXNState* txne = (TXNState*) txn;

    txne->status = -1;
  
    printf("Transaction Commited\n");
    return SUCCESS;
}
/**
 * Get the first record, I am going to ignore the transaction values
 * since we only have one index
 **/
ErrCode get(IdxState *idxState, TxnState *txn, Record *record)
{   
    STXDBState *state = (STXDBState*) idxState;
     std::pair<string, bool> ret;
    //stxbtree_type* dbp = state->dbp;
    //nbtree_int* nbt = (nbtree_int*)state->nbt;
    switch(state->type)
    {
	case SHORT: 
	    { 
	    nbtree_st* shtree = (nbtree_st*)state->nbt;
	    short k;
	    p_setKeyDataFromKey(&(record->key), &k);
            ret = shtree->get(k);
	    cout << "string returned " << ret.first << endl;
	    strcpy(record->payload, (ret.first).c_str());
	    break;
	    return SUCCESS;	    
	    }
	case INT: 
	    {
	    nbtree_int* intree = (nbtree_int*)state->nbt;
	    int k;
	    p_setKeyDataFromKey(&(record->key), &k);
	    ret = intree->get(k);
	    cout << "string returned " << ret.first << endl;
	      strcpy(record->payload, ret.first.c_str());
	      return SUCCESS;
	      break;
	    }
	case VARCHAR: 
	    {
	    nbtree_ch* chtree = (nbtree_ch*)state->nbt;
	    char* k;
	    p_setKeyDataFromKey(&(record->key), k);
	    string sk(k);
	    ret = chtree->get(sk);
	    cout << "string returend " << ret.first << endl;
	    return SUCCESS;
	    break;
		      }
	default:
	    return FAILURE;
    
    }
    
    TXNState* txne = (TXNState*) txn;
    if(txne->tid != state->tid) {
	state->tid = txne->tid;
	 memset(&(state->lastKey), 0, sizeof(Key));
	state->keyNotFound = 1;
    }

    memcpy(&(state->lastKey) , &(record->key), sizeof(Key));
    state->keyNotFound  = 0;

}


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
ErrCode getNext(IdxState *idxState, TxnState *txn, Record *record)
{
    STXDBState* state  = (STXDBState*) idxState;
    //stxbtree_type* dbp = state->dbp;
    void* nbt =(nbtree_int*) state->nbt;

    btinter bit;
    //if key wasn't found
    TXNState* txne = (TXNState*) txn;
    if(txne->tid != state->tid){
	state->tid = txne->tid;
    memset(&(state->lastKey), 0, sizeof(Key));
    state->keyNotFound = 1;
    
    if(state->keyNotFound == 1)
    {
	if(nbt->size() == 0)
	{
	    //state->keyNotFound = 0;
	    return DB_END;
	}
	//bit = dbp->begin();
	//copy data to payload
	 nbt->exits(state->lastKey)
	//move next
	///bit++;
	//strcpy(record->payload, bit->second.c_str());
	//memcpy(&(state->lastKey) , &(record->key), sizeof(Key));
	state->keyNotFound  = 0;
	return SUCCESS;
    }
    else
    {
	nbt->exists(state->lastKey);
	//move next
	bit++;
	if(bit == nbt->end())
	{   
	    return DB_END;
	}
	else
	{   
	    memcpy(&(record->key), &(bit->first), sizeof(Key));
	    memcpy(&(state->lastKey), &(bit->first), sizeof(Key));
	    strcpy(record->payload, bit->second.c_str());
	    state->keyNotFound  = 0;	 
	    return SUCCESS;
	}

    }
}

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
ErrCode insertRecord(IdxState *idxState, TxnState *txn, Key *k, const char* payload)
{

	void* vbt;
	void* mykey = NULL;
	STXDBState* state = (STXDBState*) idxState;
	
	p_setKeyDataFromKey(k, mykey);
	
	if(mykey == NULL)
	{
	    return FAILURE;
	}

	//stxbtree_type* dbp = state->dbp;
	//nbtree* nbt = state->nbt;
	
	std::string value(payload);
	//std::pair<btinter, btinter> range; 
	//range = dbp->equal_range(*k);
	switch(k->type)
	{
	    case SHORT:
		{
		if(state->type != SHORT)
		    return FAILURE;
		nbtree_st* nbt = (nbtree_st*) state->nbt;

	if(nbt->insert(*((short*)mykey), value) < 0)
	{
	    return ENTRY_EXISTS;
	}
		}
	else 
	    return SUCCESS;
		break;
	    case INT:
		{
		if(state->type != INT)
		    return FAILURE
		
		if(nbt->insert(*((int*)mykey), value) < 0)
		{
		    return ENTRY_EXISTS;
		}
		else 
		    return SUCCESS;
		break;
		}
	    case VARCHAR:
		{
		string stkey((char*)mykey);
		if(nbt->insert(stkey, value) < 0)
		{

		    return ENTRY_EXISTS;
		}
		else
		    return SUCCESS;
		break;

		}
	    default:
		return FAILURE;
	}
    	
	memcpy(&(state->lastKey) , k, sizeof(Key));

	return SUCCESS;
}


/**
 Remove the record associated with the given key from the index
 structure.  If a payload is specified in the Record, then the
 key/payload pair specified is removed. Otherwise, the payload pointer
 is a length 0 string and all records with the given key are removed from thelll
 database.  If this is called from outside of a transaction, it should
 commit immediately.

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
ErrCode deleteRecord(IdxState *idxState, TxnState *txn, Record *record)
{

	
	STXDBState* state = (STXDBState*) idxState;
	void* nbt = state->nbt;
    
	//determnine if there is a payload
	if(strlen(record->payload) != 0)
	{
	    if(nbt->erase_pair(record->key, record->payload) < 0)
		return KEY_NOTFOUND;
	}

    return SUCCESS;

}

