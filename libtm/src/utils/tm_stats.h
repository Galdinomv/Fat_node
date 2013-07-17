#ifndef TM_STATS_H_
#define TM_STATS_H_

#include "../tm_general.h"

//#define TIMINGS

typedef struct _cstats_t
{
	volatile unsigned long long	n_commits;
	volatile unsigned long long	n_deadlocks_raw;
	volatile unsigned long long	n_deadlocks_war;
	volatile unsigned long long	n_deadlocks_waw;
	volatile unsigned long long	n_invalidations; // no. of aborts caused by invalidation = no of aborts - no of aborts caused by deadlocks

	volatile unsigned long long	n_reads;
	volatile unsigned long long	n_writes;

	volatile unsigned long long	t_commits;
	volatile unsigned long long	t_deadlocks_raw;
	volatile unsigned long long	t_deadlocks_war;
	volatile unsigned long long	t_deadlocks_waw;
	volatile unsigned long long	t_invalidations;

	volatile unsigned long long	t_waiting_raw;
	volatile unsigned long long	t_waiting_war;
	volatile unsigned long long	t_waiting_waw;

	volatile unsigned long long 	begin;
	volatile unsigned long long	wt_raw;
	volatile unsigned long long	wt_war;
	volatile unsigned long long	wt_waw;
} cstats_t;


typedef struct _tstats_t
{
	volatile unsigned long long	n_commits;
	volatile unsigned long long	n_deadlocks_raw;
	volatile unsigned long long	n_deadlocks_war;
	volatile unsigned long long	n_deadlocks_waw;
	volatile unsigned long long	n_invalidations; // no. of aborts caused by invalidation = no of aborts - no of aborts caused by deadlocks
	volatile unsigned long long	n_aborts;

	volatile unsigned long long	n_reads; // no of commited reads
	volatile unsigned long long	n_writes; // no of commited writes

	volatile double	t_commits;	// time spent doing useful work (executing transaction)
	volatile double	t_deadlocks_raw;
	volatile double	t_deadlocks_war;
	volatile double	t_deadlocks_waw;
	volatile double	t_invalidations; // time waisted as a result of invalidations
	volatile double	t_aborts;

	volatile double	t_waiting_raw;
	volatile double	t_waiting_war;
	volatile double	t_waiting_waw;
} tstats_t;


void cstats_init( cstats_t* csts );
void cstats_begin( cstats_t* csts );
void cstats_commit( cstats_t* csts, uint_t n_rds, uint_t n_wrs );
void cstats_abort( cstats_t* csts, int invalidated, int reason );

tstats_t stats_get( int _tid );
tstats_t stats_get_total();
void stats_print( FILE* f, tstats_t sts );
void stats_print_v( FILE* f, tstats_t sts );




#endif /*TM_STATS_H_*/
