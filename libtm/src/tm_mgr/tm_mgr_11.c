#define RAW_Pes
#define WAR_Pes
#define WAW_Pes

#define WAITFOR_READERS

#define WRITE_BUFFERING
#define WRITE_SET_SEQB

#define mgr_on_begin_x	mgr_on_begin_11
#define mgr_on_rd_x	mgr_on_rd_11
#define mgr_on_wr_x	mgr_on_wr_11
#define mgr_on_commit_x	mgr_on_commit_11
#define mgr_on_abort_x	mgr_on_abort_11

#define cd_vers_str	"RAW_Pes_WAR_Pes_WAW_Pes"
#define cr_vers_str	"WAITFOR_READERS"


#include "../tm_mgr_x.h"
