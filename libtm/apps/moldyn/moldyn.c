/* Copyright (c) 1995 by Mark Hill, James Larus, and David Wood for
 * the Wisconsin Wind Tunnel Project and Joel Saltz for the High Performance
 * Software Laboratory, University of Maryland, College Park.
 *
 * ALL RIGHTS RESERVED.
 *
 * This software is furnished under a license and may be used and
 * copied only in accordance with the terms of such license and the
 * inclusion of the above copyright notice.  This software or any other
 * copies thereof or any derivative works may not be provided or
 * otherwise made available to any other persons.  Title to and ownership
 * of the software is retained by Mark Hill, James Larus, Joel Saltz, and
 * David Wood. Any use of this software must include the above copyright
 * notice.
 *
 * THIS SOFTWARE IS PROVIDED "AS IS".  THE LICENSOR MAKES NO
 * WARRANTIES ABOUT ITS CORRECTNESS OR PERFORMANCE.
 *
 * Shamik Sharma generated the sequential version in C.
 * Shubhendu Mukherjee (Shubu) parallelized moldyn on Tempest.
 */

/***********************************************************
 !  MITS - MARYLAND IRREGULAR TEST SET
 !
 !  Shamik D. Sharma, _OTHERS_
 !  Computer Science Department,
 !  University of Maryland,
 !  College Park, MD-20742
 !
 !  Contact: shamik@cs.umd.edu
 ************************************************************/

/************************************************************
 !File     : moldyn.c
 !Origin   : TCGMSG (Argonne), Shamik Sharma
 !Created  : Shamik Sharma,
 !Modified : Shamik Sharma,
 !Status   : Tested for BOXSIZE = 4,8,13
 !
 !Description :  Calculates the motion of  particles
 !               based on forces acting on each particle
 !               from particles within a certain radius.
 !
 !Contents : The main computation is in function main()
 !           The structure of the computation is as follows:
 !
 !     1. Initialise variables
 !     2. Initialise coordinates and velocities of molecules based on
 !          some distribution.
 !     3. Iterate for N time-steps
 !         3a. Update coordinates of molecule.
 !         3b. On Every xth iteration
 !             ReBuild the interaction-list. This list
 !             contains every pair of molecules which
 !             are within a cutoffSquare radius  of each other.
 !         3c. For each pair of molecule in the interaction list,
 !             Compute the force on each molecule, its velocity etc.
 !     4.  Using final velocities, compute KE and PE of system.
 !Usage:
 !      At command line, type :
 !      %  moldyn
 !
 !Input Data :
 !      The default setting simulates the dynamics with 8788
 !      particles. A smaller test-setting can be achieved by
 !      changing  BOXSIZE = 4.  To do this, change the #undef SMALL
 !      line below to #define SMALL. No other change is required.
 *************************************************************/

#include <assert.h>
#include <math.h>
#include <stdio.h>
#include <stdlib.h>

#include <sched.h> //for CPU_SET, sched_setaffinity()

/* TM_Modification_Begin */

/* TM include */
#include "../../src/tm.h"
#include "../../src/infra/barrier.h"
#include "../../src/infra/bitset.h"

/* TM_Modification_end */

#define LOCAL       /* Such a function that changes no global vars */
#define INPARAMS    /* All parameters following this are 'in' params  */

#define SQRT(a)  sqrt(a)
#define POW(a,b) pow(a,b)
#define SQR(a)   ((a)*(a))
#define DRAND(a)  drand_x(a)

/* TM_Modification_Begin */

/* TM Definitions */
//#define NTHREADS 4
#define NDIM 3

/* TM_Modification_End */

extern long random();
extern int srandom();

/*
 !======================  DATA-SETS  ======================================
 */

# ifdef  SMALL
#      define BOXSIZE                 4    /* creates 256 molecules */
#      define NUMBER_TIMESTEPS       30
#      define MAXINTERACT         32000    /* size of interaction array */
# elif defined(MEDIUM)
#      define BOXSIZE                 8
#      define NUMBER_TIMESTEPS       30
#      define MAXINTERACT        320000
# elif defined(SUPER)
#      define BOXSIZE                 20
#      define NUMBER_TIMESTEPS       30
#      define MAXINTERACT        38823808
# else
#      define BOXSIZE                13
#      define NUMBER_TIMESTEPS       30
#      define MAXINTERACT       1600000
# endif

#define NUM_PARTICLES      (4*BOXSIZE*BOXSIZE*BOXSIZE)
#define DENSITY            0.83134
#define TEMPERATURE        0.722
#define CUTOFF             3.5000
#define DEFAULT_TIMESTEP   0.064
#define SCALE_TIMESTEP     4
#define TOLERANCE          1.2

#define DIMSIZE NUM_PARTICLES
#define DSIZE   2
#define INDX(aa,bb)  (((aa)*DSIZE) + (bb))    /* used to index inter  */
#define IND(aa,bb)   ((aa)*DIMSIZE + (bb))    /* used to index x,f,vh */
#define MIN(a,b)     (((a)<(b))?(a):(b))

/*
 !======================  GLOBAL ARRAYS ======================================
 !
 ! Note : inter is usually the biggest array. If BOXSIZE = 13, it will
 !        cause 1 million interactions to be generated. This will need
 !        a minimum of 80 MBytes to hold 'inter'. The other
 !        arrays will need about a sum of around 800 KBytes. Note
 !        that MAXINTERACT may be defined to a more safe value causing
 !        extra memory to be allocated. (~ 130 MBytes !)
 !============================================================================
 */

/* TM_Modification_Begin */

//burceam: changed these to be static, with same dims as for the TM case
//these are all shared variables that we'll need to protect in the
//non-STM version.

//double  *x;     /* x,y,z coordinates of each molecule */
//double  *vh;    /* partial x,y,z velocity of molecule */
//double  *f;     /* partial forces on each molecule    */
//int     *inter; /* pairs of interacting molecules     */

double x[NUM_PARTICLES * NDIM];
double f[NUM_PARTICLES * NDIM];
double vh[NUM_PARTICLES * NDIM];
int inter[MAXINTERACT * DSIZE];

#ifdef MEASURE
int connect[NUM_PARTICLES];
#endif

/* TM_Modification_End */


/*
 !======================  GLOBAL VARIABLES ===================================
 */

double   side;                  /*  length of side of box                 */
double   sideHalf;              /*  1/2 of side                           */
double   cutoffRadius;          /*  cuttoff distance for interactions     */
int      neighUpdate;           /*  timesteps between interaction updates */
double   perturb;               /*  perturbs initial coordinates          */

double   timeStep;              /*  length of each timestep   */
double   timeStepSq;            /*  square of timestep        */
double   timeStepSqHalf;        /*  1/2 of square of timestep */

int      numMoles;              /*  number of molecules                   */
int      ninter;                /*  number of interacting molecules pairs  */
double   vaver;                 /*                                        */

double   epot;                  /*  The potential energy      */
double   vir;                   /*  The virial  energy        */
double   count, vel ;
double   ekin;

/* TM Variables */
int      n3;
barrier_t barrier;
//mutex to protect global scalars in ComputeForces(): vir, epot
pthread_mutex_t global_mutex1 = PTHREAD_MUTEX_INITIALIZER;
//mutex to protect global scalars in ComputeKEVel(): count, vel, ekin
pthread_mutex_t global_mutex2 = PTHREAD_MUTEX_INITIALIZER;
//mutex to protect global scalar ninter and global array inter
pthread_mutex_t mutex_ninter = PTHREAD_MUTEX_INITIALIZER;

//mutex protection for x and f
pthread_mutex_t mutex_x = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t mutex_f = PTHREAD_MUTEX_INITIALIZER;

//number of neighbours for each molecule
int num_neighbours [NUM_PARTICLES];
//counters for the f and x arrays; it would be more efficient to combine
//them into a single array; maybe long long, like below, and use a ton of macros
//to access each of its components.
int counter_f [NUM_PARTICLES];
int counter_x [NUM_PARTICLES];

//in each element, most significant 4 bytes are counter for f,
//least significant 4 bytes are counter for x.
//long long counter [NUM_PARTICLES];

//#define COUNTER_MASK_FOR_F 0xFFFFFFFF00000000
//#define COUNTER_MASK_FOR_X 0x00000000FFFFFFFF
//#define READ_COUNTER_F (i) (counter[i] & COUNTER_MASK_FOR_F)
//#define READ_COUNTER_X (i) (counter[i] & COUNTER_MASK_FOR_X)
//#define WRITE_COUNTER_F (i, value) (counter[i] = (value << 32) & COUNTER_MASK_FOR_F)
//#define WRITE_COUNTER_X (i, value) (counter[i] = (value)       & COUNTER_MASK_FOR_X)
//actually, I should use the atomic inc stuff, and some atomic write...

int NTHREADS;

/* Statistics Variavles */
double load_bal[4][30][8];
struct timeval tim2[4][30][18];  
//struct timeval tim;

/* ----------- UTILITY ROUTINES --------------------------- */

/*
 !=============================================================
 !  Function : drand_x()
 !  Purpose  :
 !    Used to calculate the distance between two molecules
 !    given their coordinates.
 !=============================================================
 */
LOCAL double drand_x(double x)
{
    double tmp = ( (double) random() ) * 4.6566128752458e-10;
#ifdef PRINT_RANDS
    printf("%lf\n", tmp);
#endif
    return tmp;
}

/*
 !=============================================================
 !  Function : Foo()
 !  Purpose  :
 !    Used to calculate the distance between two molecules
 !    given their coordinates.
 !=============================================================
 */
LOCAL double Foo(double xi, double yi, double zi, double xj, double yj, double zj)
{
    double xx, yy, zz, rd;
    
    xx = xi - xj;
    yy = yi - yj;
    zz = zi - zj;
    if ( xx < -sideHalf ) xx += side ;
    if ( yy < -sideHalf ) yy += side ;
    if ( zz < -sideHalf ) zz += side ;
    if ( xx >  sideHalf ) xx -= side ;
    if ( yy >  sideHalf ) yy -= side ;
    if ( zz >  sideHalf ) zz -= side ;
    rd = xx*xx + yy*yy + zz*zz ;
    return (rd);
}


/*---------- INITIALIZATION ROUTINES HERE               ------------------*/


/*
 !============================================================================
 !  Function :  InitSettings()
 !  Purpose  :
 !     This routine sets up the global variables
 !============================================================================
 */

int InitSettings()
{
    numMoles  = 4*BOXSIZE*BOXSIZE*BOXSIZE;
    
    side   = POW( ((double)(numMoles)/DENSITY), 0.3333333);
    sideHalf  = side * 0.5 ;
    
    cutoffRadius  = MIN(CUTOFF, sideHalf );
    
    timeStep      = DEFAULT_TIMESTEP/SCALE_TIMESTEP ;
    timeStepSq    = timeStep   * timeStep ;
    timeStepSqHalf= timeStepSq * 0.5 ;
    
    neighUpdate   = 10*(1+SCALE_TIMESTEP/4);
    perturb       = side/ (double)BOXSIZE;     /* used in InitCoordinates */
    vaver         = 1.13 * SQRT(TEMPERATURE/24.0);
    
    n3 = numMoles * 3;
    
#if (!defined(PRINT_COORDINATES) && !defined(PRINT_INTERACTION_LIST))
    fprintf(stdout,"----------------------------------------------------");
    fprintf(stdout,"\n MolDyn - A Simple Molecular Dynamics simulation \n");
    fprintf(stdout,"----------------------------------------------------");
    fprintf(stdout,"\n number of particles is ......... %6d", numMoles);
    fprintf(stdout,"\n side length of the box is ...... %13.6le",side);
    fprintf(stdout,"\n cut off radius is .............. %13.6le",cutoffRadius);
    fprintf(stdout,"\n temperature is ................. %13.6le",TEMPERATURE);
    fprintf(stdout,"\n time step is ................... %13.6le",timeStep);
    fprintf(stdout,"\n interaction-list updated every..   %d steps",neighUpdate);
    fprintf(stdout,"\n total no. of steps .............   %d ",NUMBER_TIMESTEPS);
    fprintf(stdout,
            "\n TimeStep   K.E.        P.E.        Energy    Temp.     Pres.    Vel.    rp ");
    fprintf(stdout,
            "\n -------- --------   ----------   ----------  -------  -------  ------  ------");
#endif
}


/*
 !============================================================================
 !  Function : InitCoordinates()
 !  Purpose  :
 !     Initialises the coordinates of the molecules by
 !     distribuuting them uniformly over the entire box
 !     with slight perturbations.
 !============================================================================
 */

void InitCoordinates(int numMoles, int siz, double perturb)
{
    int n, k, ij,  j, i, npoints;
    double tmp = 0;
    
    npoints = siz * siz * siz ;
    for ( n =0; n< npoints; n++) {
        k   = n % siz ;
        j   = (int)((n-k)/siz) % siz;
        i   = (int)((n - k - j*siz)/(siz*siz)) % siz ;
        
        x[IND(0,n)] = i*perturb ;
        x[IND(1,n)] = j*perturb ;
        x[IND(2,n)] = k*perturb ;
        
        x[IND(0,n+npoints)] = i*perturb + perturb * 0.5 ;
        x[IND(1,n+npoints)] = j*perturb + perturb * 0.5;
        x[IND(2,n+npoints)] = k*perturb ;
        
        x[IND(0,n+npoints*2)] = i*perturb + perturb * 0.5 ;
        x[IND(1,n+npoints*2)] = j*perturb ;
        x[IND(2,n+npoints*2)] = k*perturb + perturb * 0.5;
        
        x[IND(0,n+npoints*3)] = i*perturb ;
        x[IND(1,n+npoints*3)] = j*perturb + perturb * 0.5 ;
        x[IND(2,n+npoints*3)] = k*perturb + perturb * 0.5;
    }
}

/*
 !============================================================================
 ! Function  :  InitVelocities()
 ! Purpose   :
 !    This routine initializes the velocities of the
 !    molecules according to a maxwellian distribution.
 !============================================================================
 */

int  InitVelocities(double h)
{
    int i, j, k, nmoles1, nmoles2, iseed;
    double ts, sp, sc, r, s;
    double u1, u2, v1, v2, ujunk,tscale;
    double DRAND(double);
    
    iseed = 4711;
    ujunk = DRAND(iseed);
    iseed = 0;
    tscale = (16.0)/(1.0*numMoles - 1.0);
    
    for ( i =0; i< n3; i=i+2) {
        do {
            u1 = DRAND(iseed);
            u2 = DRAND(iseed);
            v1 = 2.0 * u1   - 1.0;
            v2 = 2.0 * u2   - 1.0;
            s  = v1*v1  + v2*v2 ;
        } while( s >= 1.0 );
        
        r = SQRT( -2.0*log(s)/s );
        vh[i]    = v1 * r;
        vh[i+1]  = v2 * r;
    }
    
    
    
    /* There are three parts - repeat for each part */
    nmoles1 = n3/3 ;
    nmoles2 = nmoles1 * 2;
    
    /*  Find the average speed  for the 1st part */
    sp   = 0.0 ;
    for ( i=0; i<nmoles1; i++) {
        sp = sp + vh[i];
    }
    sp   = sp/nmoles1;
    
    
    /*  Subtract average from all velocities of 1st part*/
    for ( i=0; i<nmoles1; i++) {
        vh[i] = vh[i] - sp;
    }
    
    /*  Find the average speed for 2nd part*/
    sp   = 0.0 ;
    for ( i=nmoles1; i<nmoles2; i++) {
        sp = sp + vh[i];
    }
    sp   = sp/(nmoles2-nmoles1);
    
    /*  Subtract average from all velocities of 2nd part */
    for ( i=nmoles1; i<nmoles2; i++) {
        vh[i] = vh[i] - sp;
    }
    
    /*  Find the average speed for 2nd part*/
    sp   = 0.0 ;
    for ( i=nmoles2; i<n3; i++) {
        sp = sp + vh[i];
    }
    sp   = sp/(n3-nmoles2);
    
    /*  Subtract average from all velocities of 2nd part */
    for ( i=nmoles2; i<n3; i++) {
        vh[i] = vh[i] - sp;
    }
    
    /*  Determine total kinetic energy  */
    ekin = 0.0 ;
    for ( i=0 ; i< n3; i++ ) {
        ekin  = ekin  + vh[i]*vh[i] ;
    }
    ts = tscale * ekin ;
    sc = h * SQRT(TEMPERATURE/ts);
    for ( i=0; i< n3; i++) {
        vh[i] = vh[i] * sc ;
    }
}

/*
 !============================================================================
 !  Function :  InitForces()
 !  Purpose :
 !    Initialize all the partial forces to 0.0
 !============================================================================
 */

int  InitForces()
{
    int i;
    
    for ( i=0; i<n3; i++ ) {
        f[i] = 0.0 ;
    }
}

int FirstCoordinates()
{
	int i;
	for ( i = 0; i < n3; i ++) {
		x[i] = x[i] + vh[i];
		if ( x[i] < 0.0 )    x[i] = x[i] + side ;
		if ( x[i] > side   ) x[i] = x[i] - side ;
	}
}

/*---------- UPDATE ROUTINES HERE               ------------------*/

/*
 !============================================================================
 !  Function :  UpdateCoordinates()
 !  Purpose  :
 !     This routine moves the molecules based on
 !     forces acting on them and their velocities
 !============================================================================
 */
/*
 int UpdateCoordinates( void* arg, int id )
 {
 int i, _tmp;
 
 //we can leave static scheduling here, it'll be sufficient;
 //no need for dynamic scheduling, workloads are identical for all threads.
 
 for ( i = id; i < n3; i += NTHREADS)
 //for ( i = 0; i < n3; i ++) // single threaded version
 {
 //BEGIN_TRANSACTION();
 //if (id == 0) {
 
 //spin until counter_f[i] is set by ComputeForces;
 //note that the read is not atomic, but the write to counter_f[i] _is_ atomic
 //(in ComputeForces).
 _tmp = num_neighbours [i];
 while (counter_f[i] != _tmp);
 
 //must be atomic
 //should this be at the very end of the transaction?
 //don't think so, as long as it's _before_ incrementing counter_x
 set_mb (&counter_f[i], 0);
 
 x[i] = x[i] + vh[i] + f[i];
 if ( x[i] < 0.0 )    x[i] = x[i] + side ;
 if ( x[i] > side   ) x[i] = x[i] - side ;
 vh[i] = vh[i] + f[i];
 f[i]  = 0.0;
 
 //must be atomic
 //this _must_ be the last thing in this "transaction" (as once we set it,
 //ComputeForces thread can start messing with data)
 atomic_inc (counter_x[i]);
 
 //COMMIT_TRANSACTION();
 //}
 }
 
 return 0;
 }
 */

void PrintCoordinates(INPARAMS int numMoles)
{
    int i, j;
    printf("%d\n", numMoles);
    for (i=0;i<numMoles;i++)
    {
        printf("%f,%f,%f\n", (double)x[IND(0,i)], (double)x[IND(1,i)],(double)x[IND(2,i)]);
    }
}

/*
 !============================================================================
 !  Function :  BuildNeigh()
 !  Purpose  :
 !     This routine is called after every x timesteps
 !     to  rebuild the list of interacting molecules
 !     Note that molecules within cutoffRad+TOLERANCE
 !     are included. This tolerance is in order to allow
 !     for molecules that might move within range
 !     during the computation.
 !============================================================================
 */

//we should use dynamic scheduling here
int BuildNeigh( void* arg, int id )
{
    double rd, cutoffSquare;
    int    i,j;
    //double Foo();
    
    cutoffSquare  = (cutoffRadius * TOLERANCE)*(cutoffRadius * TOLERANCE);
    
    for (i = id; i < numMoles; i += NTHREADS) {
        num_neighbours [i] = 0;
        //resetting these two as well
        counter_f [i] = 0;
        counter_x [i] = 0;
    }
    
    //for ( i=0; i<numMoles; i++) //single-threaded version
    for ( i=id; i<numMoles; i+=NTHREADS)
    {
        //BEGIN_TRANSACTION();
        //if (id == 0) {
        
        for ( j = i+1; j<numMoles; j++ )
        {
            //x is global var, would need exclusive access unless this method is surrounded
            //by barriers, because while x is only read here (and not written), other methods
            //do write to x.
            rd = Foo ( (double)x[IND(0,i)], (double)x[IND(1,i)], (double)x[IND(2,i)],
                      (double)x[IND(0,j)], (double)x[IND(1,j)], (double)x[IND(2,j)]);
            if ( rd <= cutoffSquare)
            {
                //inter[] and ninter are global vars, need exclusive access
                //do this more efficiently? atomic read
                pthread_mutex_lock (&mutex_ninter);
                inter[INDX(ninter,0)] = i;
                inter[INDX(ninter,1)] = j;
                //make sure atomic_inc works properly, try pthread_mutex_lock to double-check
                //atomic_inc (&ninter);
                ninter ++;
                num_neighbours [i] ++;
                num_neighbours [j] ++;
                pthread_mutex_unlock (&mutex_ninter);
                //this is a useless statement
                if ( ninter >= MAXINTERACT) perror("MAXINTERACT limit");
            }
        }
        
        //COMMIT_TRANSACTION();
        //}
    }
    
    return 0;
}

void PrintInteractionList(INPARAMS int ninter)
{
    int i;
    printf("%d\n", ninter);
    for (i=0;i<ninter;i++)
    {
        printf("%d %d\n", (int)inter[INDX(i,0)], (int)inter[INDX(i,1)]);
    }
}

#ifdef MEASURE
void PrintConnectivity()
{
    int ii, i;
    int min, max;
    float sum, sumsq, stdev, avg;
    
    bzero((char *)connect, sizeof(int) * NUM_PARTICLES);
    
    for (ii=0;ii<ninter;ii++)
    {
        assert(inter[INDX(ii,0)] < NUM_PARTICLES);
        assert(inter[INDX(ii,1)] < NUM_PARTICLES);
        
        connect[inter[INDX(ii,0)]]++;
        connect[inter[INDX(ii,1)]]++;
    }
    
    sum = 0.0;
    sumsq = 0.0;
    
    sum = connect[0];
    sumsq = SQR(connect[0]);
    min = connect[0];
    max = connect[0];
    for (i=1;i<NUM_PARTICLES;i++)
    {
        sum += connect[i];
        sumsq += SQR(connect[i]);
        if (min > connect[i])
            min = connect[i];
        if (max < connect[i])
            max = connect[i];
    }
    
    avg = sum / NUM_PARTICLES;
    stdev = sqrt((sumsq / NUM_PARTICLES) - SQR(avg));
    
    printf("avg = %4.1lf, dev = %4.1lf, min = %d, max = %d\n",
           avg, stdev, min, max);
    
}
#endif

/*
 !============================================================================
 ! Function :  ComputeForces
 ! Purpose  :
 !   This is the most compute-intensive portion.
 !   The routine iterates over all interacting  pairs
 !   of molecules and checks if they are still within
 !   inteacting range. If they are, the force on
 !   each  molecule due to the other is calculated.
 !   The net potential energy and the net virial
 !   energy is also computed.
 !============================================================================
 */

//we should use dynamic scheduling here
int ComputeForces( void* arg, int id )
{
    double cutoffSquare;
    double xx, yy, zz, rd, rrd, rrd2, rrd3, rrd4, rrd5, rrd6, rrd7, r148;
    double forcex, forcey, forcez;
    int    i,j,ii;
    
    double vir_tmp = 0;
    double epot_tmp = 0;
    
    cutoffSquare = cutoffRadius*cutoffRadius ;
    
    //ninter is global var, needs exclusive access unless this method is protected
    //by barriers from methods that modify ninter.
    //Similarly, x and inter are global vars, and need exclusive access; they are also
    //a reason why this method needs to be protected by barriers from methods that
    //modify x and inter; specifically, you do not want some threads executing some
    //other method that modifies x or inter or ninter, and some threads executing this
    //method, where these variables are read.
    
    //keep in mind you need to protect the shared variables accessed in here not only
    //from threads executing THIS function (parallel region), but also from threads
    //executing OTHER parallel regions, which may modify these shared variables;
    
    //for(ii=0; ii<ninter; ii++) { //single-threaded version
    for(ii=id; ii<ninter; ii+=NTHREADS) {
        
        //BEGIN_TRANSACTION();
        //if (id == 0) {
        
        i = inter[INDX(ii,0)];
        j = inter[INDX(ii,1)];
        
        //we're waiting on x[] to be updated for _both_ molecules (i and j)
        //by UpdateCoordinates in the previous timestep.
        //spin until coords for molecules i and j are updated (by UpdateCoordinates).
        //while ((counter_x[i] != 1) && (counter_x[j] != 1));
        
        //reset the counters for x for molecules i and j;
        //must be atomic writes
        //should this be at the end of the transaction?...
        //set_mb (&counter_x[i], 0);
        //set_mb (&counter_x[j], 0);
        
        xx = x[IND(0,i)] - x[IND(0,j)];
        yy = x[IND(1,i)] - x[IND(1,j)];
        zz = x[IND(2,i)] - x[IND(2,j)];
        
        if (xx < -sideHalf) xx += side;
        if (yy < -sideHalf) yy += side;
        if (zz < -sideHalf) zz += side;
        if (xx > sideHalf) xx -= side;
        if (yy > sideHalf) yy -= side;
        if (zz > sideHalf) zz -= side;
        rd = (xx*xx + yy*yy + zz*zz);
        if ( rd < cutoffSquare ) {
            rrd   = 1.0/rd;
            rrd2  = rrd*rrd ;
            rrd3  = rrd2*rrd ;
            rrd4  = rrd2*rrd2 ;
            rrd6  = rrd2*rrd4;
            rrd7  = rrd6*rrd ;
            r148  = rrd7 - 0.5 * rrd4 ;
            
            forcex = xx*r148;
            forcey = yy*r148;
            forcez = zz*r148;
            
            f[IND(0,i)]  += forcex ;
            f[IND(1,i)]  += forcey ;
            f[IND(2,i)]  += forcez ;
            
            f[IND(0,j)]  -= forcex ;
            f[IND(1,j)]  -= forcey ;
            f[IND(2,j)]  -= forcez ;
            
            //global vars, need exclusive access
            
            vir_tmp += rd*r148 ;
            epot_tmp += (rrd6 - rrd3);
            
            /*
             pthread_mutex_lock (&global_mutex1);
             vir -= rd*r148 ;
             epot += (rrd6 - rrd3);
             pthread_mutex_unlock (&global_mutex1);
             
             */
            //atomically increment the counter_f for both molecules i and j;
            //these _must_ be the last thing in this "transaction";
            //assumes we'll only have 2 txns, one in ComputeForces, one in UpdateCoordinates;
            //if we have more, then you may need to put these under the previous mutex as well.
            //atomic_inc(&counter_f[i]);
            //atomic_inc(&counter_f[j]);
        }
        
        
        
        //COMMIT_TRANSACTION();
        //}
    }
    
    pthread_mutex_lock (&global_mutex1);
    vir -= vir_tmp;
    epot += epot_tmp;
    pthread_mutex_unlock (&global_mutex1);
    
    return 0;
}

/*
 !============================================================================
 !  Function : Update
 !  Purpose  :
 !       Updates the everything
 !============================================================================
 */

//we should use dynamic scheduling here
int Update( void* arg, int id )
{
    int i,j, _tmp;
    
    double vaverh, sq;
    
    double sum = 0;
    double counter, velocity;
    
    double force_x, force_y, force_z;
    double velocity_x, velocity_y, velocity_z;
    double velocity_x_sq, velocity_y_sq, velocity_z_sq;
    
    vaverh = vaver * timeStep ;
    
    //keep in mind you need to protect the shared variables accessed here not
    //only from threads executing THIS function (parallel region), but also from
    //threads executing OTHER parallel regions, which may modify these
    //shared variables.
    
    //for ( i = 0; i < numMoles; i ++) //single-threaded version
    for ( i = id; i < numMoles; i += NTHREADS)
    {
        // Compute Velocity
        
        force_x  = f[IND(0,i)] * timeStepSqHalf ;
        force_y  = f[IND(1,i)] * timeStepSqHalf ;
        force_z  = f[IND(2,i)] * timeStepSqHalf ;
        
        velocity_x  = vh[ IND(0,i) ] + force_x ;
        velocity_y  = vh[ IND(1,i) ] + force_y ;
        velocity_z  = vh[ IND(2,i) ] + force_z ;
        
        velocity_x_sq = SQR(velocity_x);
        velocity_y_sq = SQR(velocity_y);
        velocity_z_sq = SQR(velocity_z);
        
        
        // Compute KEVel
        
        sum += velocity_x_sq;
        sum += velocity_y_sq;
        sum += velocity_z_sq;
        
        sq = SQRT(  velocity_x_sq + velocity_y_sq + velocity_z_sq );
        
        if ( sq > vaverh ) counter += 1.0 ;
        
        velocity += sq ;
        
        // Update Coordinates
        //_tmp = num_neighbours [i];
        //while (counter_f[i] != _tmp);
        
        //must be atomic
        //should this be at the very end of the transaction?
        //don't think so, as long as it's _before_ incrementing counter_x
        //set_mb (&counter_f[i], 0);
        
        // velocity_* is already vh[ * ] + forcex ;
        vh[IND(0,i)] = velocity_x + force_x;
        vh[IND(1,i)] = velocity_y + force_y;
        vh[IND(2,i)] = velocity_z + force_z;
        
        //protect x from readers in ComputeForces
        // pthread_mutex_lock (&mutex_x);
        
        x[IND(0,i)] += velocity_x + force_x;
        x[IND(1,i)] += velocity_y + force_y;
        x[IND(2,i)] += velocity_z + force_z;
        
        for (j = 0; j < 3; ++j) {
            if ( x[IND(j, i)] < 0.0 )    x[IND(j, i)] += side;
            if ( x[IND(j, i)] > side   ) x[IND(j, i)] -= side;
        }
        // pthread_mutex_unlock (&mutex_x);
        
        //pthread_mutex_lock (&mutex_f);
        //Reset all forces to 0
        for (j = 0; j < 3; ++j) {
            f[IND(j, i)]  = 0.0;
        }
        //pthread_mutex_unlock (&mutex_f);
        
        //must be atomic
        //this _must_ be the last thing in this "transaction" (as once we set it,
        //ComputeForces thread can start messing with data)
        //atomic_inc(&counter_x[i]);
        
        //COMMIT_TRANSACTION();
        //}
    }
    
    //global vars, need exclusive access
    pthread_mutex_lock (&global_mutex2);
    ekin += sum/timeStepSq;
    vel += velocity/timeStep;
    count += counter;
    pthread_mutex_unlock (&global_mutex2);
    
    return 0;
}

/*
 !============================================================================
 !  Function : UpdateVelocities
 !  Purpose  :
 !       Updates the velocites to take into account the
 !       new forces between interacting molecules
 !============================================================================
 */

/*
 //we should use dynamic scheduling here
 int UpdateVelocities( void* arg, int id )
 {
 int i;
 
 //keep in mind you need to protect the shared variables accessed here not
 //only from threads executing THIS function (parallel region), but also from
 //threads executing OTHER parallel regions, which may modify these
 //shared variables.
 
 //for ( i = 0; i < numMoles; i ++) //single-threaded version
 for ( i = id; i < numMoles; i += NTHREADS)
 {
 
 //BEGIN_TRANSACTION();
 //if (id == 0) {
 
 //f[IND(0,i)]  = f[IND(0,i)] * timeStepSqHalf ;
 //f[IND(1,i)]  = f[IND(1,i)] * timeStepSqHalf ;
 //f[IND(2,i)]  = f[IND(2,i)] * timeStepSqHalf ;
 
 //vh[ IND(0,i) ] += f[ IND(0,i) ];
 //vh[ IND(1,i) ] += f[ IND(1,i) ];
 //vh[ IND(2,i) ] += f[ IND(2,i) ];
 
 //burceam: optimize the above 6 lines of code to eliminate some dependences
 vh[ IND(0,i) ] += f[ IND(0,i) ] * timeStepSqHalf ;
 vh[ IND(1,i) ] += f[ IND(1,i) ] * timeStepSqHalf ;
 vh[ IND(2,i) ] += f[ IND(2,i) ] * timeStepSqHalf ;
 
 
 //COMMIT_TRANSACTION();
 //}
 }
 
 return 0;
 }
 */

/*
 !============================================================================
 !  Function :  ComputeKE()
 !  Purpose :
 !     Computes  the KE of the system by summing all
 !     the squares of the partial velocities.
 !============================================================================
 */

/*
 //we should use dynamic scheduling here
 int ComputeKEVel( void * arg, int id)
 {
 double sum = 0.0;
 double vaverh, velocity, counter, sq;
 
 int    i;
 
 //keep in mind you need to protect the shared variables read in here (vh)
 //not only from threads executing THIS function (parallel region), but also
 //from threads executing OTHER parallel regions, which may modify these
 //shared variables.
 
 vaverh = vaver * timeStep ;
 velocity    = 0.0 ;
 counter     = 0.0 ;
 
 //if (id != 0)
 //   return 0;
 //for (i = 0; i< numMoles; i++)
 for (i = id; i< numMoles; i+=NTHREADS)
 {
 sum = sum + vh[ IND(0,i) ] * vh[ IND(0,i) ];
 sum = sum + vh[ IND(1,i) ] * vh[ IND(1,i) ];
 sum = sum + vh[ IND(2,i) ] * vh[ IND(2,i) ];
 
 sq = SQRT(  SQR(vh[IND(0,i)]) + SQR(vh[IND(1,i)]) +
 SQR(vh[IND(2,i)])  );
 
 if ( sq > vaverh ) counter += 1.0 ;
 
 velocity += sq ;
 }
 
 //BEGIN_TRANSACTION();
 
 //global vars, need exclusive access
 pthread_mutex_lock (&global_mutex2);
 ekin += sum/timeStepSq;
 vel += velocity/timeStep;
 count += counter;
 pthread_mutex_unlock (&global_mutex2);
 
 //COMMIT_TRANSACTION();
 
 return 0;
 }
 */

/*
 !=============================================================
 !  Function : PrintResults()
 !  Purpose  :
 !    Prints out the KE, the PE and other results
 !=============================================================
 */

LOCAL int PrintResults(int move, double ekin, double epot,  double vir, double vel, double count, int numMoles, int ninteracts)
{
    double ek, etot, temp, pres, rp, tscale ;
    
    ek   = 24.0 * ekin ;
    epot = 4.00 * epot ;
    etot = ek + epot ;
    tscale = (16.0)/((double)numMoles - 1.0);
    temp = tscale * ekin ;
    pres = DENSITY * 16.0 * (ekin-vir)/numMoles ;
    vel  = vel/numMoles;
    rp   = (count/(double)(numMoles)) * 100.0 ;
    
    fprintf(stdout,
            "\n %4d %12.4lf %12.4lf %12.4lf %8.4lf %8.4lf %8.4lf %5.1lf",
            move, ek,    epot,   etot,   temp,   pres,   vel,     rp);
#ifdef DEBUG
    fprintf(stdout,"\n\n In the final step there were %d interacting pairs\n", ninteracts);
#endif
}

void dump_values(char *s)
{
    int i;
    printf("\n%s\n", s);
    for (i=0;i<n3/3;i++)
    {
        printf("%d: coord = (%lf, %lf, %lf), vel = (%lf, %lf, %lf), force = (%lf, %lf, %lf)\n",
               i, (double)x[IND(0,i)], (double)x[IND(1,i)], (double)x[IND(2,i)],
               (double)vh[IND(0,i)], (double)vh[IND(1,i)], (double)vh[IND(2,i)],
               (double)f[IND(0,i)], (double)f[IND(1,i)], (double)f[IND(2,i)]);
    }
}


#ifdef coredump
#define WITH_SMT
#endif

//main work function for each thread
void * do_thread_work (void * _id) {
    
    int id = (int) _id;
    int tstep, i;
    
    cpu_set_t mask;
    CPU_ZERO( &mask );
#ifdef WITH_SMT
	CPU_SET( id*2, &mask );
#else
	CPU_SET( id, &mask );
#endif
    sched_setaffinity(0, sizeof(mask), &mask);
    
    for ( tstep=0; tstep< NUMBER_TIMESTEPS; tstep++)
    {

        gettimeofday(&tim2[id][tstep][0], NULL);

        barrier_wait (&barrier);

        gettimeofday(&tim2[id][tstep][1], NULL);

        //need to reset these global vars in the beginning of every timestep
        //global vars, need to be protected
        if (id == 0) {
            vir  = 0.0;
            epot = 0.0;
            ekin = 0.0;
            vel = 0.0;
            count = 0.0;
        }
        
        //because we have this barrier here, we don't need one at the end of
        //each timestep; this barrier effectively acts as if it was at the end;
        //we need it here because we need to perform the initializations above
        //at the _beginning_ of each timestep.
        //No, you need a barrier at the end, as well, because if you only have one
        //here but not at the end, you may reset some of the above global vars
        //while a slower thread is still computing stuff in the previous timestep...
        //I'm not sure, but we may be able to replace this barrier with just
        //asm volatile("mfence" ::: "memory");

        gettimeofday(&tim2[id][tstep][2], NULL);

        barrier_wait (&barrier);

        gettimeofday(&tim2[id][tstep][3], NULL);

        //UpdateCoordinates (NULL, id);
        //PARALLEL_EXECUTE( NTHREADS, UpdateCoordinates, NULL );
        //barrier_wait (&barrier);
        
        if ( tstep % neighUpdate == 0)
        {

          gettimeofday(&tim2[id][tstep][4], NULL);

          //this barrier needed because of the single-threaded code below
          barrier_wait (&barrier);
            
          gettimeofday(&tim2[id][tstep][5], NULL);

            if (id == 0) {
            #ifdef PRINT_COORDINATES
                PrintCoordinates(numMoles);
            #endif
                
                //global var, needs to be protected
                ninter = 0;
            }
            

            gettimeofday(&tim2[id][tstep][6], NULL);

            //this barrier needed because of the single-threaded code above
            barrier_wait (&barrier);
            
           gettimeofday(&tim2[id][tstep][7], NULL);
            
            BuildNeigh (NULL, id);
            //PARALLEL_EXECUTE( NTHREADS, BuildNeigh, NULL );
            
            
            gettimeofday(&tim2[id][tstep][8], NULL);

            //this barrier needed because of the single-threaded code below
            barrier_wait (&barrier);

            gettimeofday(&tim2[id][tstep][9], NULL);

            if (id == 0) {
            #ifdef PRINT_INTERACTION_LIST
                PrintInteractionList(INPARAMS ninter);
            #endif
                
            #ifdef MEASURE
                PrintConnectivity();
            #endif
            }
            
            //we need this here because otherwise fast threads might start
            //changing the data that thread 0 is reading above while running
            //PrintInteractionList() and PrintConnectivity().

            gettimeofday(&tim2[id][tstep][10], NULL);

            barrier_wait (&barrier);
     
            gettimeofday(&tim2[id][tstep][11], NULL);

        }
        
        gettimeofday(&tim2[id][tstep][12], NULL);

        ComputeForces (NULL, id);
        //PARALLEL_EXECUTE( NTHREADS, ComputeForces, NULL );
        
        gettimeofday(&tim2[id][tstep][13], NULL);
        
        //this barrier disappears with relaxed predicated commits
        barrier_wait (&barrier);

       gettimeofday(&tim2[id][tstep][14], NULL);
        
        //UpdateVelocities (NULL, id);
        //PARALLEL_EXECUTE( NTHREADS, UpdateVelocities, NULL );
        //this barrier disappears with relaxed predicated commits
        //barrier_wait (&barrier);
        
        //ComputeKEVel (NULL, id);
        //PARALLEL_EXECUTE( NTHREADS, ComputeKEVel, NULL );
        
        //Mike: consolidated all update functions into 1
        Update(NULL, id);
        
        gettimeofday(&tim2[id][tstep][15], NULL);

        //need a barrier at the end of each timestep
        barrier_wait (&barrier);
  
       gettimeofday(&tim2[id][tstep][16], NULL);

        if (id == 0) {
            PrintResults (INPARAMS tstep, (double)ekin, (double)epot, (double)vir,(double)vel,(double)count,numMoles,(int)ninter);
        }
        
        gettimeofday(&tim2[id][tstep][17], NULL);

        barrier_wait (&barrier);
      
        gettimeofday(&tim2[id][tstep][18], NULL);
    } 
    
    return NULL;
    
}


/*
 !============================================================================
 !  Function : main()
 !  Purpose  :  
 !      All the main computational structure  is here
 !      Iterates for specified number of  timesteps.
 !      In each time step, 
 !        UpdateCoordinates() changes molecules coordinates based
 !              on the velocity of that molecules
 !              and that molecules
 !        BuildNeigh() rebuilds the interaction-list on
 !              certain time-steps
 !        ComputeForces() - the time-consuming step, iterates
 !              over all interacting pairs and computes forces
 !        UpdateVelocities() - updates the velocities of
 !              all molecules  based on the forces. 
 !============================================================================
 */

int main( int argc, char *argv[])
{
    if (argc < 2)
    {
        printf("Usage: ./moldyn NUM_THREADS\n");
        return 1;
    }
    NTHREADS = atoi(argv[1]);
    int i;

   /* Define Output file for statistics */
    FILE* out_f = NULL;
    out_f = fopen("Load_stat", "a" );
    
    InitSettings   ();
    InitCoordinates(INPARAMS numMoles, BOXSIZE, perturb);
    InitVelocities (INPARAMS timeStep);
    InitForces     ();
    
    //Mike: added this to compensate for moving updatecoordinates to the end
    FirstCoordinates();
    
    
    barrier_init(&barrier, NTHREADS); 
    
    pthread_attr_t attr;
    pthread_attr_init(&attr);
    pthread_attr_setscope(&attr, PTHREAD_SCOPE_SYSTEM);
    pthread_t* threads = new pthread_t[ NTHREADS-1 ];
    for( i = 1; i < NTHREADS; i++ )
        pthread_create( &threads[i-1], &attr, do_thread_work, (void*)i );
    
    do_thread_work ((void *)0);
    
    for( i = 1; i < NTHREADS; i++ )
        pthread_join( threads[i-1], NULL );
    
    printf("\n");
    

/* just to test it */
int c,t,s;
double time1,time2,wait;

printf("T;Step;M;T\n",c,t,wait);

for(c=0;c<4;c++)
{
for(t=0;t<30;t++)
{

/*for(s=0;s<18;s++)
{

time = tim2[c][t][s].tv_sec+(tim2[c][t][s].tv_usec/1000000.0);
fprintf(out_f,"%d;%d;%d;%.4lf\n",c,t,s,time);

} */
 
/* Function Reset Global Values */
time1 = tim2[c][t][3].tv_sec+(tim2[c][t][3].tv_usec/1000000.0);
time2 = tim2[c][t][2].tv_sec+(tim2[c][t][2].tv_usec/1000000.0);
wait=time1 - time2;
printf("%d;%d;0;%.4lf\n",c,t,wait);

/* Function Build Neigh */
time1 = tim2[c][t][9].tv_sec+(tim2[c][t][9].tv_usec/1000000.0);
time2 = tim2[c][t][8].tv_sec+(tim2[c][t][8].tv_usec/1000000.0);
wait=time1 - time2;
printf("%d;%d;1;%.4lf\n",c,t,wait);

/* Function Compute Forces */
time1 = tim2[c][t][14].tv_sec+(tim2[c][t][14].tv_usec/1000000.0);
time2 = tim2[c][t][13].tv_sec+(tim2[c][t][13].tv_usec/1000000.0);
wait=time1 - time2;
printf("%d;%d;2;%.4lf\n",c,t,wait);

/* Function Update */
time1 = tim2[c][t][16].tv_sec+(tim2[c][t][16].tv_usec/1000000.0);
time2 = tim2[c][t][15].tv_sec+(tim2[c][t][15].tv_usec/1000000.0);
wait=time1 - time2;
printf("%d;%d;3;%.4lf\n",c,t,wait);

/* Function Printing */
time1 = tim2[c][t][18].tv_sec+(tim2[c][t][18].tv_usec/1000000.0);
time2 = tim2[c][t][17].tv_sec+(tim2[c][t][17].tv_usec/1000000.0);
wait=time1 - time2;
printf("%d;%d;4;%.4lf\n",c,t,wait);

}
}

    return 0;
}
