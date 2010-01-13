LIBS += -lshlwapi -lole32 -luuid -lversion -lpsapi -lwinmm -lpowrprof -lgdi32
ifndef MINGW64
		LIBS += -lwtsapi32
endif

TARGET = emergeLib
SUFFIX = .dll
EXPORTS = -DEMERGELIB_EXPORTS
TARGET_OBJS = $(TINYOBJS)

include ../functions.in
include ../globals.in
include ../lib_globals.in
