#include "../tm_general.h"
#include "../utils/hrtime.h"

#include "../infra/tatas.h"
#include "../infra/bitset.h"

#include "../tm_mgr.h" 


tatas_lock_t _big_lock = 0;

jmp_buf* mgr_on_begin_00()
{
	int l_thread_id = p_thread_id;
	if( l_thread_id == -1 )		
	    l_thread_id = mgr_on_init();
	tran_t*	l_tran = &p_trans[ l_thread_id ];
	
	#ifdef TIMINGS
	l_tran->stats.begin = get_c();
	#endif
	l_tran->stats.t_waiting_raw += tatas_lock( &_big_lock );
	
	return &(l_tran->jbuf);
}

void mgr_on_commit_00()
{
	int l_thread_id = p_thread_id;	
	tm_assert( l_thread_id != -1 );
	tran_t*	l_tran = &p_trans[ l_thread_id ];
	
	tatas_unlock( &_big_lock );
	
	#ifdef TIMINGS
	l_tran->stats.t_commits += get_c() - l_tran->stats.begin;
	#endif
	l_tran->stats.n_commits++;	
}
