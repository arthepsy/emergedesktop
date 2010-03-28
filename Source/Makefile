# Note: The 'src' and 'bin' targets require the zip program.  For a Windows command prompt, you
# can download zip from http://gnuwin32.sourceforge.net/packages/zip.htm.

include functions.in

LIBS = emergeLib emergeGraphics emergeAppletEngine emergeSchemeEngine emergeBaseClasses emergeIcons
APPLETS = emergeCore emergeDesktop emergeHotkeys emergeTasks emergeCommand emergeLauncher \
					emergeVWM emergeTray emerge emergePower emergeSysMon reg2xml
VERSION = 4.1
PACKAGE = EmergeDesktop
BIN_DIR = bin
SHELL_BREAK = &
DOCUMENTATION = ./Documentation/*.chm
BIN_LIST = ./$(BIN_DIR)/*.exe ./$(BIN_DIR)/*.dll
SVN_FILE = hgversion.h
ifeq ($(MINGW64), 1)
		PACKAGE = EmergeDesktop64
		BIN_DIR = bin64
endif
ifdef CYGWIN
		SHELL_BREAK = ;
endif

all: $(LIBS) $(APPLETS)

include hg_file.in

.PHONY: makedir makedep clean distclean
makedir: all

clean: all

distclean: all

makedep: all

.PHONY: $(APPLETS)
$(APPLETS): $(LIBS)
	$(MAKE) -C $@ $(MAKECMDGOALS)

.PHONY: emergeLib
emergeLib:
	$(MAKE) -C $@ $(MAKECMDGOALS)

.PHONY: emergeGraphics
emergeGraphics: emergeLib
	$(MAKE) -C $@ $(MAKECMDGOALS)

.PHONY: emergeAppletEngine
emergeAppletEngine: emergeLib
	$(MAKE) -C $@ $(MAKECMDGOALS)

.PHONY: emergeSchemeEngine
emergeSchemeEngine: emergeGraphics
	$(MAKE) -C $@ $(MAKECMDGOALS)

.PHONY: emergeBaseClasses
emergeBaseClasses: emergeSchemeEngine emergeAppletEngine
	$(MAKE) -C $@ $(MAKECMDGOALS)

.PHONY: dist
dist: src bin

.PHONY: src
src:
	$(call remove_file,$(PACKAGE)-$(VERSION)-src.zip)
	7z a $(PACKAGE)-$(VERSION)-src.7z * -xr@excludes.lst

.PHONY: bin
bin:
	$(call remove_file,$(PACKAGE)-$(VERSION).zip)
	7z a $(PACKAGE)-$(VERSION).7z $(BIN_LIST)

.PHONY: tarsrc
tarsrc:
	$(call remove_file,$(PACKAGE)-$(VERSION)-src.zip)
	tar -Jcvf $(PACKAGE)-$(VERSION)-src.tar.xz --exclude-from=excludes.lst *

.PHONY: tarbin
tarbin:
	$(call remove_file,$(PACKAGE)-$(VERSION).zip)
	tar -Jcvf $(PACKAGE)-$(VERSION).tar.xz --transform='s,./$(BIN_DIR)/,,S' $(BIN_LIST)
