#
# Makefile
#
# http://www.gnu.org/software/make/manual/make.html
#

CC		:= g++
TARGET	:= robot_2wd
TARGETS := $(TARGET) keydrive

LIBPPATH 	:= ../lib

OBJECTS := orcp2.o robot_2wd.o robot_sensors.o $(LIBPPATH)/serial.o $(LIBPPATH)/times.o $(LIBPPATH)/console.o $(LIBPPATH)/network.o

CFLAGS		:= -I. -I.. -I$(LIBPPATH) -DLINUX=1 

LIBRARIES	:= -lpthread -lrt

.PHONY: all clean

all: $(TARGETS)

$(TARGET): $(OBJECTS)
	make -C $(LIBPPATH)
	$(CC) $(CFLAGS) -o $(TARGET) $(OBJECTS) main.cpp $(LIBRARIES)

keydrive:  $(OBJECTS)
	make -C $(LIBPPATH)
	$(CC) $(CFLAGS) -o keydrive $(OBJECTS) keydrive.cpp $(LIBRARIES)

%.o: %.cpp
	$(CC) $(CFLAGS) -c -o $@ $< $(LIBRARIES)
	
clean:
	make -C $(LIBPPATH) clean
	rm -f *.o $(TARGETS)
	
rebuild:
	make clean
	make
