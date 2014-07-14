#
# Makefile
#

CC 		:= gcc
CCPP 	:= g++

OBJECTSC := $(patsubst %.c, %.o, $(wildcard *.c))
OBJECTSCPP := $(patsubst %.cpp, %.o, $(wildcard *.cpp))

TARGET	:= $(patsubst %.o, %, $(OBJECTSC)) $(patsubst %.o, %, $(OBJECTSCPP)) 

CFLAGS		:= -I. -DLINUX=1
LIBRARIES	:= -lpthread -lrt

.PHONY: all clean

all: $(OBJECTSC) $(OBJECTSCPP)

%.o: %.c
	$(CC) $(CFLAGS) -o $(patsubst %.o, %, $@) $< $(LIBRARIES)

%.o: %.cpp
	$(CCPP) $(CFLAGS) -o $(patsubst %.o, %, $@) $< $(LIBRARIES)

clean:
	rm -f *.o $(TARGET)

targets:
	echo "[i] targets: $(TARGET)"

rebuild:
	make clean
	make
