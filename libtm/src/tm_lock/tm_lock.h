#ifndef _tm_lock_H
#define	_tm_lock_H 

#include "../infra/bitset.h"
#include "../infra/tatas.h"


#define WRITE_BIT	31

void mgr_on_abort_x(int reason);
#define ABORT_TRANSACTION(_reason)	mgr_on_abort_x(_reason)


#ifdef ABORT_READERS

extern uint_t		volatile	aborted_table[ MAX_NO_THREADS*16 ];
#define aborted_check( l_tran_id )	if( unlikely( aborted_table[ l_tran_id*16 ] ) )	ABORT_TRANSACTION(0)
#define aborted_check2( l_tran_id )	aborted_table[ l_tran_id*16 ]

#else

#define aborted_check( l_tran_id )
#define aborted_check2( l_tran_id ) 0
#endif



extern tm_lock_t* volatile deadlock_table[ MAX_NO_THREADS*16 ];



#define set_deadlock_table( _tran_id, __lock )	\
	if( !deadlock_table[_tran_id*16] )	atomic_set_mask( __lock, &deadlock_table[_tran_id*16] )
#define clear_deadlock_table( _tran_id )	\
	if( deadlock_table[_tran_id*16] )	deadlock_table[_tran_id*16] = 0;

//	if( deadlock_table[_tran_id*16] )		atomic_clear_mask( 0xffffffff, &deadlock_table[_tran_id*16] )





#endif




