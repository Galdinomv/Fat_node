#ifndef __TM_H__
#define __TM_H__

#include "tm_general.h"
#include "utils/tm_threads.h"

#include "utils/tm_vers.h"
#include "utils/tm_stats.h"


#define __LIB_TM_PS__


/**************************************************************************
 *		TM_MGR
 **************************************************************************/

#define BEGIN_TRANSACTION()		sigsetjmp( *mgr_on_begin(), 0 )		
#define COMMIT_TRANSACTION()		mgr_on_commit()

#define CHECK_TRANSACTION()		mgr_on_check()
#define SAVE_TRANSACTION( s_id )	0


/**************************************************************************
 *		TM_OBJ - MEMORY MANAGEMENT
 **************************************************************************/


void*	mgr_on_new( size_t size );
void	mgr_on_delete( void* ptr );

class tm_obj
{
	public:
		void* operator new(size_t size)		{	return mgr_on_new( size );	}
		void* operator new[](size_t size)	{	return mgr_on_new( size );	}
		void  operator delete(void* ptr)	{	mgr_on_delete( ptr );		}
		void  operator delete[](void* ptr)	{	mgr_on_delete( ptr );		}
};

void	tm_delete( void* ptr );



/**************************************************************************
 *		TM_TYPE
 **************************************************************************/


#define  _read_T( r_meta, r_addr )	\
({					\
	T resT;				\
	(*(T*)mgr_on_rd( r_meta, r_addr, sizeof(T), (ptr_t)&resT ));	\
})
#define  read_T()		_read_T( ((ptr_t)     __meta), ((ptr_t)     &_t) )
#define  read_tm_T( tm_r )	_read_T( ((ptr_t)tm_r.__meta), ((ptr_t)&tm_r._t) )

#define  write_pT()		((T*)mgr_on_wr( (ptr_t)__meta, (ptr_t)&_t, sizeof(T) ))



template <typename T>
class tm_type : public tm_obj
{
	typedef tm_type<T> tm_T;
public:
    volatile T	_t;
    volatile unsigned int __meta[3];

    tm_type()				{__meta[0] = 0;__meta[1] = 0;__meta[2] = 0;}
    tm_type( T const& r) : _t( r )	{__meta[0] = 0;__meta[1] = 0;__meta[2] = 0;}
    
    operator volatile T () const volatile	{return read_T();}

    T& operator = (    T const&    r )	{return ( *write_pT() = r);			}
    T& operator = ( tm_T const& tm_r )	{return ( *write_pT() = read_tm_T( tm_r ));	}


    T& operator ++ ()			{T tt = read_T();return ( *write_pT() = tt + 1 );}
    T& operator -- ()			{T tt = read_T();return ( *write_pT() = tt - 1 );}

    T operator ++ (int)			{T tt = read_T(); *write_pT() = tt + 1; return tt;}
    T operator -- (int)			{T tt = read_T(); *write_pT() = tt - 1; return tt;}

	/**********/

	T&  operator += (   T const&    r ){	T tt = read_T();return ( *write_pT() = tt + r );}
	T&  operator -= (   T const&    r ){	T tt = read_T();return ( *write_pT() = tt - r );}
	T&  operator *= (   T const&    r ){	T tt = read_T();return ( *write_pT() = tt * r );}
	T&  operator /= (   T const&    r ){	T tt = read_T();return ( *write_pT() = tt / r );}

	/**********/

	T&   operator += (tm_T const& tm_r ){	T tt = read_T();return ( *write_pT() = tt + read_tm_T( tm_r ) );}
	T&   operator -= (tm_T const& tm_r ){	T tt = read_T();return ( *write_pT() = tt - read_tm_T( tm_r ) );}
	T&   operator *= (tm_T const& tm_r ){	T tt = read_T();return ( *write_pT() = tt * read_tm_T( tm_r ) );}
	T&   operator /= (tm_T const& tm_r ){	T tt = read_T();return ( *write_pT() = tt / read_tm_T( tm_r ) );}

	void assign_lock( ptr_t new_meta ){ mgr_on_assign( (ptr_t)__meta, new_meta);}
	void unassign_lock()		  { mgr_on_unassign( (ptr_t) __meta);}

};



/**************************************************************************
 *		ENABLE EXTRAS
 **************************************************************************/

#ifdef ENABLE_EXTRAS


#define TM_WAIT_CONDITION( cond_semaphore )		\
{							\
	COMMIT_TRANSACTION();				\
							\
	volatile int done = 0, cond_i;			\
	while( !done )					\
	{						\
		BEGIN_TRANSACTION();			\
		done = 0;				\
		if( cond_semaphore > 0 )		\
		{					\
			cond_semaphore--;		\
			done = 1;			\
		}					\
		COMMIT_TRANSACTION();			\
							\
		for( cond_i = 0; cond_i < 128; cond_i++ )	\
			asm volatile("nop");		\
	}						\
							\
	BEGIN_TRANSACTION();				\
}


#define TM_SIGNAL_CONDITION( cond_semaphore, signal_n )	\
	cond_semaphore += (signal_n)

ptr_t	mgr_on_tw( ptr_t t_addr, uint_t sz );
#define on_tw()			mgr_on_tw( (ptr_t)this, sizeof(T) )

template <typename T>
class tw_type
{
	typedef tw_type<T> tw_T;
public:
    T	_t;
	tw_type() {}
	tw_type( T const& r) : _t( r )	{}

	operator T (){	return _t;   }

	T& operator = ( T const&    r    ) { on_tw();return _t = r;	}
        T& operator = ( tw_T const& tw_r ) { on_tw();return _t = tw_r._t;}

	T& operator ++ ()		   { on_tw();return ++_t;}
        T& operator -- ()		   { on_tw();return --_t;}

	T& operator ++ (int)		   { on_tw();return _t++;}
        T& operator -- (int)		   { on_tw();return _t--;}

	/**********/

	T&   operator += (   T const&    r ){ on_tw();return ( _t += r );}
        T&   operator -= (   T const&    r ){ on_tw();return ( _t -= r );}
	T&   operator *= (   T const&    r ){ on_tw();return ( _t *= r );}
	T&   operator /= (   T const&    r ){ on_tw();return ( _t /= r );}

	/**********/

        T&   operator += (tw_T const& tw_r ){ on_tw();return ( _t += tw_r._t );}
        T&   operator -= (tw_T const& tw_r ){ on_tw();return ( _t -= tw_r._t );}
	T&   operator *= (tw_T const& tw_r ){ on_tw();return ( _t *= tw_r._t );}
	T&   operator /= (tw_T const& tw_r ){ on_tw();return ( _t /= tw_r._t );}
}; 

#endif	// ENABLE_EXTRAS




/**************************************************************************
 *		TYPE REDEFINITIONS
 **************************************************************************/


typedef	tm_type<char>			tm_char;
typedef	tm_type<short>			tm_short;
typedef	tm_type<int>			tm_int;
typedef	tm_type<long>			tm_long;
typedef	tm_type<long long>		tm_llong;

typedef	tm_type<unsigned char>		tm_uchar;
typedef	tm_type<unsigned short>		tm_ushort;
typedef	tm_type<unsigned int>		tm_uint;
typedef	tm_type<unsigned long>		tm_ulong;
typedef	tm_type<unsigned long long>	tm_ullong;

typedef	tm_type<float>			tm_float;
typedef	tm_type<double>			tm_double;


#endif	// __TM_H__

