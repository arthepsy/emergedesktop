MAKEFILE = win32/makefile.mingw
RC = windres

ifeq ($(BTYPE), 1)
MAKEFILE = win32/makefile.w64
RC = x86_64-w64-mingw32-windres
else ifeq ($(BTYPE), 4)
MAKEFILE = win32/makefile.w64
endif

dll:
	$(MAKE) -f $(MAKEFILE) RC=$(RC) NOASM=1 dll

clean:
	$(MAKE) -f $(MAKEFILE) clean

