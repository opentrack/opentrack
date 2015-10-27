noinst_SCRIPTS = 
if WINE_PLUGIN
  noinst_SCRIPTS += Tester.exe
if WINE64
  noinst_SCRIPTS += Tester64.exe
endif #WINE64
endif #WINE_PLUGIN

if DARWIN
  LDFLAGS += -Wl,-no_arch_warnings
else
  LDFLAGS += -Wl,--no-warn-search-mismatch
endif

CC = winegcc

CXX = wineg++

SUFFIXES = .o .cpp .c .rc 64.o

.cpp.o :
	$(CXX) -c $(CXXFLAGS) -m32 -o $@ $<

.c.o :
	$(CC) -c $(CFLAGS) -m32 -o $@ $<

.cpp64.o :
	$(CXX) -c $(CXXFLAGS) -o $@ $<

.c64.o :
	$(CC) -c $(CFLAGS) -o $@ $<

.rc.o :
	wrc -o $@ $(RCFLAGS) $<

CXXFLAGS += -g -DHAVE_CONFIG_H -I../../.. -I. -I@srcdir@/../.. -I@top_builddir@
CFLAGS += -g -I../.. -I../../.. -DHAVE_CONFIG_H -I@srcdir@/../.. -I@top_builddir@
RCFLAGS = -I @srcdir@
#VPATH = ../..:@srcdir@/../..:@top_builddir@:@srcdir@
vpath %.h @srcdir@/../..
vpath %.h @top_builddir@
vpath %.c @srcdir@
vpath %.c @srcdir@/../..


Tester64.exe : main64.o rest64.o npifc64.o npview.o
	wineg++ -g -o Tester64 -L. $(WINE64_LIBS) $(LDFLAGS) -Wall -Wextra $^

Tester.exe : main.o npview.o rest.o npifc.o
	wineg++ -g -o Tester -L. $(WINE_LIBS) $(LDFLAGS) -m32 -Wall -Wextra $^

main.o : main.cpp Makefile

main64.o : main.cpp Makefile

npview.o : npview.rc

rest.o : rest.c rest.h Makefile

rest64.o : rest.c rest.h Makefile

npifc.o : npifc.c npifc.h Makefile

npifc64.o : CFLAGS+="-DFOR_WIN64=1"
npifc64.o : npifc.c npifc.h Makefile

clean-local: clean-local-check
.PHONY: clean-local-check
clean-local-check:
	rm -f *.exe* *.dll* *.sh *.o

distclean-local: distclean-local-check
.PHONY: distclean-local-check
distclean-local-check:
	rm -f *.exe* *.dll* *.sh *.o

EXTRA_DIST = main.cpp npifc.c npifc.h resource.h rest.c rest.h

