LIBS += -lversion -lmsimg32 -lshlwapi -lgdi32 -lcomdlg32 -lemergeLib -lemergeGraphics

TARGET = emergeSchemeEngine
SUFFIX = .dll
EXPORTS = -DEMERGESCHEMEENGINE_EXPORTS

include ../functions.in
include ../globals.in
include ../lib_globals.in
include ../svn.in

# Export all symbols as a temporary workaround for a bug in GCC 4.5.0
LDFLAGS += -Wl,--export-all-symbols
