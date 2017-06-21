#\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\-*-Makefile-*-
#                         \\                                                 \
#  Giovanni A. Squillero  //  Via Beaumont, 27  -  10138 TORINO  -  ITALY    /
#                         \\  +39-0114345498  (Roma: +39-065816242)          \
#           (!)           //  squillero@polito.it                            /
#                         \\                                                 \
#/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/
#
#   $Source: /cadtools/program/Fenice-3.x/RCS/Makefile,v $
# $Revision: 1.2 $
#     $Date: 2000/03/15 14:25:56 $ 
#
#/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\
#                                                                            \
#   #####       Permission is granted to anyone  to use this software for    /
#  ######       any purpose  on  any computer system,  and  to  alter and    \
#  ###   \      redistribute the same.                                       /
#   ##G  c\     Because this software  is not copyrighted  and it is free    \
#   #     _\    of charge, there  is absolutely no warranty  on it.  Thus    /
#   |   _/      the author is not responsible for the consequences of use    \
#   |  _/       of this software, no matter how awful, even if they arise    /
#               from flaws in it.                                            \
#	                                                                     /
#\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/

# Some are standards
CC = gcc
CP = cp -p
INSTALL = /usr/sbin/install
LN = ln -sf
MKDIR = mkdir
PRINT = echo
RM = rm -f
RMDIR = rm -Rf
SED = sed
SHELL = /bin/sh

# Prefixes used in constructing the default values of the variables listed below.
prefix = /cadtools/program
#prefix = /home/tesi/squillero
exec_prefix = $(prefix)

# Directory for installing executable programs that users can run.
bindir = $(exec_prefix)/bin

# Directory for installing the man pages.
mandir = $(prefix)/man/man1

# Directory for installing libs
libdir = $(prefix)/lib

# Directory for installing includes
incdir = $(prefix)/include

# Directory for installing distribution tar file
distdir = $(prefix)/sources

# -----------------------------------------------------------------------------

# Local include and lib directory (for fault2.h and create...)
LIBPATH = -L. -L/cadtools/program/lib
INCLUDE = -I. -I/cadtools/program/include

# -----------------------------------------------------------------------------

# Options
WARNINGS = -Winline -Wchar-subscripts -Wcomment -Wformat -Wimplicit \
	-Wreturn-type -Wunused -Wuninitialized

STANDARD = $(WARNINGS) $(INCLUDE) -DVALUE_3 -g
OPTIMIZE = -O99 -finline-functions -funroll-loops
DEBUG = -DDEBUG

CFLAGS = $(STANDARD) $(DEBUG)
CFLAGS_D = $(STANDARD) $(DEBUG)
CFLAGS_O = $(STANDARD) $(OPTIMIZE)

# -----------------------------------------------------------------------------
# -----------------------------------------------------------------------------
# -              NO MODIFICATIONS NEEDED AFTER THIS POINT                     -
# -----------------------------------------------------------------------------
# -----------------------------------------------------------------------------

VERSION = 3.65
MAKEFILE_DEPEND = Makefile.depend

# -----------------------------------------------------------------------------

default: exe
exe: seq2com get_pi get_po get_ff fau2nfau Molokh-3.6.optimized Molokh-3.6.debug cinfo 
test: testcm

all: TAGS exe libs $(MOLOKH_PURE)
new: realclean all

debug: libfenice-debug.a libcircuit-debug.a Molokh-3.6.debug
optimized: libfenice-optimized.a libcircuit-optimized.a Molokh-3.6.optimized

libs: libfenice-optimized.a libfenice-debug.a \
	libcircuit-debug.a libcircuit-optimized.a

.PHONY: cinfoedf

# -----------------------------------------------------------------------------

FENICE_FILES = Circuit.o Events.o Get.o gutils.o \
	MyClock.o Set.o SharedVars.o \
	Simulation.o FSimulation.o Buffers.o Compat.o \
	TransientFaults.o Debug.o FeniceVer.o
FENICE_FILES_DEB = $(FENICE_FILES:.o=_D.o)
FENICE_FILES_OPT = $(FENICE_FILES:.o=_O.o)
FENICE_HEADERS = Fenice.h Check.h DescrVal.h Events.h Faults.h SharedVars.h Simulation.h gutils.h
ALL_FENICE_FILES = $(FENICE_FILES:.o=.c) $(FENICE_HEADERS) 

libfenice-debug.a: $(FENICE_FILES_DEB) $(GARBAGE_COLLECTOR)
libfenice-optimized.a: $(FENICE_FILES_OPT) $(GARBAGE_COLLECTOR)

# -----------------------------------------------------------------------------

CMOD_FILES = CircMod.o gutils.o Debug.o
CMOD_HEADERS = CircuitMod.h
ALL_CMOD_FILES = $(CMOD_FILES:.o=.c) $(CMOD_HEADERS) CircuitMod.doc

libcircuit-debug.a: $(CMOD_FILES:.o=_D.o)
libcircuit-optimized.a: $(CMOD_FILES:.o=_O.o)

# -----------------------------------------------------------------------------

MOLOKH_FILES = Molokh.o
MOLOKH_FILES_OPT = $(MOLOKH_FILES:.o=_O.o)
MOLOKH_FILES_DEB = $(MOLOKH_FILES:.o=_D.o)

ALL_MOLOKH_FILES = $(MOLOKH_FILES:.o=.c) molokh.1

Molokh-3.6.optimized: $(MOLOKH_FILES_OPT) libfenice-optimized.a libcircuit-optimized.a
Molokh-3.6.debug: $(MOLOKH_FILES_DEB) $(FENICE_FILES_DEB) $(CMOD_FILES:.o=_D.o) $(GARBAGE_COLLECTOR)

Molokh-3.6.pure: $(MOLOKH_FILES_DEB) $(FENICE_FILES_DEB) $(CMOD_FILES:.o=_D.o) $(GARBAGE_COLLECTOR)

# -----------------------------------------------------------------------------

DISTFILES = $(ALL_FENICE_FILES) \
	$(ALL_MOLOKH_FILES) \
	$(ALL_CMOD_FILES) \
	cinfo.c seq2com.c get_pi.c get_po.c get_ff.c fau2nfau.c \
	SimpleSimulator.c \
	ReachedStates.c \
	StateExample.c \
	Makefile Makefile.depend RIFIUTO_DI_RESPONSABILITA

# -----------------------------------------------------------------------------

# assembler
%.S: %.c
	$(CC) $(CFLAGS) -fverbose-asm -S -o $@ $<

%_O.S: %.c
	$(CC) $(CFLAGS_O) -fverbose-asm -S -o $@ $<

%_D.S: %.c
	$(CC) $(CFLAGS_D) -fverbose-asm -S -o $@ $<

# pre-process only
%.E: %.c
	$(CC) $(CFLAGS) -C -E -o $@ $<

# Debug version
%_D.o: %.c
	$(CC) $(CFLAGS_D) -c -o $@ $<

# Optimized version
%_O.o: %.c
	$(CC) $(CFLAGS_O) -c -o $@ $<

# Libraries
lib%.a:
	ar ruv $@ $^
	ranlib $@

# Tar directory
%.tar: %
	tar cf $@ $^
	@chmod 644 $@

# Gzip
%.tgz: %.tar
	gzip -c9 $< > $@
	@chmod 644 $@

# Shar
%.shar:	%
	shar -V $< > $@
	@chmod 644 $@

# -----------------------------------------------------------------------------

FeniceVer_D.o: $(ALL_FENICE_FILES) 
	@$(PRINT) "Setting version info for debug library"
	@$(CC) $(CFLAGS_D) -DFENICE_VERSION='"$(VERSION) [debug]"' -c -o $@ FeniceVer.c

FeniceVer_O.o: $(ALL_FENICE_FILES) 
	@$(PRINT) "Setting version info for optimized library"
	@$(CC) $(CFLAGS_O) -DFENICE_VERSION='"$(VERSION)"' -c -o $@ FeniceVer.c

# -----------------------------------------------------------------------------

testcm: testcm_D.o
	$(CC) $(CFLAGS) $(LIBPATH) -o $@ $< -lcircuit-debug -lcreate

get_pi: get_pi_D.o  $(FENICE_FILES_DEB) $(CMOD_FILES:.o=_D.o)
	$(CC) $(CFLAGS_D) $(LIBPATH) -o $@ $^ -lfau -lcreate

get_po: get_po_D.o  $(FENICE_FILES_DEB) $(CMOD_FILES:.o=_D.o)
	$(CC) $(CFLAGS_D) $(LIBPATH) -o $@ $^ -lfau -lcreate

get_ff: get_ff_D.o  $(FENICE_FILES_DEB) $(CMOD_FILES:.o=_D.o)
	$(CC) $(CFLAGS_D) $(LIBPATH) -o $@ $^ -lfau -lcreate

seq2com: seq2com_D.o $(FENICE_FILES_DEB) $(CMOD_FILES:.o=_D.o)
	$(CC) $(CFLAGS_D) $(LIBPATH) -o $@ $^  -ltermcap -lfau -lcreate

fau2nfau: fau2nfau_O.o
	$(CC) $(CFLAGS_O) $(LIBPATH) -o $@ $< -ltermcap -lfau -lfenice-optimized -lcreate

#ReachedStates: ReachedStates_D.o $(FENICE_FILES_DEB) $(CMOD_FILES:.o=_D.o)
#	$(CC) $(CFLAGS_D) $(LIBPATH) -o $@ $^ -lfau -lcreate -lcudd -lst -lutil -lmtr -lm
ReachedStates: ReachedStates_O.o $(FENICE_FILES_OPT) $(CMOD_FILES:.o=_O.o)
	$(CC) $(CFLAGS_O) $(LIBPATH) -o $@ $^ -lfau -lcreate -lcudd -lst -lutil -lmtr -lm

SimpleSimulator: SimpleSimulator_D.o $(FENICE_FILES_DEB) $(CMOD_FILES:.o=_D.o)
	$(CC) $(CFLAGS_D) $(LIBPATH) -o $@ $^ -lfau -lcreate

cinfo: cinfo_O.o 
	$(CC) $(CFLAGS_D) $(LIBPATH) -o $@ $^ \
		-ltermcap -lcircuit-optimized -lfenice-optimized -lcreate -lfau

cinfoedf: cinfo_D.o 
	purify $(CC) $(CFLAGS_D) $(LIBPATH) -o $@ $^ \
		../create_edf/create_edf.a \
		-ltermcap -lcircuit-debug -lfenice-debug -lfau

cinfo-debug: cinfo_D.o $(FENICE_FILES_DEB) $(CMOD_FILES:.o=_D.o)
	$(CC) $(CFLAGS_D) $(LIBPATH) -o $@ $^ -ltermcap -lcreate -lfau

StateExample: StateExample_D.o libfenice-debug.a libcircuit-optimized.a
	$(CC) $(CFLAGS_D) $(LIBPATH) -o $@ $^ -lcreate -lfau

TestFF: TestFF_O.o $(FENICE_FILES_OPT)
	$(CC) $(CFLAGS_D) $(LIBPATH) -o $@ $^ -lfau -lcircuit -lcreate

Molokh-3.6.profile:
	$(CC) -g -pg $(CFLAGS_O) $(LIBPATH) -o $@ \
		$(MOLOKH_FILES:.o=.c) \
		$(FENICE_FILES:.o=.c) \
		CircMod.c \
		-ltermcap -lcreate

Molokh-3.6.optimized:
	$(CC) $(CFLAGS_O) $(LIBPATH) -o $@ $(MOLOKH_FILES_OPT) \
		-ltermcap -lcircuit-optimized -lfenice-optimized -lcreate -lfau

Molokh-3.6.debug:
	$(CC) $(CFLAGS_D) $(LIBPATH) -o $@ $^ -ltermcap -lcreate -lfau

Molokh-3.6.pure:
	purify -cache-dir=$(HOME)/.purify-cache \
		$(CC) $(CFLAGS_D) $(LIBPATH) -o $@ $^ -ltermcap -lcreate -lfau

TAGS: $(ALL_FENICE_FILES) $(ALL_MOLOKH_FILES) $(ALL_CMOD_FILES)
	etags $^

# -----------------------------------------------------------------------------

mostlyclean:
	-$(RM) Molokh-3.6.debug $(MOLOKH_FILES) $(MOLOKH_FILES_DEB) $(MOLOKH_FILES_OPT)

clean: mostlyclean
	-$(RM) seq2com get_pi get_po get_ff fau2nfau Molokh-3.6.optimized \
		libfenice-optimized.a libfenice-debug.a \
		$(FENICE_FILES) $(FENICE_FILES_DEB) $(FENICE_FILES_OPT) \
		$(CMOD_FILES) $(CMOD_FILES:.o=_D.o) $(CMOD_FILES:.o=_O.o)

realclean: clean distclean
	-$(RM) TAGS
	-$(RM) libcircuit-optimized.a libcircuit-debug.a
	-$(RM) $(FENICE_FILES:.o=.S) $(FENICE_FILES_DEB:.o=.S) $(FENICE_FILES_OPT:.o=.S)
	-$(RM) $(MOLOKH_FILES:.o=.S) $(MOLOKH_FILES_DEB:.o=.S) $(MOLOKH_FILES_OPT:.o=.S)
	-$(RM) $(CMOD_FILES:.o=.S) $(CMOD_FILES:.o=_D.S) $(CMOD_FILES:.o=_O.S)
	-$(RM) $(FENICE_FILES:.o=.E) $(FENICE_FILES_DEB:.o=.E) $(FENICE_FILES_OPT:.o=.E)
	-$(RM) $(MOLOKH_FILES:.o=.E) $(MOLOKH_FILES_DEB:.o=.E) $(MOLOKH_FILES_OPT:.o=.E)
	-$(RM) $(CMOD_FILES:.o=.E) $(CMOD_FILES:.o=_D.E) $(CMOD_FILES:.o=_O.E)
	-$(RM) seq2com.o seq2com.E seq2com.S get_pi.o get_po.o get_ff.o fau2nfau.o get_pi.E get_po.E fau2nfau.E \
		get_pi.S get_po.S fau2nfau.S

distclean: 
	-$(RM) Fenice-$(VERSION).tgz
	-$(RM) Fenice-$(VERSION).shar
	-$(RMDIR) Fenice-$(VERSION)

# -----------------------------------------------------------------------------

Fenice-$(VERSION): $(DISTFILES)
	@$(PRINT) Making distribution directory...
	@$(INSTALL) -m 775 -d Fenice-$(VERSION)
	-@$(CP) -f $(DISTFILES) Fenice-$(VERSION) 
	chmod  644 $(DISTFILES)
	chmod 0444 Fenice-$(VERSION)/RIFIUTO_DI_RESPONSABILITA

dist: Fenice-$(VERSION).tgz Fenice-$(VERSION).shar

install: installlibs installexe

installlibs: libs
	-$(RM) $(libdir)/libfenice-optimized.a
	$(INSTALL) -m 664 -f $(libdir) libfenice-optimized.a
	-$(RM) $(libdir)/libfenice-debug.a
	$(INSTALL) -m 664 -f $(libdir) libfenice-debug.a
	-$(RM) $(libdir)/libcircuit-optimized.a 
	$(INSTALL) -m 664 -f $(libdir) libcircuit-optimized.a 
	-$(RM) $(libdir)/libcircuit-debug.a 
	$(INSTALL) -m 664 -f $(libdir) libcircuit-debug.a 
	ranlib $(libdir)/libfenice-optimized.a \
		$(libdir)/libfenice-debug.a \
		$(libdir)/libcircuit-optimized.a \
		$(libdir)/libcircuit-debug.a
	-$(RM) $(libdir)/libfenice.a
	$(LN) $(libdir)/libfenice-optimized.a $(libdir)/libfenice.a
	-$(RM) $(libdir)/libcircuit.a
	$(LN) $(libdir)/libcircuit-optimized.a $(libdir)/libcircuit.a
	-$(RM) $(incdir)/gutils.h
	$(INSTALL) -m 664 -f $(incdir) gutils.h
	-$(RM) $(incdir)/CircuitMod.h 
	$(INSTALL) -m 664 -f $(incdir) CircuitMod.h 
	-$(RM) $(incdir)/Check.h 
	$(INSTALL) -m 664 -f $(incdir) Check.h 
	-$(RM) $(incdir)/Fenice.h
	$(INSTALL) -m 664 -f $(incdir) Fenice.h
	-$(RM) $(incdir)/DescrVal.h
	$(INSTALL) -m 664 -f $(incdir) DescrVal.h
	-$(RM) $(incdir)/Faults.h
	$(INSTALL) -m 664 -f $(incdir) Faults.h
	-$(RM) $(libdir)/gutils_O.o
	$(INSTALL) -m 664 -f $(libdir) gutils_O.o
	-$(RM) $(libdir)/gutils_D.o 
	$(INSTALL) -m 664 -f $(libdir) gutils_D.o 
	-$(RM) $(libdir)/gutils.o
	$(LN) $(libdir)/gutils_O.o $(libdir)/gutils.o

installexe: exe
	-$(RM) $(bindir)/Molokh-3.6.debug
	$(INSTALL) -m 775 -f $(bindir) Molokh-3.6.debug
	-$(RM) $(bindir)/Molokh-3.6.optimized
	$(INSTALL) -m 775 -f $(bindir) Molokh-3.6.optimized
	-$(RM) $(bindir)/molokh
	$(LN) $(bindir)/Molokh-3.6.optimized $(bindir)/molokh
	-$(RM) $(mandir)/molokh.1
	$(INSTALL) -m 664 -f $(mandir) molokh.1
	-$(RM) $(bindir)/get_pi
	$(INSTALL) -m 775 -f $(bindir) get_pi
	-$(RM) $(bindir)/get_po
	$(INSTALL) -m 775 -f $(bindir) get_po
	-$(RM) $(bindir)/get_ff
	$(INSTALL) -m 775 -f $(bindir) get_ff
	-$(RM) $(bindir)/seq2com
	$(INSTALL) -m 775 -f $(bindir) seq2com
	-$(RM) $(bindir)/cinfo
	$(INSTALL) -m 775 -f $(bindir) cinfo

uninstall:
	-$(RM) $(libdir)/libfenice-debug.a 
	-$(RM) $(libdir)/libfenice-optimized.a
	-$(RM) $(libdir)/libfenice.a 
	-$(RM) $(libdir)/libcircuit-debug.a 
	-$(RM) $(libdir)/libcircuit-optimized.a
	-$(RM) $(incdir)/Fenice.h $(incdir)/Faults.h $(incdir)/Check.h
	-$(RM) $(incdir)/CircuitMod.h
	-$(RM) $(bindir)/Molokh-3.6
	-$(RM) $(bindir)/Molokh-3.6.debug
	-$(RM) $(bindir)/Molokh-3.6.optimized
	-$(RM) $(mandir)/molokh.1

depend:
	@$(PRINT) "Building dependencies ...";
	@-$(RM) $(MAKEFILE_DEPEND).tmp
	@for src in `echo $(FENICE_FILES) $(MOLOKH_FILES) $(CMOD_FILES) " " | $(SED) -e 's/\.o[ \t\n]/.c /g`; do \
	    $(PRINT) -n "   " $$src ": "; \
	    $(CC) -MM -DFENICE_VERSION='"[dependencies]"' $(CFLAGS) $$src >> $(MAKEFILE_DEPEND).tmp; \
	    $(PRINT) " done."; \
	done;
	@$(PRINT) "Commiting dependencies to $(MAKEFILE_DEPEND)..."
	@-$(RM) -f $(MAKEFILE_DEPEND)
	@$(PRINT) "# make depend started on" `hostname` "on" `date` > $(MAKEFILE_DEPEND)
	@$(SED) -e 's/\.o/_D.o/g' $(MAKEFILE_DEPEND).tmp >> $(MAKEFILE_DEPEND)
	@$(SED) -e 's/\.o/_O.o/g' $(MAKEFILE_DEPEND).tmp >> $(MAKEFILE_DEPEND)
	@-$(RM) $(MAKEFILE_DEPEND).tmp
	@chmod 644 $(MAKEFILE_DEPEND)
	@$(PRINT) "Done."

#
# Grab dependencies.  Note that the file $(MAKEFILE_DEPEND) *must*
# exist, even if empty.
#
include $(MAKEFILE_DEPEND)
