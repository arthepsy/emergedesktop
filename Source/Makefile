# Note: The 'src' and 'bin' targets require the zip program.  For a Windows command prompt, you
# can download zip from http://gnuwin32.sourceforge.net/packages/zip.htm.

include functions.in

LIBS = zip unzip emergeLib emergeGraphics emergeAppletEngine emergeStyleEngine emergeBaseClasses emergeIcons
APPLETS = emergeCore emergeWorkspace emergeHotkeys emergeTasks emergeCommand emergeLauncher \
					emergeVWM emergeTray emerge emergePower emergeSysMon reg2xml
VERSION = 4.1
PACKAGE = EmergeDesktop
BIN_DIR = bin
SHELL_BREAK = &
DOCUMENTATION = Documentation/*.chm
BIN_LIST = ./$(BIN_DIR)/*.exe ./$(BIN_DIR)/*.dll
SVN_FILE = hgversion.h
BTYPE = 4
ifeq ($(BTYPE), 1)
		PACKAGE = EmergeDesktop64
		BIN_DIR = bin64
else ifeq ($(BTYPE), 4)
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
	$(MAKE) -C $@ BTYPE=$(BTYPE) $(MAKECMDGOALS)

.PHONY: zip
zip:
	$(MAKE) -C $@ BTYPE=$(BTYPE) $(MAKECMDGOALS)

.PHONY: unzip
unzip:
	$(MAKE) -C $@ BTYPE=$(BTYPE) $(MAKECMDGOALS)

.PHONY: emergeLib
emergeLib: zip unzip
	$(MAKE) -C $@ BTYPE=$(BTYPE) $(MAKECMDGOALS)

.PHONY: emergeIcons
emergeIcons: emergeLib
	$(MAKE) -C $@ BTYPE=$(BTYPE) $(MAKECMDGOALS)

.PHONY: emergeGraphics
emergeGraphics: emergeLib
	$(MAKE) -C $@ BTYPE=$(BTYPE) $(MAKECMDGOALS)

.PHONY: emergeAppletEngine
emergeAppletEngine: emergeLib
	$(MAKE) -C $@ BTYPE=$(BTYPE) $(MAKECMDGOALS)

.PHONY: emergeStyleEngine
emergeStyleEngine: emergeGraphics
	$(MAKE) -C $@ BTYPE=$(BTYPE) $(MAKECMDGOALS)

.PHONY: emergeBaseClasses
emergeBaseClasses: emergeStyleEngine emergeAppletEngine
	$(MAKE) -C $@ BTYPE=$(BTYPE) $(MAKECMDGOALS)

.PHONY: dist
dist: src bin

.PHONY: src
src:
	$(call remove_file,$(PACKAGE)-$(VERSION)-src.7z)
	7z a $(PACKAGE)-$(VERSION)-src.7z * -xr@excludes.lst

.PHONY: bin
bin:
	$(call remove_file,$(PACKAGE)-$(VERSION).7z)
	7z a $(PACKAGE)-$(VERSION).7z $(BIN_LIST)
	cd ..; 7z a Source/$(PACKAGE)-$(VERSION).7z $(DOCUMENTATION)

.PHONY: tarsrc
tarsrc:
	$(call remove_file,$(PACKAGE)-$(VERSION)-src.tar.xz)
	tar -Jcvf $(PACKAGE)-$(VERSION)-src.tar.xz --exclude-from=excludes.lst *

.PHONY: tarbin
tarbin:
	$(call remove_file,$(PACKAGE)-$(VERSION).tar.xz)
	tar -Jcvf $(PACKAGE)-$(VERSION).tar.xz --transform='s,./$(BIN_DIR)/,,S' $(BIN_LIST)
