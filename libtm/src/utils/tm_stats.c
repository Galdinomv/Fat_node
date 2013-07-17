#include "hrtime.h"

#include "tm_stats.h"
#include "../tm_mgr.h"


void cstats_init( cstats_t* csts )
{
	csts->n_commits = 0;
	csts->n_deadlocks_raw = 0;
	csts->n_deadlocks_war = 0;
	csts->n_deadlocks_waw = 0;
	csts->n_invalidations = 0;

	csts->n_reads = 0;
	csts->n_writes = 0;

	csts->t_commits = 0;
	csts->t_deadlocks_raw = 0;
	csts->t_deadlocks_war = 0;
	csts->t_deadlocks_waw = 0;
	csts->t_invalidations = 0;

	csts->t_waiting_raw = 0;
	csts->t_waiting_war = 0;
	csts->t_waiting_waw = 0;

	csts->begin = 0;
	csts->wt_raw = 0;
	csts->wt_war = 0;
	csts->wt_waw = 0;
}

void cstats_begin( cstats_t* csts )
{
	#ifdef TIMINGS
	csts->begin = get_c();
	#endif
}

void cstats_commit( cstats_t* csts, uint_t n_rds, uint_t n_wrs )
{
	csts->t_waiting_raw += csts->wt_raw;
	csts->t_waiting_war += csts->wt_war;
	csts->t_waiting_waw += csts->wt_waw;

	#ifdef TIMINGS
	csts->t_commits += ( get_c() - csts->begin - csts->wt_raw - csts->wt_war - csts->wt_waw );
	#endif

	csts->wt_raw = 0;
	csts->wt_war = 0;
	csts->wt_waw = 0;

	csts->n_reads  += n_rds;
	csts->n_writes += n_wrs;
	csts->n_commits++;
	
	//burceam: stats for Prof. Steffan to get num_reads and num_writes per committed transaction
	//should not be left in, normally.
	//fprintf (stderr, "%d %d\n", n_rds, n_wrs);
	//fflush (stderr);
	
	//burceam: end stats
	
}

void cstats_abort( cstats_t* csts, int invalidated, int reason )
{
	csts->t_waiting_raw += csts->wt_raw;
	csts->t_waiting_war += csts->wt_war;
	csts->t_waiting_waw += csts->wt_waw;

	unsigned long long abort_time = 0;
	#ifdef TIMINGS
	abort_time = ( get_c() - csts->begin - csts->wt_raw - csts->wt_war - csts->wt_waw );
	csts->begin = get_c();
	#endif

	csts->wt_raw = 0;
	csts->wt_war = 0;
	csts->wt_waw = 0;

	if( invalidated )
	{
		csts->n_invalidations++;
		csts->t_invalidations += abort_time;
	}
	else
	{
		if( reason == 0 )
		{
			csts->n_deadlocks_raw++;
			csts->t_deadlocks_raw += abort_time;
		}
		if( reason == 1 )
		{
			csts->n_deadlocks_war++;
			csts->t_deadlocks_war += abort_time;
		}
		if( reason == 2 )
		{
			csts->n_deadlocks_waw++;
			csts->t_deadlocks_waw += abort_time;
		}
	}
}


tstats_t stats_get( int _tid )
{
	tstats_t out;
	_tid = ((_tid == -1) ? (p_thread_id != -1 ? p_thread_id : 0) : _tid);

	out.n_commits 		= p_trans[ _tid ].stats.n_commits;
	out.n_deadlocks_raw	= p_trans[ _tid ].stats.n_deadlocks_raw;
	out.n_deadlocks_war	= p_trans[ _tid ].stats.n_deadlocks_war;
	out.n_deadlocks_waw	= p_trans[ _tid ].stats.n_deadlocks_waw;
	out.n_invalidations = p_trans[ _tid ].stats.n_invalidations;
	out.n_aborts = out.n_deadlocks_raw + out.n_deadlocks_war + out.n_deadlocks_waw + out.n_invalidations;

	out.n_reads 		= p_trans[ _tid ].stats.n_reads;
	out.n_writes 		= p_trans[ _tid ].stats.n_writes;

	out.t_commits 		= c_to_t( p_trans[ _tid ].stats.t_commits );
	out.t_deadlocks_raw	= c_to_t( p_trans[ _tid ].stats.t_deadlocks_raw );
	out.t_deadlocks_war	= c_to_t( p_trans[ _tid ].stats.t_deadlocks_war );
	out.t_deadlocks_waw	= c_to_t( p_trans[ _tid ].stats.t_deadlocks_waw );
	out.t_invalidations	= c_to_t( p_trans[ _tid ].stats.t_invalidations );
	out.t_aborts = out.t_deadlocks_raw + out.t_deadlocks_war + out.t_deadlocks_waw + out.t_invalidations;

	out.t_waiting_raw 	= c_to_t( p_trans[ _tid ].stats.t_waiting_raw );
	out.t_waiting_war	= c_to_t( p_trans[ _tid ].stats.t_waiting_war );
	out.t_waiting_waw	= c_to_t( p_trans[ _tid ].stats.t_waiting_waw );

	return out;
}

tstats_t stats_get_total()
{
	unsigned int i;
	tstats_t tot;

	tot.n_reads = 0;	tot.n_writes = 0;
	tot.n_commits = 0;	tot.n_invalidations = 0;	tot.n_aborts = 0;
	tot.t_commits = 0;	tot.t_invalidations = 0;	tot.t_aborts = 0;

	tot.n_deadlocks_raw = 0;	tot.n_deadlocks_war = 0;	tot.n_deadlocks_waw = 0;
	tot.t_deadlocks_raw = 0;	tot.t_deadlocks_war = 0;	tot.t_deadlocks_waw = 0;
	tot.t_waiting_raw = 0;		tot.t_waiting_war = 0;		tot.t_waiting_waw = 0;

	for( i = 0; i < __no_th; i++ )
	{
		tot.n_commits 		+= p_trans[ i ].stats.n_commits;
		tot.n_deadlocks_raw	+= p_trans[ i ].stats.n_deadlocks_raw;
		tot.n_deadlocks_war	+= p_trans[ i ].stats.n_deadlocks_war;
		tot.n_deadlocks_waw	+= p_trans[ i ].stats.n_deadlocks_waw;
		tot.n_invalidations += p_trans[ i ].stats.n_invalidations;

		tot.n_reads 		+= p_trans[ i ].stats.n_reads;
		tot.n_writes 		+= p_trans[ i ].stats.n_writes;

		tot.t_commits 		+= c_to_t( p_trans[ i ].stats.t_commits );
		tot.t_deadlocks_raw	+= c_to_t( p_trans[ i ].stats.t_deadlocks_raw );
		tot.t_deadlocks_war += c_to_t( p_trans[ i ].stats.t_deadlocks_war );
		tot.t_deadlocks_waw += c_to_t( p_trans[ i ].stats.t_deadlocks_waw );
		tot.t_invalidations += c_to_t( p_trans[ i ].stats.t_invalidations );

		tot.t_waiting_raw 	+= c_to_t( p_trans[ i ].stats.t_waiting_raw );
		tot.t_waiting_war 	+= c_to_t( p_trans[ i ].stats.t_waiting_war );
		tot.t_waiting_waw 	+= c_to_t( p_trans[ i ].stats.t_waiting_waw );
	}

	tot.n_aborts = tot.n_deadlocks_raw + tot.n_deadlocks_war + tot.n_deadlocks_waw + tot.n_invalidations;
	tot.t_aborts = tot.t_deadlocks_raw + tot.t_deadlocks_war + tot.t_deadlocks_waw + tot.t_invalidations;

	return tot;
}

void stats_print( FILE* f, tstats_t sts )
{
	fprintf( f, "%llu ", sts.n_commits );
#ifdef TIMINGS
	fprintf( f, "%.4lf  ", sts.t_commits );
#endif

	fprintf( f, "%llu ", sts.n_deadlocks_raw );
#ifdef TIMINGS
	fprintf( f, "%.4lf ", sts.t_deadlocks_raw );
	fprintf( f, "%.4lf  ", sts.t_waiting_raw );
#endif

	fprintf( f, "%llu ", sts.n_deadlocks_war );
#ifdef TIMINGS
	fprintf( f, "%.4lf ", sts.t_deadlocks_war );
	fprintf( f, "%.4lf  ", sts.t_waiting_war );
#endif

	fprintf( f, "%llu ", sts.n_deadlocks_waw );
#ifdef TIMINGS
	fprintf( f, "%.4lf ", sts.t_deadlocks_waw );
	fprintf( f, "%.4lf  ", sts.t_waiting_waw );
#endif

	fprintf( f, "%llu ", sts.n_invalidations );
#ifdef TIMINGS
	fprintf( f, "%.4lf  ", sts.t_invalidations );
#endif

	fprintf( f, "%llu ", sts.n_aborts );
#ifdef TIMINGS
	fprintf( f, "%.4lf  ", sts.t_aborts );
#endif

	double t_waiting = sts.t_waiting_raw + sts.t_waiting_war + sts.t_waiting_waw;
	fprintf( f, "%.4lf  ", t_waiting );
#ifdef TIMINGS
	fprintf( f, "%.4lf  ", sts.t_commits + sts.t_aborts + t_waiting );
#endif

	fprintf( f, "%.4lf ", ((double)(sts.n_aborts))/((double)(sts.n_commits + sts.n_aborts)) );
	fprintf( f, "%.4lf ", ((double)(sts.n_writes))/((double)(sts.n_reads + sts.n_writes)) );
}

void stats_print_v( FILE* f, tstats_t sts )
{
	fprintf( f, "N_CMT %llu ", sts.n_commits );
#ifdef TIMINGS
	fprintf( f, "T_CMT %.4lf  ", sts.t_commits );
#endif

	fprintf( f, "N_DLK_RAW %llu ", sts.n_deadlocks_raw );
#ifdef TIMINGS
	fprintf( f, "T_DLK_RAW %.4lf ", sts.t_deadlocks_raw );
	fprintf( f, "T_WT_RAW %.4lf  ", sts.t_waiting_raw );
#endif

	fprintf( f, "N_DLK_WAR %llu ", sts.n_deadlocks_war );
#ifdef TIMINGS
	fprintf( f, "T_DLK_WAR %.4lf ", sts.t_deadlocks_war );
	fprintf( f, "T_WT_WAR %.4lf  ", sts.t_waiting_war );
#endif

	fprintf( f, "N_DLK_WAW %llu ", sts.n_deadlocks_waw );
#ifdef TIMINGS
	fprintf( f, "T_DLK_WAW %.4lf ", sts.t_deadlocks_waw );
	fprintf( f, "T_WT_WAW %.4lf  ", sts.t_waiting_waw );
#endif

	fprintf( f, "N_INV %llu ", sts.n_invalidations );
#ifdef TIMINGS
	fprintf( f, "T_INV %.4lf  ", sts.t_invalidations );
#endif

	fprintf( f, "N_ABR %llu ", sts.n_aborts );
#ifdef TIMINGS
	fprintf( f, "T_ABR %.4lf  ", sts.t_aborts );
#endif

	double t_waiting = sts.t_waiting_raw + sts.t_waiting_war + sts.t_waiting_waw;
	fprintf( f, "T_WT_ALL %.4lf  ", t_waiting );
#ifdef TIMINGS
	fprintf( f, "T_ALL %.4lf  ", sts.t_commits + sts.t_aborts + t_waiting );
#endif

	fprintf( f, "AbRatio %.4lf ", ((double)(sts.n_aborts))/((double)(sts.n_commits + sts.n_aborts)) );
	fprintf( f, "WrRatio %.4lf ", ((double)(sts.n_writes))/((double)(sts.n_reads + sts.n_writes)) );
}

