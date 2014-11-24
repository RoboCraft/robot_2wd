#
# Makefile
#
# http://www.gnu.org/software/make/manual/make.html
#

CC		:= g++

LIBPPATH 	:= ../lib
ORCPPATH 	:= ../orcp2

OBJECTS := $(ORCPPATH)/orcp2.o $(ORCPPATH)/robot_2wd.o $(ORCPPATH)/robot_sensors.o $(LIBPPATH)/serial.o $(LIBPPATH)/times.o $(LIBPPATH)/console.o $(LIBPPATH)/network.o $(LIBPPATH)/roboipc.o

OBJECTSCPP := $(patsubst %.cpp, %.o, $(wildcard *.cpp))

TARGETS	:= $(patsubst %.o, %, $(OBJECTSCPP)) 

#TARGETSS := 2wd_driver speecher robopet eye test_2wd_telemetry test_speecher

ADDOBJECTS := pet.o

OPENCVFLAGS	:= -DUSEOPENCV=1 -I/usr/include/opencv -L/usr/lib -Wl,-rpath,/usr/lib
OPENCVLIBRARIES := -lopencv_core -lopencv_imgproc -lopencv_highgui -lopencv_video -lopencv_objdetect

CFLAGS		:= -I. -I.. -I$(ORCPPATH) -I$(LIBPPATH) -DLINUX=1 

LIBRARIES	:= -lpthread -lrt

.PHONY: all clean

all: $(OBJECTSCPP)

eye.o: eye.cpp
	make -C $(LIBPPATH)
	$(CC) $(CFLAGS) $(OBJECTS) $(OPENCVFLAGS) -o eye $< $(LIBRARIES) $(OPENCVLIBRARIES)

face_detect_woof.o: face_detect_woof.cpp
	make -C $(LIBPPATH)
	$(CC) $(CFLAGS) $(OBJECTS) $(OPENCVFLAGS) -o face_detect_woof $< $(LIBRARIES) $(OPENCVLIBRARIES)	
	
pet.o: pet.cpp
	make -C $(LIBPPATH)
	make -C $(ORCPPATH)
	$(CC) $(CFLAGS) $(OBJECTS) -c -o $@ $< $(LIBRARIES)

robopet.o: robopet.cpp
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
