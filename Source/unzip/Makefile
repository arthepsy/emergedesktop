MAKEFILE = win32/makefile.mingw

ifeq ($(MINGW64), 1)
		MAKEFILE = win32/makefile.w64
endif

dll:
	$(MAKE) -f $(MAKEFILE) NOASM=1 dll

clean:
	$(MAKE) -f $(MAKEFILE) clean
