#include "tm_mgr.h"

 word_t     __no_th =  0;
__thread    int p_thread_id = -1;
__thread    int __thread_id = -1;


tm_lock_t*	volatile	deadlock_table[ MAX_NO_THREADS*16 ] = {0};
uint_t		volatile	aborted_table[ MAX_NO_THREADS*16 ] = {0};

tran_t  p_trans[MAX_NO_THREADS] = {{0}};

/**********************************************************************
		ON_INIT
**********************************************************************/

int mgr_on_init()
{
	int l_thread_id = atomic_add_return_prev( 1, &__no_th );
	tm_assert( l_thread_id < MAX_NO_THREADS );
	
	tran_t* l_tran = &p_trans[ l_thread_id ];
	
	read_set_init(  &l_tran->reads );
	write_set_init( &l_tran->writes );

	commit_free_set_init( &l_tran->commit_frees );
	abort_free_set_init( &l_tran->abort_frees );
	delayed_free_set_init( &l_tran->delayed_frees );	
	
	#ifdef ENABLE_EXTRAS
	twin_set_init(  &l_tran->twins );
	#endif

	l_tran->global_meta[0] = 0;
	l_tran->global_meta[1] = 0;
	l_tran->global_meta[2] = 0;
	
	cstats_init( &l_tran->stats );
	
	l_tran->tran_id = l_thread_id;
	p_thread_id = l_thread_id;
	
	return l_thread_id;
}

/**********************************************************************
		ON_BEGIN
**********************************************************************/

jmp_buf* mgr_on_begin_all()
{
	int l_thread_id = p_thread_id;
	if( l_thread_id == -1 )		
	   l_thread_id = mgr_on_init();
	
	tm_assert( __thread_id == -1 );
	__thread_id = l_thread_id;

	tm_log( "%x - mgr_on_begin\n" );
	
	epochs[ l_thread_id*16 ]++;	
	cstats_begin( &p_trans[ l_thread_id ].stats );
	
	return &(p_trans[ l_thread_id ].jbuf);
}


/**********************************************************************
		OWNERSHIP_ASSIGNMENT
**********************************************************************/

void mgr_on_assign( ptr_t a_meta, ptr_t new_meta ) 
{
	//int		l_thread_id = p_thread_id;	tm_assert( l_thread_id != -1 );
	
	//we're gonna use REDIRECTION_BIT of the first meta word to signal redirection to a different lock
	//REDIRECTION_BIT = 1 means that this object is managed through the "new_meta" lock
	//REDIRECTION_BIT = 0 means that this object is managed through the local _meta lock
	
	if (((uint_t *) a_meta)[0] != (1 << REDIRECTION_BIT)) 
  	   ((uint_t *) a_meta)[0] = (1 << REDIRECTION_BIT);
  	
  	if (((uint_t *) a_meta)[1] != (uint_t) new_meta)
	   ((uint_t *) a_meta)[1] = (uint_t) new_meta;//p_trans[ l_thread_id ].global_meta;
	//this tells us that this object is managed (REDIRECTION_BIT) by the "new_meta" lock
}


void mgr_on_unassign (ptr_t a_meta) 
{

	//make the object associated with this metadata shared (among threads), as it is by default
	((uint_t *) a_meta)[0] = 0;
	((uint_t *) a_meta)[1] = 0;
}

