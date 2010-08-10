MAKEFILE = win32/makefile.mingw
BINDIR = ../bin

ifeq ($(MINGW64), 1)
		MAKEFILE = win32/makefile.w64
		BINDIR = ../bin64
endif

include ../functions.in

dll:
	$(MAKE) -f $(MAKEFILE) NOASM=1 dll
	$(call move_file, unzip32.dll, $(BINDIR))
	$(call move_file, libunzip32.dll.a, $(BINDIR))

clean:
	$(MAKE) -f $(MAKEFILE) clean
	$(call remove_file, $(BINDIR)/unzip32.dll)
	$(call remove_file, $(BINDIR)/libunzip32.dll.a)
