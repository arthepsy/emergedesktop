MV = mv
DLLTOOL = dlltool
MAKEFILE = win32/makefile.mingw
BINDIR = ../bin

ifeq ($(MINGW64), 1)
		MAKEFILE = win32/makefile.w64
		BINDIR = ../bin64
endif

dll:
	$(MAKE) -f $(MAKEFILE) NOASM=1 dll
	$(MV) zip32z64.dll $(BINDIR)
	$(MV) libzip32z64.a $(BINDIR)

clean:
	$(MAKE) -f win32/makefile.gcc clean
