MAKEFILE = win32/makefile.mingw

ifeq ($(BTYPE), 1)
MAKEFILE = win32/makefile.w64
else ifeq ($(BTYPE), 4)
MAKEFILE = win32/makefile.w64
endif

dll:
	$(MAKE) -f $(MAKEFILE) NOASM=1 dll

clean:
	$(MAKE) -f $(MAKEFILE) clean
