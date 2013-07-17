#define RAW_Opt
#define WAR_Opt
#define WAW_Pes

#define ABORT_READERS

#define WRITE_BUFFERING
#define WRITE_SET_SEQB

#define mgr_on_begin_x	mgr_on_begin_32
#define mgr_on_rd_x	mgr_on_rd_32
#define mgr_on_wr_x	mgr_on_wr_32
#define mgr_on_commit_x	mgr_on_commit_32
#define mgr_on_abort_x	mgr_on_abort_32
#define mgr_on_check_x	mgr_on_check_32

#define cd_vers_str	"RAW_Opt_WAR_Opt_WAW_Pes"
#define cr_vers_str	"ABORT_READERS"


#include "../tm_mgr_x.h"
