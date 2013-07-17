#include "tm_vers.h"

volatile unsigned int	cd_version = 1;
volatile unsigned int	cr_version = 1;




#ifdef VERSION_DYNAMIC

void print_version( FILE* f_out )
{
	if( cd_version == 0 )	fprintf( f_out, "Biglock " );
	if( cd_version == 1 )	fprintf( f_out, "RAW_Pes_WAR_Pes_WAW_Pes" );
	if( cd_version == 2 )	fprintf( f_out, "RAW_Pes_WAR_Opt_WAW_Pes" );
	if( cd_version == 3 )	fprintf( f_out, "RAW_Opt_WAR_Opt_WAW_Pes" );
	if( cd_version == 4 )	fprintf( f_out, "RAW_Opt_WAR_Opt_WAW_Opt" );

	if( cr_version == 1 )	fprintf( f_out, "_WAITFOR_READERS " );
	if( cr_version == 2 )	fprintf( f_out, "_ABORT_READERS " );
}

void set_version( int cd_v, int cr_v )
{
	tm_assert( cd_v >= 0 && cd_v <= 4 );
	tm_assert( cr_v >= 0 && cr_v <= 2 );
	tm_assert( cd_v != 0 || cr_v == 0 );
	tm_assert( cd_v == 0 || cr_v >= 1 );
	
	cd_version = cd_v;
	cr_version = cr_v;
}

#endif



