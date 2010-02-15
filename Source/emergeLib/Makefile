LIBS += -lshlwapi -lole32 -luuid -lversion -lpsapi -lwinmm -lpowrprof -lgdi32
ifneq ($(MINGW64), 1)
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

CFLAGS += -DTIXML_USE_STL
