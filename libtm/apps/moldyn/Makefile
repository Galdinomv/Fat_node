#  Copyright (c) 1995 by Mark Hill, James Larus, and David Wood for 
#  the Wisconsin Wind Tunnel Project and Joel Saltz for the High Performance 
#  Software Laboratory, University of Maryland, College Park.
# 
#  ALL RIGHTS RESERVED.
# 
#  This software is furnished under a license and may be used and
#  copied only in accordance with the terms of such license and the
#  inclusion of the above copyright notice.  This software or any other
#  copies thereof or any derivative works may not be provided or
#  otherwise made available to any other persons.  Title to and ownership
#  of the software is retained by Mark Hill, James Larus, Joel Saltz, and 
#  David Wood. Any use of this software must include the above copyright 
#  notice.
# 
#  THIS SOFTWARE IS PROVIDED "AS IS".  THE LICENSOR MAKES NO
#  WARRANTIES ABOUT ITS CORRECTNESS OR PERFORMANCE.
# 
#  Shamik Sharma generated the sequential version in C. 
#  Shubhendu Mukherjee (Shubu) parallelized moldyn on Tempest. 
#

#***********************************************************
#  MITS - MARYLAND IRREGULAR TEST SET
#
#  Shamik D. Sharma, _OTHERS_
#  Computer Science Department,
#  University of Maryland,
#  College Park, MD-20742
#
#  Contact: shamik@cs.umd.edu
#***********************************************************/

#***********************************************************/
# File :  Makefile  (for  moldyn.c)
# Created  :  Shamik Sharma,
# Modified :  Mike Dai Wang 
#***********************************************************
    CC = g++
#   CFLAGS = -O  -dalign -DSMALL -DMEASURE
#   CFLAGS = -O  -dalign -DMEDIUM -DMEASURE
#   CFLAGS = -O  -dalign -DMEDIUM -DPRINT_COORDINATES
#   CFLAGS = -O  -dalign -DMEDIUM -DPRINT_INTERACTION_LIST
#   CFLAGS = -O  -dalign -DMEDIUM 
    CFLAGS = -m32 -g -O3 -DSMALL -DPRINT_COORDINATES	-DVERSION_DYNAMIC -DCD=2 -DCR=2
    LDFLAGS = -lm -lpthread -l_tm

    VERSION = -DVERSION_DYNAMIC

    export VERSION

LIBTM_DIR = ../..
OUT_DIR = ../..

.c.o:
	$(CC) $(CFLAGS) -c $*.c 

#***********************************************************
    TARGET = moldyn
    OBJS   = moldyn.o
#***********************************************************

$(TARGET) : $(OBJS)
	$(CC) $(CFLAGS) -o $(OUT_DIR)/$@ $(OBJS) $(LDFLAGS) -L$(LIBTM_DIR)

#***********************************************************
#    Dependencies
#***********************************************************

moldyn.o   : moldyn.c Makefile

#***********************************************************
clean:
	/bin/rm -f $(OBJS) $(OUT_DIR)/$(TARGET)
