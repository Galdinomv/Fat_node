#ifndef TM_MGR_EXTRAS_H_
#define TM_MGR_EXTRAS_H_


/***********************************************************
 *		EXTRAS	INFRASTRUCTURE
 ***********************************************************/



#ifdef ENABLE_EXTRAS

/****    TWINS	INFRASTRUCTURE    ****/

typedef struct
{
    ptr_t       addr;
    uint_t      sz;
    byte_t      twin[0];
} twin_t;

typedef  hshtbl twin_set_t;

#define twin_set_init  hsh_init
#define twin_set_for_each( t_tw, t_twins )  hsh_for_each_r( t_tw, t_twins, twin_t, (t_tw)->sz ) 

#define has_tw( t_addr, t_tran ) hsh_find_r( &(t_tran)->twins, (int_pointer_t)t_addr )


___always_inline
void twin_set_put( ptr_t t_addr, uint_t t_sz, twin_set_t* t_twins )															
{																										
	twin_t* tw = (twin_t*)hsh_insert_r( t_twins, (int_pointer_t)t_addr, sizeof(twin_t) + (t_sz) );
	tw->addr = t_addr;																					
	tw->sz = t_sz;																						
	memcpy( tw->twin, t_addr, t_sz );																	
}



#endif	// ENABLE_EXTRAS



#endif /*TM_MGR_EXTRAS_H_*/
