LIBS += -lshlwapi \
				-lole32 \
				-luuid \
				-lversion \
				-lpsapi \
				-lwinmm \
				-lpowrprof \
				-lgdi32 \
				-ladvapi32
ifneq ($(BTYPE), 4)
LIBS += -lwtsapi32
endif

TARGET = emergeLib
SUFFIX = .dll
EXPORTS = -DEMERGELIB_EXPORTS
TARGET_OBJS = $(TINYOBJS)

include ../functions.in
include ../globals.in
include ../lib_globals.in
include ../hg_file.in

CXXFLAGS += -DTIXML_USE_STL -Wno-long-long -Wno-unused-value -Wno-strict-aliasing
