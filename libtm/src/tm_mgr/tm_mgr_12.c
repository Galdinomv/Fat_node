#define RAW_Pes
#define WAR_Pes
#define WAW_Pes

#define ABORT_READERS

#define WRITE_BUFFERING
#define WRITE_SET_SEQB

#define mgr_on_begin_x	mgr_on_begin_12
#define mgr_on_rd_x	mgr_on_rd_12
#define mgr_on_wr_x	mgr_on_wr_12
#define mgr_on_commit_x	mgr_on_commit_12
#define mgr_on_abort_x	mgr_on_abort_12
#define mgr_on_check_x	mgr_on_check_12

#define cd_vers_str	"RAW_Pes_WAR_Pes_WAW_Pes"
#define cr_vers_str	"ABORT_READERS"


#include "../tm_mgr_x.h"
