###########################################################################
#             #                                                           #
#   #####     #  Copyright (c) 2000 Giovanni Squillero                    #
#  ######     #  http://staff.polito.it/giovanni.squillero/               #
#  ###   \    #  giovanni.squillero@polito.it                             #
#   ##G  c\   #                                                           #
#   #     _\  #  This code is licensed under a BSD license.               #
#   |  _/     #  See <https://github.com/squillero/fenice> for details    #
#             #                                                           #
###########################################################################

SHELL=/bin/sh
CFLAGS=-I. -Icreate_edf -O1 -g -std=gnu90 -w -DVALUE_2
CC=gcc

VERSION = 3.65

all: molokh 
library: create_edf.a fenice.a

CREATE_EDF_OBJS = create_edf/celltable.o \
	create_edf/create_supp.o \
	create_edf/hash.o \
	create_edf/macro.o \
	create_edf/parse.o \
	create_edf/create.o \
	create_edf/ediflex.o \
	create_edf/library.o \
	create_edf/mem_alloc.o \
	create_edf/parse_supp.o \
	create_edf/tree_ops.o

CREATE_FAU_OBJS = create_fau/create_fau.o

FENICE_OBJS = Buffers.o \
	CircMod.o \
	Circuit.o \
	Compat.o \
	Debug.o \
	Events.o \
	FeniceVer.o \
	FSimulation.o \
	Get.o \
	Memory.o \
	Messages.o \
	Set.o \
	SharedVars.o \
	Simulation.o \
	TransientFaults.o

FeniceVer.o: $(ALL_FENICE_FILES) 
	@$(CC) $(CFLAGS) -DFENICE_VERSION='"$(VERSION)"' -c -o $@ FeniceVer.c

create_edf.a: ${CREATE_EDF_OBJS}
	ar ruv $@ $^
	ranlib $@

fenice.a: ${FENICE_OBJS}
	ar ruv fenice.a ${FENICE_OBJS}
	ranlib fenice.a

molokh: Molokh.o gutils.o create_edf.a fenice.a
	${CC} ${CFLAGS} -o $@ $^ -lcurses

clean: 
	rm -f ${CREATE_EDF_OBJS}
	rm -f ${FENICE_OBJS}
	rm -f test_create create_edf.a fenice.a
	rm -f molokh 

realclean: clean
	rm -f *~ create_edf/*~ create_fau/*~
	rm -f *.o create_edf/*.o create_fau/*.o
	rm -f *.a create_edf/*.a create_fau/*.a

# DEPS (via -MM)
