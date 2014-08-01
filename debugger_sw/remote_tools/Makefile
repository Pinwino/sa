# This work is part of the White Rabbit project
#
# Jose Jimenez  <jjimenez.wr@gmail.com>, Copyright (C) 2014.
# Released according to the GNU GPL version 3 (GPLv3) or later.
#
# Makefile to compile remote tools
#	ethb-cl		: Simple tool for RAM programming
#	ethb-mem-tst: Easy way to test RAM usage and optimize memory regions
#	remote-vuart: Implements virtual UART remote acces for configuration
#

#
# I know, I know that is a huge mess.
# I know I shouldn't be mixing standard c with c++. In my defense,
# Etherbone API was witten in standard C while CALoE mix C/C++. 


# ethb-cl & ethb-mem-tst are in standard C 
#
# This RAM-related tools us acces_internals which is
# written in C
#

CC  ?= gcc
CPP = g++
DESTDIR ?= /usr/local
ETHERBONE_DIR = ./caloe/etherbone

#
# For standard C tools, we will be compiling only the stuff we really need
# C
progs-cc-y = ethb-mem-tst 
progs-cc-y += ethb-cl

CFLAGS = -I -Icaloe/lib -Icaloe/devices -I. -Wno-trigraphs -O2 -Wall -ggdb
LFLAGS = -L. -l:$(ETHERBONE_DIR)/api/libetherbone.a

obj-y = caloe/lib/access_internals.o
OBJS = $(obj-y)
#/C

#CPP
OUTPUT_CPP = remote-vuart

LFLAGS_CPP = -L. -l:caloe/lib/libcaloe.a -l:$(ETHERBONE_DIR)/api/libetherbone.a

obj-cpp-y = caloe/devices/dio/Dio.o
obj-cpp-y += caloe/devices/vuart/Vuart.o
OBJS_CPP = $(obj-cpp-y)
#/CPP

all: $(progs-cc-y) $(OUTPUT_CPP)

clean:
	$(RM) -f $(progs-cc-y) $(OUTPUT_CPP) $(OBJS) $(OBJS_CPP) *.o *.a
	$(MAKE) -C $(ETHERBONE_DIR)/api -f Makefile clean
	$(MAKE) -C caloe/devices -f Makefile clean
	$(MAKE) -C caloe/lib     -f Makefile clean
	$(MAKE) -C caloe/tools   -f Makefile clean

$(progs-cc-y): $(OBJS)

install:
	install -d $(DESTDIR)/bin
	install -D $(progs-cc-y) $(DESTDIR)/bin
	install -D $(OUTPUT_CPP) $(DESTDIR)/bin

build-etherbone: git_etherbone caloe/etherbone
	$(MAKE) -C $(ETHERBONE_DIR)/api -f Makefile
	
%: %.c
	$(CC) $(CFLAGS) $*.c $(OBJS) $(LFLAGS) -o $@
	
%.o: build-etherbone     %.c
	$(CC) $(CFLAGS) -c $*.c -o $@
	
git_etherbone:
	@test -f ./caloe -d $(ETHERBONE_DIR)/api || echo "Checking out submodules"
	@test -f ./caloe -d $(ETHERBONE_DIR)/api || \
	$(MAKE) -C ./caloe -f Makefile etherbone-submodule

# The vuart thing is based on the cpp-wtritten CALoE files
#
# When using c++, I don't do much work (only very simple rules). I decided to 
# keep it simple and relay on the author's Makefiles, saving time.
#
# We will be compiling everthing and using the stuff we really need (sorry)
#

$(OUTPUT_CPP): build-etherbone
	$(MAKE) -C caloe/lib -f Makefile
	$(MAKE) -C caloe/devices -f Makefile
	$(MAKE) -C caloe/tools   -f Makefile
	$(CPP) -DCURPATH="\"$(CURDIR)"\" $(CFLAGS) $@.cpp $(OBJS_CPP) $(LFLAGS_CPP) -o $@
