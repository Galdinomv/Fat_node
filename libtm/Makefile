ifeq ($(DEBUG), )
	DEBUG = -O3
endif


ifeq ($(VERSION), )
	VERSION = -DVERSION_DYNAMIC
endif


CXX=g++ 
LDFLAGS = -lpthread
MACHINE := $(shell uname -n) 
CFLAGS := -m32 -Wall $(DEBUG) -D$(MACHINE) $(TM_FLAGS) $(VERSION)

#ifneq ($(MY_FLAGS), )
#	CFLAGS += $(MY_FLAGS)
#endif


# -g3 -gdwarf-2
#-DUSE_DL_PREFIX -DUSE_MALLOC_LOCK -DDEBUG_HEAP 

SRC_DIR		= src
BUILD_DIR	= build

INCLUDES	= $(wildcard $(SRC_DIR)/*.h)	$(wildcard $(SRC_DIR)/infra/*.h)	$(wildcard $(SRC_DIR)/lock_mgr/*.h)		\
			  $(wildcard $(SRC_DIR)/utils/*.h)
SRCS		= $(wildcard $(SRC_DIR)/*.c)	$(wildcard $(SRC_DIR)/infra/*.c)	$(wildcard $(SRC_DIR)/tm_mgr/*.c)		\
  			  $(wildcard $(SRC_DIR)/utils/*.c)
OBJS		= $(SRCS:$(SRC_DIR)/%.c=$(BUILD_DIR)/%.o)

all: lib_tm.a

init:
	mkdir -p $(BUILD_DIR)
	mkdir -p $(BUILD_DIR)/infra
	mkdir -p $(BUILD_DIR)/tm_mgr
	mkdir -p $(BUILD_DIR)/utils

lib_tm.a: init $(OBJS)
	rm -f $@
	ar -qc $@ $(OBJS)


hash: lib_tm.a
	$(MAKE) -C apps/hash hash

bench: lib_tm.a
	$(MAKE) -C apps/bench bench CFLAGS="$(CFLAGS)"

qs: lib_tm.a
	$(MAKE) -C apps/qs qs

test: lib_tm.a
	$(MAKE) -C apps/test test CFLAGS="$(CFLAGS)"
test2: lib_tm.a
	$(MAKE) -C apps/test test2 CFLAGS="$(CFLAGS)"
	
test_c: lib_tm.a
	$(MAKE) -C apps/test test_c CFLAGS="$(CFLAGS)"
test_d: lib_tm.a
	$(MAKE) -C apps/test test_d CFLAGS="$(CFLAGS)"
test_ps: lib_tm.a
	$(MAKE) -C apps/test test_ps CFLAGS="$(CFLAGS)"	
	
	
test_m: lib_tm.a
	$(MAKE) -C apps/test test_m CFLAGS="$(CFLAGS)"



test_c_dis_lks: CFLAGS += -DDISABLE_LOCKS
test_c_dis_lks: lib_tm.a
	$(MAKE) -C apps/test test_c CFLAGS="$(CFLAGS)"
test_c_dis_all: CFLAGS += -DDISABLE_ALL
test_c_dis_all: lib_tm.a
	$(MAKE) -C apps/test test_c CFLAGS="$(CFLAGS)"


moldyn: lib_tm.a
	$(MAKE) -C apps/moldyn moldyn

md: lib_tm.a
	$(MAKE) -C apps/md md
mmd: lib_tm.a
	$(MAKE) -C apps/md mmd


$(BUILD_DIR)/%.o: $(SRC_DIR)/%.cc $(INCLUDES)
	$(CXX) -c $(CFLAGS) -o $@ $<

$(BUILD_DIR)/%.o: $(SRC_DIR)/%.c $(INCLUDES)
	$(CXX) -c $(CFLAGS) -o $@ $<



clean:
	-rm -f -R $(BUILD_DIR)
	-rm -f lib_tm.a core* out.txt

clean_hash:
	$(MAKE) -C apps/hash clean
clean_bench:
	$(MAKE) -C apps/bench clean
clean_test:
	$(MAKE) -C apps/test clean
clean_moldyn:
	$(MAKE) -C apps/moldyn clean
clean_md:
	$(MAKE) -C apps/md clean
clean_qs:
	$(MAKE) -C apps/qs clean

clean_all:	clean clean_hash clean_test clean_moldyn clean_md clean_bench clean_qs
