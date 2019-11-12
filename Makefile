# Makefile  -  by Martin Ankerl
#
# $Id: Makefile,v 1.9 2002/03/09 15:30:20 martin Exp $

CC      = g++      # compiler
CCFLAGS = -O3 -fomit-frame-pointer
#CCFLAGS = -g  # compileflags
LD      = g++  # linker

# list all filenames for .c and .o files 

CFILES_QMARS = code.cpp main.cpp
OFILES_QMARS = code.o   main.o

CFILES_GEN = code.cpp genbigswitch.cpp
OFILES_GEN = code.o   genbigswitch.o

# clean does not produce a file, so it has to be phony.
.PHONY : clean

# describe how to compile .c-files to .o
.cpp.o:
	$(CC) $(CCFLAGS) -c $<

#########################
# dependencies

all: clean gen qmars

qmars: $(OFILES_QMARS)  # first all .o-files must be available
	$(LD) -o $@ $(OFILES_QMARS) $(LDFLAGS)

gen: $(OFILES_GEN)
	$(LD) -o $@ $(OFILES_GEN) $(LDFLAGS)
	./gen >sw.cpp

# remove files not necessary
clean:
	rm -f $(OFILES_GEN) $(OFILES_QMARS) pathname.core *~

# automatically generate dependencies
depend:
	makedepend $(CFILES_QMARS) $(CFILES_GEN)
# DO NOT DELETE

code.o: code.hpp
main.o: warrior.hpp code.hpp qmars.hpp fqueue.hpp sw.cpp parser.hpp
code.o: code.hpp
genbigswitch.o: code.hpp
