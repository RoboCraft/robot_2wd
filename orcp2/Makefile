#
# Makefile
#
# http://www.gnu.org/software/make/manual/make.html
#

CC		:= g++
TARGET	:= robot_2wd
TARGETS := $(TARGET) keydrive

OBJECTS := orcp2.o serial.o times.o console.o network.o robot_2wd.o robot_sensors.o

CFLAGS		:= -I. -DLINUX=1 

LIBRARIES	:= -lpthread -lrt

.PHONY: all clean

all: $(TARGETS)

$(TARGET): $(OBJECTS)
	$(CC) $(CFLAGS) -o $(TARGET) $(OBJECTS) main.cpp $(LIBRARIES)

keydrive:  $(OBJECTS)
	$(CC) $(CFLAGS) -o keydrive $(OBJECTS) keydrive.cpp $(LIBRARIES)

%.o: %.cpp
	$(CC) $(CFLAGS) -c -o $@ $< $(LIBRARIES)
	
clean:
	rm -f *.o $(TARGETS)
	
rebuild:
	make clean
	make
