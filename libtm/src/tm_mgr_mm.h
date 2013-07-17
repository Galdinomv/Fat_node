#ifndef TM_MGR_MM_H_
#define TM_MGR_MM_H_

#include "tm_general.h"
#include "infra/seqbuff.h"

/***********************************************************
 *
 *		MEMORY MANAGEMENT INFRASTRUCTURE
 *
 ***********************************************************/


/************    DELAYED FREE INFRASTRUCTURE    ************/

extern volatile uint_t epochs[MAX_NO_THREADS*16];
#define POOL_SIZE	32



___always_inline
bool isStrictlyOlder(uint_t* newer, uint_t* older)
{
	uint_t i;
	for( i = 0; i < MAX_NO_THREADS; i++)
		if( (newer[i] <= older[i]) && (newer[i] % 2 == 1) )
			return false;
	return true;
}
___always_inline
void copy_globalEpoch(uint_t* ts_ptr)
{
	uint_t i;
	for( i = 0; i < MAX_NO_THREADS; i++)
		ts_ptr[i] = epochs[i*16];
}



typedef struct _limbo_t
{
	void* pool[POOL_SIZE];
	uint_t ts[MAX_NO_THREADS];
	uint_t length;
	struct _limbo_t* older;  // next limbo list
} limbo_t;

#define limbo_init( l_lim ) \
({							\
	l_lim = (limbo_t*)malloc( sizeof(limbo_t) );	\
	l_lim->length = 0;				\
	l_lim->older = NULL;				\
})

typedef struct
{
	limbo_t* prelimbo;
	limbo_t* limbo;
} delayed_free_set_t;




#define delayed_free_set_init( df_delayed_frees )	\
({							\
	(df_delayed_frees)->limbo = NULL;		\
	limbo_init( (df_delayed_frees)->prelimbo );	\
})

___always_inline
void delayed_free_set_transfer( delayed_free_set_t* df_delayed_frees )
{
    // we're going to move prelimbo to the head of the limbo list, and then
    // clean up anything on the limbo list that has become dominated

    limbo_t* df_prelimbo = df_delayed_frees->prelimbo;

    // now get the current timestamp from the epoch.
    copy_globalEpoch(df_prelimbo->ts);

    // push prelimbo onto the front of the limbo list:
    df_prelimbo->older = df_delayed_frees->limbo;
    df_delayed_frees->limbo = df_prelimbo;

    // loop through everything after limbo->head, comparing the timestamp to
    // the head's timestamp.  Exit the loop when the list is empty, or when we
    // find something that is strictly dominated.  NB: the list is in sorted
    // order by timestamp.
    limbo_t* df_limbo = df_delayed_frees->limbo;
    limbo_t* current = df_limbo->older;
    limbo_t* prev = df_limbo;
    while( current != NULL )
	{
        if( isStrictlyOlder(df_limbo->ts, current->ts))            
            break;
        else
		{
            prev = current;
            current = current->older;
        }
    }

    // If current != NULL, then it is the head of the kill list
    if (current) {
        // detach /current/ from the list
        prev->older = NULL;

        // for each node in the list headed by current, delete all blocks in
        // the node's pool, delete the node's timestamp, and delete the node
        while (current != NULL)
		{
            // free blocks in current's pool
            for (int i = 0; i < POOL_SIZE; i++)		
               free( current->pool[i] );
            
            // free the node and move on
            limbo_t* old = current;
            current = current->older;
            free(old);
        }
    }
}


#define delayed_free_set_put( df_ptr, df_delayed_frees )	\
({								\
    limbo_t* df_prelimbo = df_delayed_frees->prelimbo;		\
    df_prelimbo->pool[df_prelimbo->length] = df_ptr;		\
    df_prelimbo->length++;					\
								\
    if (df_prelimbo->length == POOL_SIZE) {			\
	delayed_free_set_transfer( df_delayed_frees );		\
        limbo_init( df_delayed_frees->prelimbo );		\
    }								\
})


/************    COMMIT FREE INFRASTRUCTURE    ************/

typedef void* commit_free_t;

typedef seqbuff_t commit_free_set_t;

#define commit_free_set_init	sm_seqbuff_init

#define commit_free_set_put( cf_ptr, cf_commit_frees )	\
	({ (*(commit_free_t*)sm_seqbuff_put_ptr( cf_commit_frees, sizeof(commit_free_t) )) = (cf_ptr); })
#define commit_free_set_for_each( cf_comfr, cf_commit_frees )	\
	seqbuff_for_each( cf_comfr, cf_commit_frees, commit_free_t, 0 )



/************    ABORT FREE	INFRASTRUCTURE    ************/

typedef void* abort_free_t;

typedef seqbuff_t abort_free_set_t;

#define abort_free_set_init sm_seqbuff_init

#define abort_free_set_put( af_ptr, af_abort_frees )	\
	({ (*(abort_free_t*)sm_seqbuff_put_ptr( af_abort_frees, sizeof(abort_free_t) )) = (af_ptr); })
#define abort_free_set_for_each( af_abrfr, af_abort_frees )	\
	seqbuff_for_each( af_abrfr, af_abort_frees, abort_free_t, 0 )







#endif /*TM_MGR_MM_H_*/
