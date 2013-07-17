#ifndef TM_VERS_H_
#define TM_VERS_H_

#include "../tm_general.h"


jmp_buf* mgr_on_begin_00();
void	mgr_on_commit_00();

jmp_buf* mgr_on_begin_all();

ptr_t	mgr_on_rd_11( ptr_t r_meta, ptr_t r_addr, uint_t sz, ptr_t r_res );
ptr_t	mgr_on_wr_11( ptr_t w_meta, ptr_t w_addr, uint_t sz );
void	mgr_on_commit_11();

ptr_t	mgr_on_rd_12( ptr_t r_meta, ptr_t r_addr, uint_t sz, ptr_t r_res );
ptr_t	mgr_on_wr_12( ptr_t w_meta, ptr_t w_addr, uint_t sz );
void	mgr_on_commit_12();
void	mgr_on_check_12();

ptr_t	mgr_on_rd_21( ptr_t r_meta, ptr_t r_addr, uint_t sz, ptr_t r_res );
ptr_t	mgr_on_wr_21( ptr_t w_meta, ptr_t w_addr, uint_t sz );
void	mgr_on_commit_21();

ptr_t	mgr_on_rd_22( ptr_t r_meta, ptr_t r_addr, uint_t sz, ptr_t r_res );
ptr_t	mgr_on_wr_22( ptr_t w_meta, ptr_t w_addr, uint_t sz );
void	mgr_on_commit_22();
void	mgr_on_check_22();

ptr_t	mgr_on_rd_31( ptr_t r_meta, ptr_t r_addr, uint_t sz, ptr_t r_res );
ptr_t	mgr_on_wr_31( ptr_t w_meta, ptr_t w_addr, uint_t sz );
void	mgr_on_commit_31();

ptr_t	mgr_on_rd_32( ptr_t r_meta, ptr_t r_addr, uint_t sz, ptr_t r_res );
ptr_t	mgr_on_wr_32( ptr_t w_meta, ptr_t w_addr, uint_t sz );
void	mgr_on_commit_32();
void	mgr_on_check_32();

ptr_t	mgr_on_rd_41( ptr_t r_meta, ptr_t r_addr, uint_t sz, ptr_t r_res );
ptr_t	mgr_on_wr_41( ptr_t w_meta, ptr_t w_addr, uint_t sz );
void	mgr_on_commit_41();

ptr_t	mgr_on_rd_42( ptr_t r_meta, ptr_t r_addr, uint_t sz, ptr_t r_res );
ptr_t	mgr_on_wr_42( ptr_t w_meta, ptr_t w_addr, uint_t sz );
void	mgr_on_commit_42();
void	mgr_on_check_42();



#ifdef VERSION_DYNAMIC

extern volatile unsigned int	cd_version;
extern volatile unsigned int	cr_version;

inline
jmp_buf*	mgr_on_begin()
{
	if( cd_version == 0 && cr_version == 0 )		
	    return mgr_on_begin_00();

	return mgr_on_begin_all();
}

inline
ptr_t	mgr_on_rd( ptr_t r_meta, ptr_t r_addr, uint_t sz, ptr_t r_res )
{
	if( cd_version == 0 && cr_version == 0 )		
	    return r_addr;

	if( cd_version == 1 )
	{
		if( cr_version == 1 )		
		    return mgr_on_rd_11( r_meta, r_addr, sz, r_res );
		if( cr_version == 2 )		
		    return mgr_on_rd_12( r_meta, r_addr, sz, r_res );
	}
	if( cd_version == 2 )
	{
		if( cr_version == 1 )		
		    return mgr_on_rd_21( r_meta, r_addr, sz, r_res );
		if( cr_version == 2 )		
		    return mgr_on_rd_22( r_meta, r_addr, sz, r_res );
	}
	if( cd_version == 3 )
	{
		if( cr_version == 1 )		
		    return mgr_on_rd_31( r_meta, r_addr, sz, r_res );
		if( cr_version == 2 )		
		    return mgr_on_rd_32( r_meta, r_addr, sz, r_res );
	}
	if( cd_version == 4 )
	{
		if( cr_version == 1 )		
		    return mgr_on_rd_41( r_meta, r_addr, sz, r_res );
		if( cr_version == 2 )		
		    return mgr_on_rd_42( r_meta, r_addr, sz, r_res );
	}
	tm_assert(0);
	return NULL;
}

inline
ptr_t	mgr_on_wr( ptr_t w_meta, ptr_t w_addr, uint_t sz )
{
	if( cd_version == 0 && cr_version == 0 )		
	    return w_addr;

	if( cd_version == 1 )
	{
		if( cr_version == 1 )		
		    return mgr_on_wr_11( w_meta, w_addr, sz );
		if( cr_version == 2 )		
		    return mgr_on_wr_12( w_meta, w_addr, sz );
	}
	if( cd_version == 2 )
	{
		if( cr_version == 1 )		
		    return mgr_on_wr_21( w_meta, w_addr, sz );
		if( cr_version == 2 )		
		    return mgr_on_wr_22( w_meta, w_addr, sz );
	}
	if( cd_version == 3 )
	{
		if( cr_version == 1 )		
		    return mgr_on_wr_31( w_meta, w_addr, sz );
		if( cr_version == 2 )		
		    return mgr_on_wr_32( w_meta, w_addr, sz );
	}
	if( cd_version == 4 )
	{
		if( cr_version == 1 )		
		    return mgr_on_wr_41( w_meta, w_addr, sz );
		if( cr_version == 2 )		
		    return mgr_on_wr_42( w_meta, w_addr, sz );
	}
	tm_assert(0);
	return NULL;
}

inline
void mgr_on_commit()
{
	if( cd_version == 0 && cr_version == 0 )		
	    return mgr_on_commit_00();

	if( cd_version == 1 )
	{
		if( cr_version == 1 )		
		    return mgr_on_commit_11();
		if( cr_version == 2 )		
		    return mgr_on_commit_12();
	}
	if( cd_version == 2 )
	{
		if( cr_version == 1 )		
		    return mgr_on_commit_21();
		if( cr_version == 2 )		
		    return mgr_on_commit_22();
	}
	if( cd_version == 3 )
	{
		if( cr_version == 1 )		
		    return mgr_on_commit_31();
		if( cr_version == 2 )		
		    return mgr_on_commit_32();
	}
	if( cd_version == 4 )
	{
		if( cr_version == 1 )		
		    return mgr_on_commit_41();
		if( cr_version == 2 )		
		    return mgr_on_commit_42();
	}
	tm_assert(0);
}

inline
void mgr_on_check()
{
	if( cr_version == 2 )
	{
		if( cd_version == 1 )	mgr_on_check_12();
		if( cd_version == 2 )	mgr_on_check_22();
		if( cd_version == 3 )	mgr_on_check_32();
		if( cd_version == 4 )	mgr_on_check_42();
	}
}

void print_version( FILE* f_out );
void set_version( int cd_v, int cr_v );

#endif

#ifdef VERSION_00

#define	mgr_on_begin()				mgr_on_begin_00()
#define	mgr_on_rd( r_meta, r_addr, sz, r_res )	({ *r_res = 0; r_addr; })
#define	mgr_on_wr( w_meta, w_addr, sz )		(w_addr)
#define	mgr_on_commit()				mgr_on_commit_00()
#define	mgr_on_check()				()
#define	print_version( f_out )			fprintf( f_out, "Biglock " )
#define	set_version( cd_v, cr_v )

#endif


#ifdef VERSION_11

#define	mgr_on_begin()				mgr_on_begin_all()
#define	mgr_on_rd( r_meta, r_addr, sz, r_res )	mgr_on_rd_11( r_meta, r_addr, sz, r_res )
#define	mgr_on_wr( w_meta, w_addr, sz )		mgr_on_wr_11( w_meta, w_addr, sz )
#define	mgr_on_commit()				mgr_on_commit_11()
#define	mgr_on_check()
#define	print_version( f_out )			fprintf( f_out, "RAW_Pes_WAR_Pes_WAW_Pes_WAITFOR_READERS " )
#define	set_version( cd_v, cr_v )

#endif

#ifdef VERSION_12

#define	mgr_on_begin()				mgr_on_begin_all()
#define	mgr_on_rd( r_meta, r_addr, sz, r_res )	mgr_on_rd_12( r_meta, r_addr, sz, r_res )
#define	mgr_on_wr( w_meta, w_addr, sz )		mgr_on_wr_12( w_meta, w_addr, sz )
#define	mgr_on_commit()				mgr_on_commit_12()
#define	mgr_on_check()				mgr_on_check_12()
#define	print_version( f_out )			fprintf( f_out, "RAW_Pes_WAR_Pes_WAW_Pes_ABORT_READERS " )
#define	set_version( cd_v, cr_v )

#endif

#ifdef VERSION_21

#define	mgr_on_begin()				mgr_on_begin_all()
#define	mgr_on_rd( r_meta, r_addr, sz, r_res )	mgr_on_rd_21( r_meta, r_addr, sz, r_res )
#define	mgr_on_wr( w_meta, w_addr, sz )		mgr_on_wr_21( w_meta, w_addr, sz )
#define	mgr_on_commit()				mgr_on_commit_21()
#define	mgr_on_check()
#define	print_version( f_out )			fprintf( f_out, "RAW_Pes_WAR_Opt_WAW_Pes_WAITFOR_READERS " )
#define	set_version( cd_v, cr_v )

#endif

#ifdef VERSION_22

#define	mgr_on_begin()				mgr_on_begin_all()
#define	mgr_on_rd( r_meta, r_addr, sz, r_res )	mgr_on_rd_22( r_meta, r_addr, sz, r_res )
#define	mgr_on_wr( w_meta, w_addr, sz )		mgr_on_wr_22( w_meta, w_addr, sz )
#define	mgr_on_commit()				mgr_on_commit_22()
#define	mgr_on_check()				mgr_on_check_22()
#define	print_version( f_out )			fprintf( f_out, "RAW_Pes_WAR_Opt_WAW_Pes_ABORT_READERS " )
#define	set_version( cd_v, cr_v )

#endif


#ifdef VERSION_31

#define	mgr_on_begin()				mgr_on_begin_all()
#define	mgr_on_rd( r_meta, r_addr, sz, r_res )	mgr_on_rd_31( r_meta, r_addr, sz, r_res )
#define	mgr_on_wr( w_meta, w_addr, sz )		mgr_on_wr_31( w_meta, w_addr, sz )
#define	mgr_on_commit()				mgr_on_commit_31()
#define	mgr_on_check()
#define	print_version( f_out )			fprintf( f_out, "RAW_Opt_WAR_Opt_WAW_Pes_WAITFOR_READERS " )
#define	set_version( cd_v, cr_v )

#endif

#ifdef VERSION_32

#define	mgr_on_begin()				mgr_on_begin_all()
#define	mgr_on_rd( r_meta, r_addr, sz, r_res )	mgr_on_rd_32( r_meta, r_addr, sz, r_res )
#define	mgr_on_wr( w_meta, w_addr, sz )		mgr_on_wr_32( w_meta, w_addr, sz )
#define	mgr_on_commit()				mgr_on_commit_32()
#define	mgr_on_check()				mgr_on_check_32()
#define	print_version( f_out )			fprintf( f_out, "RAW_Opt_WAR_Opt_WAW_Pes_ABORT_READERS " )
#define	set_version( cd_v, cr_v )

#endif


#ifdef VERSION_41

#define	mgr_on_begin()				mgr_on_begin_all()
#define	mgr_on_rd( r_meta, r_addr, sz, r_res )	mgr_on_rd_41( r_meta, r_addr, sz, r_res )
#define	mgr_on_wr( w_meta, w_addr, sz )		mgr_on_wr_41( w_meta, w_addr, sz )
#define	mgr_on_commit()				mgr_on_commit_41()
#define	mgr_on_check()
#define	print_version( f_out )			fprintf( f_out, "RAW_Opt_WAR_Opt_WAW_Opt_WAITFOR_READERS " )
#define	set_version( cd_v, cr_v )

#endif

#ifdef VERSION_42

#define	mgr_on_begin()				mgr_on_begin_all()
#define	mgr_on_rd( r_meta, r_addr, sz, r_res )	mgr_on_rd_42( r_meta, r_addr, sz, r_res )
#define	mgr_on_wr( w_meta, w_addr, sz )		mgr_on_wr_42( w_meta, w_addr, sz )
#define	mgr_on_commit()				mgr_on_commit_42()
#define	mgr_on_check()				mgr_on_check_42()
#define	print_version( f_out )			fprintf( f_out, "RAW_Opt_WAR_Opt_WAW_Opt_ABORT_READERS " )
#define	set_version( cd_v, cr_v )

#endif


#if !defined(VERSION_DYNAMIC) && !defined(VERSION_00) && !defined(VERSION_11) && !defined(VERSION_12)							\
	 && !defined(VERSION_21) && !defined(VERSION_22) && !defined(VERSION_31) && !defined(VERSION_32) && !defined(VERSION_41)	\
	 && !defined(VERSION_42)
#error "VERSION undefined"
#endif


void	mgr_on_assign( ptr_t a_meta, ptr_t new_meta );
void	mgr_on_unassign (ptr_t a_meta);

#endif /*TM_VERS_H_*/
