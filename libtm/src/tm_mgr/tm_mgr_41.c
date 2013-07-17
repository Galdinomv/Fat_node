#define RAW_Opt
#define WAR_Opt
#define WAW_Opt

#define WAITFOR_READERS

#define WRITE_BUFFERING
#define WRITE_SET_HTAB

#define mgr_on_begin_x	mgr_on_begin_41
#define mgr_on_rd_x	mgr_on_rd_41
#define mgr_on_wr_x	mgr_on_wr_41
#define mgr_on_commit_x	mgr_on_commit_41
#define mgr_on_abort_x	mgr_on_abort_41

#define cd_vers_str	"RAW_Opt_WAR_Opt_WAW_Opt"
#define cr_vers_str	"WAITFOR_READERS"


#include "../tm_mgr_x.h"
