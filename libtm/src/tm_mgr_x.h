#include "tm_general.h"
#include "tm_mgr.h"



#if defined( RAW_Pes ) && defined( WAR_Pes ) && defined( WAW_Pes )
#include "tm_lock/tm_lock_xx.h"
#endif
#if defined( RAW_Pes ) && defined( WAR_Opt ) && defined( WAW_Pes )
#include "tm_lock/tm_lock_xy.h"
#endif
#if defined( RAW_Opt ) && defined( WAR_Opt ) && defined( WAW_Pes )
#include "tm_lock/tm_lock_xy.h"
#endif
#if defined( RAW_Opt ) && defined( WAR_Opt ) && defined( WAW_Opt )
#include "tm_lock/tm_lock_xx.h"
#endif





/***********************************************************
		READS	WRAPPERS
***********************************************************/

#define read_set_put( r_lock, r_reads )		seqbuff_put_ptr( r_reads, sizeof(read_t) )
#define read_set_for_each( r_rd, r_reads )	seqbuff_for_each( r_rd, r_reads, read_t, 0 )

//burceam: for read, you lock only once for all the objects that you lock - there is no need for any
//per-item bookkeeping, because i don't have entries in the write buffer; for writes, you do need
//bookkeeping per item, because every written item has to have an entry in the write buffer.
#define rd_has(  r_meta, r_tran_id )		tm_lock_has_rd( &(r_meta)->lock, r_tran_id )
#define rd_askfor( r_meta, r_tran_id )		tm_lock_rdlock( &(r_meta)->lock, r_tran_id )
#define rd_release( r_rd, r_tran_id )		tm_lock_rdunlock( r_rd->lock, r_tran_id )

#define rd_record( r_meta, r_tran_id )								\
({ 												\
	read_t* rd = (read_t*)read_set_put( &(r_meta)->lock, &p_trans[r_tran_id].reads );	\
	rd->lock = &(r_meta)->lock;								\
})

/***********************************************************
		WRITES	WRAPPERS
***********************************************************/

#ifdef WRITE_SET_SEQB

#define write_set_put( w_addr, w_sz, w_writes )		seqbuff_put_ptr( &(w_writes)->seqb, sizeof(write_t)+(w_sz) )
#define write_set_for_each(  w_wr, w_writes )		seqbuff_for_each( w_wr, &(w_writes)->seqb, write_t, (w_wr)->sz )
#define write_set_pass_by( w_wp, w_writes )		seqbuff_pass_by( w_wp, &(w_writes)->seqb, write_t, (w_wp)->sz )


#define wr_has_lock( w_meta, w_addr, w_tran_id )	tm_lock_has_wr( &(w_meta)->lock, w_tran_id )
#define wr_has_wb_entry( w_meta, w_addr, w_tran_id, w_meta_local )	\
                   ( tm_lock_has_wr( &(w_meta)->lock, w_tran_id ) && (w_meta_local->link) )

#endif


#ifdef WRITE_SET_HTAB

#define write_set_put( w_addr, w_sz, w_writes ) 	hsh_insert_r2( w_writes, (int_pointer_t)(w_addr), sizeof(write_t) + (w_sz) )
#define write_set_for_each( w_wr, w_writes )		hsh_for_each_r( w_wr, w_writes, write_t, (w_wr)->sz )
#define write_set_pass_by( w_wp, w_writes )		hsh_pass_by_r( w_wp, w_writes, write_t, (w_wp)->sz )


#define wr_has( w_addr, w_tran_id )			\
	(res_addr = (ptr_t)hsh_find_r2( &p_trans[w_tran_id].writes, (int_pointer_t)(w_addr) ))

	
#define wr_has_lock( w_meta, w_addr, w_tran_id )			wr_has( w_addr, w_tran_id )
#define wr_has_wb_entry( w_meta, w_addr, w_tran_id, w_meta_local )	wr_has( w_addr, w_tran_id )


#endif


#if defined( WAW_Opt )

#define wr_askfor( w_lock, w_tran_id )

//here i must verify if I already have the lock (if it is the global lock, i just might);
#define wr_pre_commit( w_wr, w_tran_id )			\
	if( !(tm_lock_has_wr( w_wr->lock, w_tran_id ) ) )	tm_lock_wrlock( w_wr->lock, w_tran_id )

#else

#define wr_askfor( w_lock, w_tran_id )		tm_lock_wrlock( w_lock, w_tran_id )
#ifdef WAR_Opt
#define wr_pre_commit( w_wr, w_tran_id )	tm_lock_wrlock_pre_com( w_wr->lock, w_tran_id )
//here it is no longer fully optimistic, we acquire the lock at access time, and resolve conflicts at
//commit time; daniel thinks that conflict resolution is re-entrant, and it's ok if you solve
//conflicts multiple times (although it is expensive); we're leaving it like this, for the time being.
#endif

#endif


//here we need to reset the link in the write buffer, it wasn't done before.

___always_inline static	
void wr_release_link( write_t* w_wr, uint_t w_tran_id ) 
{
	#ifdef WRITE_SET_SEQB
	meta_t * _meta_local = (meta_t *) (w_wr->addr + ((w_wr->sz-1)/4+1)*4  );
	_meta_local->link = NULL;

	//tm_assert( w_wr->lock->wlock == 0 );
	tm_assert( _meta_local->link == 0 );

	//tm_assert( w_wr->lock->rlock == 0 );		
	tm_assert( ((meta_t *)w_wr->lock)->link == 0 );

	tm_assert( (ptr_t)_meta_local == (ptr_t)w_wr->lock || _meta_local->lock.rlock == (1<<REDIRECTION_BIT) );
	tm_assert( (ptr_t)_meta_local == (ptr_t)w_wr->lock || (ptr_t)_meta_local->lock.wlock == (ptr_t)w_wr->lock );
	#endif
}

___always_inline static	
void wr_release_lock( write_t* w_wr, uint_t w_tran_id ) 
{
	if( tm_lock_has_wr( w_wr->lock, w_tran_id ) )
		tm_lock_wrunlock( w_wr->lock, w_tran_id );
}



#if defined( WRITE_BUFFERING ) && defined( WRITE_SET_SEQB )
#define get_res_addr()	(_meta_local->link)
#define set_res_addr()	(w_meta_local->link = wr->twin) //here we save the address of the copy that is in the write buffer
#endif

#if defined( WRITE_BUFFERING ) && defined( WRITE_SET_HTAB )
#define get_res_addr()	(res_addr += sizeof( write_t ))
#define set_res_addr()	(wr->twin)
#endif

___always_inline  static
ptr_t wr_record( meta_t* w_meta, ptr_t w_addr, uint_t w_sz, uint_t w_tran_id, meta_t * w_meta_local )										
{
	write_t* wr = (write_t*)write_set_put( w_addr, w_sz, &p_trans[w_tran_id].writes );	
	wr->addr = w_addr;																	
	wr->lock = &(w_meta)->lock;															
	wr->sz = w_sz;
	
	return set_res_addr();																		
}




/**********************************************************************
		ON_RD
**********************************************************************/

___always_inline static
ptr_t _mgr_on_rd( int l_tran_id, meta_t* _meta, ptr_t addr, uint_t sz, ptr_t r_res, meta_t * _meta_local )
{
	ptr_t	res_addr = addr;
	
	if( wr_has_wb_entry( _meta, addr, l_tran_id, _meta_local ) )	
	    return get_res_addr();

	if( !rd_has( _meta, l_tran_id ) )
	{
		rd_askfor( _meta, l_tran_id );
		rd_record( _meta, l_tran_id );
	}
	
	#ifdef ABORT_READERS
	tm_memcpy( r_res, addr, sz );
	aborted_check( l_tran_id );
	res_addr = r_res;
	#else
	res_addr = addr;
	#endif

	return res_addr;
}


/**********************************************************************
		ON_WR
**********************************************************************/

___always_inline static
ptr_t _mgr_on_wr( int l_tran_id, meta_t* _meta, ptr_t addr, uint_t sz, meta_t * _meta_local )
{
	ptr_t res_addr = addr;
	
	if( !wr_has_lock( _meta, addr, l_tran_id ) )	
	    wr_askfor( &_meta->lock, l_tran_id );
	
	if(  wr_has_wb_entry( _meta, addr, l_tran_id, _meta_local ) )		
	    return get_res_addr();
	
	res_addr = wr_record( _meta, addr, sz, l_tran_id, _meta_local );
	return res_addr;
}


/**********************************************************************
		ON_COMMIT
**********************************************************************/

___always_inline static
void _mgr_on_commit( int l_tran_id, tran_t* l_tran, uint_t* n_rds, uint_t* n_wrs  )
{
	write_set_t* writes = &l_tran->writes;
	read_set_t*  reads  = &l_tran->reads;

	
	#ifdef WAR_Opt
	write_set_pass_by( wp2, writes )
		wr_pre_commit( wp2, l_tran_id );

	aborted_check( l_tran_id );
	#endif

	write_set_pass_by( wp3, writes )
	{
		wr_release_link( wp3, l_tran_id );
		tm_memcpy( wp3->addr, wp3->twin, wp3->sz );
	}

	write_set_for_each( wr, writes )
	{
		wr_release_lock( wr, l_tran_id );
		(*n_wrs)++;
	}

	read_set_for_each(  rd, reads )
	{
		rd_release(  rd, l_tran_id );
		(*n_rds)++;
	}	

	commit_twins( l_tran );
	commit_frees( l_tran );
}


/**********************************************************************
		ON_ABORT
**********************************************************************/

___always_inline static
void _mgr_on_abort( int l_tran_id, tran_t* l_tran )
{
	write_set_t* writes = &l_tran->writes;
	read_set_t*  reads  = &l_tran->reads;
	
	write_set_pass_by( wp2, writes )
	wr_release_link( wp2, l_tran_id );

	write_set_for_each( wr, writes )
	wr_release_lock( wr, l_tran_id );
	
	read_set_for_each(  rd, reads )
	rd_release(  rd, l_tran_id );
		
	abort_twins( l_tran );
	abort_frees( l_tran );
}




/**************************************************************************/
/*********************     TM_MGR WRAPPERS      ***************************/
/**************************************************************************/

// returns the correct _meta to be used for this object
___always_inline static
ptr_t mgr_get_meta( ptr_t _meta )
{
	if( is_set( ((uint_t*) _meta)[0], REDIRECTION_BIT ) ) 
	{
		//_meta is the local metadata word of the data
		// here we get the global_meta of thread that owns it (OWNERSHIP bit)
		_meta = (ptr_t) ((uint_t*) _meta)[1];
	}

	return _meta;
}


ptr_t mgr_on_rd_x( ptr_t _meta, ptr_t addr, uint_t sz, ptr_t r_res )
{
	int l_tran_id = __thread_id;
	if( unlikely( l_tran_id == -1 ) )	
	    return addr;

	aborted_check( l_tran_id );
	
	meta_t * _meta_local = (meta_t *)_meta;
	_meta = mgr_get_meta( _meta );	
	
	//tm_log3( "%x - mgr_on_rd - a: %x m: %x ml: %x\n", (uint_t)addr, (uint_t)_meta, (uint_t)_meta_local );
	return _mgr_on_rd( l_tran_id, (meta_t*)_meta, addr, sz, r_res, _meta_local );
}


ptr_t mgr_on_wr_x( ptr_t _meta, ptr_t addr, uint_t sz )
{
	int l_tran_id = __thread_id;
	if( unlikely( l_tran_id == -1 ) )	
	    return addr;

	aborted_check( l_tran_id );

	meta_t * _meta_local = (meta_t *)_meta;
	_meta = mgr_get_meta( _meta );	
	//tm_log3( "%x - mgr_on_wr - a: %x m: %x ml: %x\n", (uint_t)addr, (uint_t)_meta, (uint_t)_meta_local );

	return _mgr_on_wr( l_tran_id, (meta_t*)_meta, addr, sz, _meta_local );
}


void mgr_on_commit_x()
{
	int l_tran_id = __thread_id;	
	tm_assert( l_tran_id != -1 );
	tran_t*	l_tran = &p_trans[ l_tran_id ];
	
	aborted_check( l_tran_id );
	tm_log( "%x - mgr_on_commit\n" );
	
	uint_t n_wrs = 0;
	uint_t n_rds = 0;
	_mgr_on_commit( l_tran_id, l_tran, &n_rds, &n_wrs );
	
	cstats_commit( &l_tran->stats, n_rds, n_wrs );
	aborted_table[ l_tran_id*16 ] = 0;
	
	__thread_id = -1;
}


void mgr_on_abort_x( int reason )
{
	int l_tran_id = __thread_id;	
	tm_assert( l_tran_id != -1 );
	tran_t*	l_tran = &p_trans[ l_tran_id ];

	clear_deadlock_table( l_tran_id );
	int invalidated = aborted_table[ l_tran_id*16 ];
	tm_log1( "%x - mgr_on_abort - inv(%d)\n", invalidated );

	_mgr_on_abort( l_tran_id, l_tran );
	
	cstats_abort( &l_tran->stats, invalidated, reason );
	aborted_table[ l_tran_id*16 ] = 0;
	
	longjmp( l_tran->jbuf, 1 );
}

#ifdef ABORT_READERS

void mgr_on_check_x()
{
	int l_tran_id = __thread_id;
	if( unlikely( l_tran_id != -1 ) )	
	    aborted_check( l_tran_id );	
}

#endif



