#-------------------------------------------------
#
# Project created by QtCreator 2012-11-30T12:20:31
#
#-------------------------------------------------

QT       += 
#widgets

TARGET = keydrive
TEMPLATE = app

CONFIG += console

SOURCES += keydrive.cpp \
    pet.cpp \
#    robopet.cpp \
    ../orcp2/orcp2.cpp \
    ../orcp2/robot_2wd.cpp \
    ../orcp2/robot_sensors.cpp \
    ../lib/console.cpp \
    ../lib/network.cpp \
    ../lib/roboipc.cpp \
    ../lib/serial.cpp \
    ../lib/times.cpp

HEADERS  += commands.h \
    pet.h \
    robopet.h \
    ../orcp2/orcp2.h \
    ../orcp2/robot_2wd.h \
    ../orcp2/robot_sensors.h \
    ../lib/buffer.h \
    ../lib/console.h \
    ../lib/network.h \
    ../lib/roboipc.h \
    ../lib/serial.h \
    ../lib/stream.h \
    ../lib/times.h \
    ../lib/types.h \
    ../lib/filtration.h

#FORMS    += 

INCLUDEPATH +=  . \
                .. \
                ../lib
LIBS += -lws2_32
