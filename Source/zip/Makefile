MV = mv
DLLTOOL = dlltool

dll:
	$(MAKE) -f win32/makefile.mingw NOASM=1 dll
	$(MV) zip32z64.dll ../bin
	$(MV) libzip32z64.a ../bin

clean:
	$(MAKE) -f win32/makefile.gcc clean
