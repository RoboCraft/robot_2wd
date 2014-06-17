#
# Makefile
#
# http://www.gnu.org/software/make/manual/make.html
#

CC		:= g++
TARGET	:= robot_2wd

OBJECTS := orcp2.o serial.o times.o console.o network.o robot_2wd.o robot_sensors.o

CFLAGS		:= -I. -DLINUX=1 

LIBRARIES	:= -lpthread -lrt

.PHONY: all clean

all: $(TARGET)

$(TARGET): $(OBJECTS)
	$(CC) $(CFLAGS) -o $(TARGET) $(OBJECTS) main.cpp $(LIBRARIES)

%.o: %.cpp
	$(CC) $(CFLAGS) -c -o $@ $< $(LIBRARIES)
	
clean:
	rm -f *.o $(TARGET)
	
rebuild:
	make clean
	make
