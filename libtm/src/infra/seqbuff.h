#ifndef _seqbuff_H
#define	_seqbuff_H


#ifdef	__cplusplus
extern "C" {
#endif


#include <stdlib.h>

#define likely(x)       __builtin_expect(!!(x), 1)
#define unlikely(x)     __builtin_expect(!!(x), 0)



#define SBUFF_SIZE	(1024 * 1024 * 16 )
// 4080
#define SM_SBUFF_SIZE	240

#define sm_seqbuff_init( new_seqb_ptr )		seqbuff_init( new_seqb_ptr, SM_SBUFF_SIZE )
#define sm_seqbuff_put_ptr( seqb_ptr, __sz )	seqbuff_put_ptr( seqb_ptr, __sz, SM_SBUFF_SIZE )

typedef struct _sbuff_t
{
    char* put_pos;
    char* get_pos;
    char* last_pos;
    struct _sbuff_t* next;
    char data[0];
} sbuff_t;

typedef struct _seqbuff_t
{
    sbuff_t* put_buff;
    sbuff_t* get_buff;
} seqbuff_t;

inline __attribute__((always_inline))
sbuff_t* new_sbuff( int size )
{
    sbuff_t* new_sb = (sbuff_t*) malloc( sizeof( sbuff_t ) + (size) );
    new_sb->put_pos = &new_sb->data[0];
    new_sb->get_pos = &new_sb->data[0];
    new_sb->last_pos = &new_sb->data[size];
    new_sb->next = NULL;

    return new_sb;
}


inline __attribute__((always_inline))
void seqbuff_init( seqbuff_t* new_seqb, int sbuff_size = SBUFF_SIZE  )
{
    sbuff_t* new_sb = new_sbuff( sbuff_size );
    new_seqb->put_buff = new_sb;
    new_seqb->get_buff = new_sb;
    return;
}


inline __attribute__((always_inline))
seqbuff_t* new_seqbuff(int sbuff_size = SBUFF_SIZE )
{
    seqbuff_t* new_seqb = (seqbuff_t*) malloc( sizeof( seqbuff_t ) );
    seqbuff_init( new_seqb, sbuff_size );
    return new_seqb;
}


inline __attribute__((always_inline))
void free_seqbuff( seqbuff_t* seqb )
{
    sbuff_t* next_sb;
    sbuff_t* sb = seqb->get_buff;

    do
    {
	next_sb = sb->next;
	free(sb);
	sb = next_sb;
    }
    while( sb );

    free( seqb );
}

inline __attribute__((always_inline))
char* seqbuff_put_ptr( seqbuff_t* seqb, unsigned int _sz, int sbuff_size = SBUFF_SIZE )
{
    char* rez;
    sbuff_t* sb = seqb->put_buff;

    if( (sb->put_pos + _sz) > sb->last_pos )
    {
	sb->next = new_sbuff( sbuff_size );
	sb = sb->next;
        seqb->put_buff = sb;
    }
    rez = sb->put_pos;
    sb->put_pos += _sz;

    return rez;
}


inline __attribute__((always_inline))
sbuff_t* advance_sb( seqbuff_t* seqb, sbuff_t* sb )
{
	sbuff_t* next_sb = (sb)->next;
	if( next_sb )
	{
	    free( sb );
	    (seqb)->get_buff = next_sb;
	}
	else
	{
	    (sb)->put_pos = &(sb)->data[0];
	    (sb)->get_pos = &(sb)->data[0];
	}

	return next_sb;
}

#define end_of_sb( sb )		( (sb)->get_pos == (sb)->put_pos )
#define inc_sb( sb, _sz)	sb->get_pos += _sz
#define has_next_sb( seqb, sb )	( !end_of_sb(sb) || ( sb = advance_sb(seqb, sb) ) )


inline __attribute__((always_inline))
char* seqbuff_get_ptr( seqbuff_t* seqb, unsigned int _sz )
{
    char* rez = NULL;
    sbuff_t* sb = seqb->get_buff;

    if( has_next_sb( seqb, sb ) )
    {
	rez = sb->get_pos;
    	inc_sb( sb, _sz);
    }
	return rez;
}

#define seqbuff_read_ptr( seqb, sb )	( has_next_sb( seqb, sb ) ? (sb)->get_pos : NULL )

#define _seqbuff_for_each( el, f_seqb, el_t, f_sb, el_sz )	\
	el_t*    el;						\
	sbuff_t* f_sb = (f_seqb)->get_buff;			\
	for( el = (el_t*) seqbuff_read_ptr( (f_seqb), f_sb );	\
		 el != NULL;					\
		 inc_sb( f_sb, el_sz),				\
		 el = (el_t*) seqbuff_read_ptr( (f_seqb), f_sb ) ) 

#define  seqbuff_for_each( el, f_seqb, el_t, f_offset )		\
	_seqbuff_for_each( el, f_seqb, el_t, el ## _sb, sizeof(el_t)+(f_offset) )




#define end_of_sb2( pos, sb )	( pos == (sb)->put_pos )
#define advance_sb2( pos, sb )	\
({				\
	sb = (sb)->next;	\
	pos = (sb) ? (sb)->get_pos : NULL;	\
	pos;			\
})

#define seqbuff_read_ptr2( pos, sb )	( !end_of_sb2( pos, sb ) ? pos : advance_sb2( pos, sb ) )

#define _seqbuff_pass_by( el, f_seqb, el_t, f_sb, f_pos, f_sz )	\
	el_t*    el;						\
	sbuff_t* f_sb = (f_seqb)->get_buff;			\
	char*    f_pos = f_sb->get_pos;				\
	for( el = (el_t*) seqbuff_read_ptr2( f_pos, f_sb );	\
		 el != NULL;					\
		 f_pos += f_sz,					\
		 el = (el_t*) seqbuff_read_ptr2( f_pos, f_sb ) )

#define  seqbuff_pass_by( el, f_seqb, el_t, f_offset )		\
	_seqbuff_pass_by( el, f_seqb, el_t, el ## _sb, el ## _pos, sizeof(el_t)+(f_offset) )


#ifdef	__cplusplus
}
#endif

#endif	/* _seqbuff_H */

