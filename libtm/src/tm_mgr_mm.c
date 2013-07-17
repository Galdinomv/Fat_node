#include "tm_mgr_mm.h"
#include "tm_mgr.h"


/**********************************************************************
		MEMMAG
**********************************************************************/

volatile uint_t epochs[MAX_NO_THREADS*16] = {0};


void* mgr_on_new( size_t size )
{
	void* ptr = malloc(size);
	memset( ptr, 0, size );
	
	tran_t*	l_tran = ( __thread_id != -1 ) ? &p_trans[ __thread_id ] : NULL;
	if( l_tran )	
	    abort_free_set_put( ptr, &l_tran->abort_frees );

	return ptr;
}

void mgr_on_delete( void* ptr )
{
	free(ptr);
}

void tm_delete( void* ptr )
{
	tran_t*	l_tran = ( __thread_id != -1 ) ? &p_trans[ __thread_id ] : NULL;
	if( l_tran )	
	    commit_free_set_put( ptr, &l_tran->commit_frees );
	else			
	    free(ptr);
}




void commit_frees( tran_t* f_tran )
{
	commit_free_set_t* commit_frees = &f_tran->commit_frees;					
	delayed_free_set_t* delayed_frees = &f_tran->delayed_frees;				
	commit_free_set_for_each( comfr, commit_frees )								
	delayed_free_set_put( (*comfr), delayed_frees );
	
	abort_free_set_t* abort_frees = &f_tran->abort_frees;						
	abort_free_set_for_each( abrfr, abort_frees );
	
	epochs[ f_tran->tran_id*16 ]++;	
}


void abort_frees( tran_t* f_tran )
{
	commit_free_set_t* commit_frees = &f_tran->commit_frees;					
	commit_free_set_for_each( comfr, commit_frees );
	
	
	abort_free_set_t* abort_frees = &f_tran->abort_frees;						
	delayed_free_set_t* delayed_frees = &f_tran->delayed_frees;				
	abort_free_set_for_each( abrfr, abort_frees )								
	delayed_free_set_put( (*abrfr), delayed_frees );
	
	epochs[ f_tran->tran_id*16 ] += 2;
}
