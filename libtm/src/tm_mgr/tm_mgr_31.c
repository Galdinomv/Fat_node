#define RAW_Opt
#define WAR_Opt
#define WAW_Pes

#define WAITFOR_READERS

#define WRITE_BUFFERING
#define WRITE_SET_SEQB

#define mgr_on_begin_x	mgr_on_begin_31
#define mgr_on_rd_x	mgr_on_rd_31
#define mgr_on_wr_x	mgr_on_wr_31
#define mgr_on_commit_x	mgr_on_commit_31
#define mgr_on_abort_x	mgr_on_abort_31

#define cd_vers_str	"RAW_Opt_WAR_Opt_WAW_Pes"
#define cr_vers_str	"WAITFOR_READERS"


#include "../tm_mgr_x.h"
