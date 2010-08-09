MAKEFILE = win32/makefile.mingw
BINDIR = ../bin

ifeq ($(MINGW64), 1)
		MAKEFILE = win32/makefile.w64
		BINDIR = ../bin64
endif

include ../functions.in

dll:
	$(MAKE) -f $(MAKEFILE) NOASM=1 dll
	$(call move_file, zip32z64.dll, $(BINDIR))
	$(call move_file, libzip32z64.dll.a, $(BINDIR))

clean:
	$(MAKE) -f $(MAKEFILE) clean
	$(call remove_file, $(BINDIR)/zip32z64.dll)
	$(call remove_file, $(BINDIR)/libzip32z64.dll.a)
