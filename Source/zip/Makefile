MAKEFILE = win32/makefile.mingw
RC = windres

ifeq ($(ARCH), 64)
MAKEFILE = win32/makefile.w64
endif

dll:
	$(MAKE) -f $(MAKEFILE) RC=$(RC) NOASM=1 dll

clean:
	$(MAKE) -f $(MAKEFILE) clean

makedir:

makedep:


