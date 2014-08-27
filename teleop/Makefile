#
# Makefile
#
# http://www.gnu.org/software/make/manual/make.html
#

CC		:= g++

LIBPPATH 	:= ../lib
ORCPPATH 	:= ../orcp2

OBJECTS := $(ORCPPATH)/orcp2.o $(ORCPPATH)/robot_2wd.o $(ORCPPATH)/robot_sensors.o $(LIBPPATH)/serial.o $(LIBPPATH)/times.o $(LIBPPATH)/console.o $(LIBPPATH)/network.o $(LIBPPATH)/roboipc.o $(LIBPPATH)/mutex.o $(LIBPPATH)/thread.o

OBJECTSCPP := $(patsubst %.cpp, %.o, $(wildcard *.cpp))

TARGETS	:= $(patsubst %.o, %, $(OBJECTSCPP))

ADDOBJECTS := 

CFLAGS		:= -I. -I.. -I$(ORCPPATH) -I$(LIBPPATH) -DLINUX=1 

LIBRARIES	:= -lpthread -lrt

.PHONY: all clean

all: $(OBJECTSCPP)
	
pet.o: pet.cpp
	make -C $(LIBPPATH)
	make -C $(ORCPPATH)
	$(CC) $(CFLAGS) $(OBJECTS) -c -o $@ $< $(LIBRARIES)

teleop.o: teleop.cpp
	make -C $(LIBPPATH)
	make -C $(ORCPPATH)
	$(CC) $(CFLAGS) $(OBJECTS) $(ADDOBJECTS) -o $(patsubst %.o, %, $@) $< $(LIBRARIES)	
	
%.o: %.cpp
	make -C $(LIBPPATH)
	make -C $(ORCPPATH)
	$(CC) $(CFLAGS) $(OBJECTS) -o $(patsubst %.o, %, $@) $< $(LIBRARIES)
		
clean:
	make -C $(LIBPPATH) clean
	make -C $(ORCPPATH) clean
	rm -f *.o $(TARGETS)
	
targets:
	echo "[i] targets: $(TARGETS)"	
	
rebuild:
	make clean
	make
