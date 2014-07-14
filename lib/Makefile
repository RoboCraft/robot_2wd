#
# Makefile for make crossfunctions
#

CC		:= g++
TARGET	:= 

OBJECTS := $(patsubst %.cpp, %.o, $(wildcard *.cpp))

LIBPATH := ..

CFLAGS		:= -I. -I$(LIBPATH) -DLINUX=1
# flags for debug
CFLAGSD		:= -I. -I$(LIBPATH) -DLINUX=1 -O0 -g
# flags for release
CFLAGSR		:= -I. -I$(LIBPATH) -DLINUX=1 -O3

LIBRARIES	:= -lpthread -lrt


.PHONY: all clean

all: $(OBJECTS)

%.o: %.cpp
	$(CC) $(CFLAGS) -c -o $@ $< $(LIBRARIES)

debug:
	make CFLAGS="$(CFLAGSD)"

release:
	make CFLAGS="$(CFLAGSR)"
	
clean:
	rm -f *.o $(TARGET)
