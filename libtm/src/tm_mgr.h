#ifndef _tm_mgr_basics_d_H
#define	_tm_mgr_basics_d_H

#include "tm_general.h"

#include "infra/bitset.h"
#include "infra/hashlib.h"
#include "infra/seqbuff.h"

#include "utils/tm_stats.h"

#include "tm_mgr_mm.h"
#include "tm_mgr_extras.h"


#define REDIRECTION_BIT 30


typedef struct
{
	word_t rlock;
	word_t wlock;
} tm_lock_t;


typedef struct
{
	tm_lock_t	lock;
	ptr_t 		link;
} meta_t;

/***********************************************************
		READS	INFRASTRUCTURE
***********************************************************/

typedef struct _read_t
{
    tm_lock_t*    	lock;
} read_t;

typedef seqbuff_t	read_set_t;
#define read_set_init	seqbuff_init


/***********************************************************
		WRITES	INFRASTRUCTURE
***********************************************************/

typedef struct _write_t
{
    //burceam: we reversed these, so that we can index in the hashtable by addr, and not by lock;
    //this is because when using a global lock, you can no longer index in the hashtable by the lock, 
    //which is now the same.
    //(daniel is indexing based on the first 4 bytes, the first integer)
    //(initially, lock was the first member, and addr was the second one; since daniel always indexes
    //based on the first element, this meant that he used to index based on the lock; however, now
    //that we can make things point to a global lock, it would be inefficient to index in the 
    //hashtable based on that, because it would be the same for (potentially many) different players;
    //so instead, we exchanged the order of the first two members, such that the addr is first,
    //and the lock is second; presumably this is easier to do than to explicitly index on the lock,
    //or there may be other causes, but I cannot recall them).
    ptr_t           addr;
    tm_lock_t*      lock;

    uint_t          sz;
    byte_t          twin[0];
} write_t;

typedef 	hshtbl		write_set_t;
#define 	write_set_init  hsh_init


/***********************************************************
 *
 *  	TRAN_T
 *
 **********************************************************/

typedef struct _tran_t
{
	uint_t     	tran_id;
	
	read_set_t	reads;  //  read owned tm_locks
        write_set_t	writes; //  read & write owned tm_locks

	commit_free_set_t	commit_frees;
	abort_free_set_t	abort_frees;
	delayed_free_set_t	delayed_frees;
	
	#ifdef ENABLE_EXTRAS
        twin_set_t twins; //  twins for non-shared variables
        #endif

	jmp_buf	jbuf;
	
	cstats_t stats;
	
	//thread-global lock
	volatile unsigned int global_meta[3];
} tran_t;


extern tran_t  p_trans[MAX_NO_THREADS];


extern 	word_t     __no_th;
extern __thread    int p_thread_id;
extern __thread    int __thread_id;


extern tm_lock_t*	volatile deadlock_table[ MAX_NO_THREADS*16 ];
extern uint_t		volatile aborted_table[ MAX_NO_THREADS*16 ];

extern uint_t decode_table[129];



int mgr_on_init();

void commit_frees( tran_t* f_tran );
void abort_frees( tran_t* f_tran );

void commit_twins( tran_t* t_tran );
void abort_twins( tran_t* t_tran );


#endif	/* _tm_mgr_basics_H */

