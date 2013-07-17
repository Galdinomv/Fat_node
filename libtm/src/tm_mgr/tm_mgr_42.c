#define RAW_Opt
#define WAR_Opt
#define WAW_Opt

#define ABORT_READERS

#define WRITE_BUFFERING
#define WRITE_SET_HTAB

#define mgr_on_begin_x	mgr_on_begin_42
#define mgr_on_rd_x	mgr_on_rd_42
#define mgr_on_wr_x	mgr_on_wr_42
#define mgr_on_commit_x	mgr_on_commit_42
#define mgr_on_abort_x	mgr_on_abort_42
#define mgr_on_check_x	mgr_on_check_42

#define cd_vers_str	"RAW_Opt_WAR_Opt_WAW_Opt"
#define cr_vers_str	"ABORT_READERS"


#include "../tm_mgr_x.h"
