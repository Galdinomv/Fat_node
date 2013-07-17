#include "tm_mgr.h"


/***********************************************************
 *
 *		EXTRAS	INFRASTRUCTURE
 *
 ***********************************************************/


#ifdef ENABLE_EXTRAS




/****    TWINS	INFRASTRUCTURE    ****/

void mgr_on_tw( ptr_t addr, uint_t sz )
{
	if( __thread_id == -1 )	
	   return;
	tran_t*	l_tran = &p_trans[ __thread_id ];
	
	if( has_tw( addr, l_tran ) )
	   return;	
	twin_set_put( addr, sz, &l_tran->twins );
}

void commit_twins( tran_t* t_tran )									
{																
	twin_set_t*  twins  = &t_tran->twins;						
	twin_set_for_each(  tw, twins )		;		
}

void abort_twins( tran_t* t_tran )									
{																
	twin_set_t*  twins  = &t_tran->twins;						
	twin_set_for_each(  tw, twins )		
	memcpy( tw->addr, tw->twin, tw->sz );		
}


#else


void commit_twins( tran_t* t_tran ){}
void abort_twins( tran_t* t_tran ){}

#endif	// ENABLE_EXTRAS
