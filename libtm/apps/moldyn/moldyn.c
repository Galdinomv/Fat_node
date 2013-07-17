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

/* TM_Modification_Begin */

/* TM include */
#include "../../src/tm.h"

/* TM_Modification_end */

#define LOCAL       /* Such a function that changes no global vars */
#define INPARAMS    /* All parameters following this are 'in' params  */

#define SQRT(a)  sqrt(a)
#define POW(a,b) pow(a,b)
#define SQR(a)   ((a)*(a))
#define DRAND(a)  drand_x(a)

/* TM_Modification_Begin */

/* TM Definitions */
#define NTHREADS 4
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

//double  *x;     /* x,y,z coordinates of each molecule */
//double  *vh;    /* partial x,y,z velocity of molecule */
//double  *f;     /* partial forces on each molecule    */
//int     *inter; /* pairs of interacting molecules     */ 

/* TM variables */

tm_int inter[MAXINTERACT * DSIZE];

tm_double x[NUM_PARTICLES * NDIM];
tm_double f[NUM_PARTICLES * NDIM];
tm_double vh[NUM_PARTICLES * NDIM];

#ifdef MEASURE
tm_int connect[NUM_PARTICLES];
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
tm_int      ninter;                /*  number of interacting molecules pairs  */
double   vaver;                 /*                                        */

tm_double   epot;                  /*  The potential energy      */
tm_double   vir;                   /*  The virial  energy        */
tm_double   count, vel ;
tm_double   ekin;

/* TM Variables */
int      n3;


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

/*---------- UODATE ROUTINES HERE               ------------------*/

/*
!============================================================================
!  Function :  UpdateCoordinates()
!  Purpose  : 
!     This routine moves the molecules based on 
!     forces acting on them and their velocities
!============================================================================
*/

int UpdateCoordinates( void* arg, int id )
{
 int i;
 for ( i = id; i < n3; i += NTHREADS) 
   {
     BEGIN_TRANSACTION();

     x[i] = x[i] + vh[i] + f[i];
     if ( x[i] < 0.0 )    x[i] = x[i] + side ;
     if ( x[i] > side   ) x[i] = x[i] - side ;
     vh[i] = vh[i] + f[i];
     f[i]  = 0.0;

     COMMIT_TRANSACTION();
   }

   return 0;
}

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

int BuildNeigh( void* arg, int id )
{
  double rd, cutoffSquare;
  int    i,j;
  //double Foo();
    
  cutoffSquare  = (cutoffRadius * TOLERANCE)*(cutoffRadius * TOLERANCE); 

  for ( i=id; i<numMoles; i+=NTHREADS)
  {
    BEGIN_TRANSACTION();

    for ( j = i+1; j<numMoles; j++ ) 
    {
      rd = Foo ( (double)x[IND(0,i)], (double)x[IND(1,i)], (double)x[IND(2,i)], 
                 (double)x[IND(0,j)], (double)x[IND(1,j)], (double)x[IND(2,j)]); 
      if ( rd <= cutoffSquare) 
      {
        inter[INDX(ninter,0)] = i;
        inter[INDX(ninter,1)] = j;
        ninter ++;   
        if ( ninter >= MAXINTERACT) perror("MAXINTERACT limit");
      }  
    } 

    COMMIT_TRANSACTION();
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

int ComputeForces( void* arg, int id )
{
  double cutoffSquare;
  double xx, yy, zz, rd, rrd, rrd2, rrd3, rrd4, rrd5, rrd6, rrd7, r148;
  double forcex, forcey, forcez;
  int    i,j,ii;

  cutoffSquare = cutoffRadius*cutoffRadius ;
   
  for(ii=id; ii<ninter; ii+=NTHREADS) {

    BEGIN_TRANSACTION();

    i = inter[INDX(ii,0)];
    j = inter[INDX(ii,1)];

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

      vir -= rd*r148 ;
      epot += (rrd6 - rrd3);      
    }

    COMMIT_TRANSACTION();
  } 
  
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

int UpdateVelocities( void* arg, int id )
{
  int i;
  for ( i = id; i < numMoles; i += NTHREADS) 
  {

    BEGIN_TRANSACTION();

    f[IND(0,i)]  = f[IND(0,i)] * timeStepSqHalf ;
    f[IND(1,i)]  = f[IND(1,i)] * timeStepSqHalf ;
    f[IND(2,i)]  = f[IND(2,i)] * timeStepSqHalf ;

    vh[ IND(0,i) ] += f[ IND(0,i) ];
    vh[ IND(1,i) ] += f[ IND(1,i) ];
    vh[ IND(2,i) ] += f[ IND(2,i) ];

    COMMIT_TRANSACTION();
  }

  return 0;
}

/*
!============================================================================
!  Function :  ComputeKE()
!  Purpose :
!     Computes  the KE of the system by summing all
!     the squares of the partial velocities.
!============================================================================
*/

int ComputeKEVel( void * arg, int id)
{
  double sum = 0.0;
  double vaverh, velocity, counter, sq;

  int    i;

  vaverh = vaver * timeStep ;
  velocity    = 0.0 ;
  counter     = 0.0 ;

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

  BEGIN_TRANSACTION();

  ekin += sum/timeStepSq;
  vel += velocity/timeStep;
  count += counter;

  COMMIT_TRANSACTION();

  return 0;
}

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
  if (argc < 2) {
	printf("Usage: CONF_DET CONF_RES\n");
	return 0;
  }
  
  int cd, cr;

  cd = atoi( argv[1] );
  cr = atoi( argv[2] );
  /* Set conflict detection and conflict resolution policies */
  printf("Setting CD = %d, CR = %d\n", cd, cr);
  set_version(cd, cr);

  int tstep;
  int i;

  /* Initialization */
  /*........................................................*/

  InitSettings   ();
  InitCoordinates(INPARAMS numMoles, BOXSIZE, perturb);
  InitVelocities (INPARAMS timeStep);
  InitForces     (); 

  CREATE_TM_THREADS( NTHREADS );

  /*........................................................*/
  for ( tstep=0; tstep< NUMBER_TIMESTEPS; tstep++) 
  {
    int i;

    PARALLEL_EXECUTE( NTHREADS, UpdateCoordinates, NULL );
    
    if ( tstep % neighUpdate == 0) 
    {
      
#ifdef PRINT_COORDINATES
      PrintCoordinates(numMoles); 
#endif

      ninter = 0;
      PARALLEL_EXECUTE( NTHREADS, BuildNeigh, NULL );

#ifdef PRINT_INTERACTION_LIST     
      PrintInteractionList(INPARAMS ninter);
#endif 

#ifdef MEASURE
      PrintConnectivity();
#endif
    }

    vir  = 0.0;
    epot = 0.0;

    PARALLEL_EXECUTE( NTHREADS, ComputeForces, NULL );


    PARALLEL_EXECUTE( NTHREADS, UpdateVelocities, NULL );

    ekin = 0.0;
    vel = 0.0;
    count = 0.0;

    /* Computer Kinetic energy */
    PARALLEL_EXECUTE( NTHREADS, ComputeKEVel, NULL );


    PrintResults (INPARAMS tstep, (double)ekin, (double)epot, (double)vir,(double)vel,(double)count,numMoles,(int)ninter);
  } 
  /*........................................................*/


  /*........................................................*/
  
  DESTROY_TM_THREADS( NTHREADS );

  printf("\n");
}
