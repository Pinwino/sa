 # /**
 # ******************************************************************************* 
 # *  Jose Jimenez
 # *
 # *  Copyright (C) 2013
 # *
 # *******************************************************************************
 # *  This library is free software; you can redistribute it and/or
 # *  modify it under the terms of the GNU Lesser General Public
 # *  License as published by the Free Software Foundation; either
 # *  version 3 of the License, or (at your option) any later version.
 # *
 # *  This library is distributed in the hope that it will be useful,
 # *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 # *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 # *  Lesser General Public License for more details.
 # *  
 # *  You should have received a copy of the GNU Lesser General Public
 # *  License along with this library. If not, see <http://www.gnu.org/licenses/>.
 # *******************************************************************************

# I know, I knwo that is huge mess!!!
# I know I shouldn't be mixing standard c with c++. 
# Etherbone API was witten in standard C while CALoE mix C/C++. 
# 
# The RAM reprogammint tool uses acces_internals which is
#  written in C (?). The vuart thing is based on the cpp CALoE files
#

CC  ?= gcc
CPP = g++
DESTDIR ?= /usr/local
ETHERBONE_DIR = ./caloe/etherbone

# For standard C, we will be compiling only the stuff we really need
progs-cc-y = ethb-mem-tst 
progs-cc-y += ethb-cl

CFLAGS = -I -Icaloe/lib -Icaloe/devices -I. -Wno-trigraphs -O2 -Wall -ggdb
LFLAGS = -L. -l:$(ETHERBONE_DIR)/api/libetherbone.a

obj-y = caloe/lib/access_internals.o
OBJS = $(obj-y)

#CPP
OUTPUT_CPP = remote-vuart

LFLAGS_CPP = -L. -l:caloe/lib/libcaloe.a -l:$(ETHERBONE_DIR)/api/libetherbone.a

obj-cpp-y = caloe/devices/dio/Dio.o
obj-cpp-y += caloe/devices/vuart/Vuart.o
OBJS_CPP = $(obj-cpp-y)

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
	
# When using c++, I don't do much work (only very simple rules). As developed
# at the final stages I decided to keep it simple and relay on the author's 
# Makefiles, saving time.
# We will be compiling everthing and using the stuff we really need (sorry)
$(OUTPUT_CPP): build-etherbone
	$(MAKE) -C caloe/lib -f Makefile
	$(MAKE) -C caloe/devices -f Makefile
	$(MAKE) -C caloe/tools   -f Makefile
	$(CPP) $(CFLAGS) $(OUTPUT_CPP).cpp $(OBJS_CPP) $(LFLAGS_CPP) -o $@
