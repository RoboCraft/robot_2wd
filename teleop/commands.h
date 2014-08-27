//
//
// available commands
//
// http://robocraft.ru
//

#ifndef _COMMANDS_H_
#define _COMMANDS_H_

#include "lib/types.h"

#define CMD_SIG_SIZE 6

typedef struct CmdAcknowledgment {
    char sig[CMD_SIG_SIZE]; // "ackmnt"
    int32_t code;

} CmdAcknowledgment;

//
// robopet commands
//

// Arduino/CraftDuino

typedef struct CmdTelemetry2WD {
    char sig[CMD_SIG_SIZE]; // "tlmtry"
    uint8_t Bumper;
    int16_t pwm[2];
    uint32_t US;
    uint32_t IR[2];
    uint32_t Voltage;

} CmdTelemetry2WD;

typedef struct CmdDrive2WD {
    char sig[CMD_SIG_SIZE]; // "drive2"
    int16_t pwm[2];

} CmdDrive2WD;

typedef struct CmdDigitalWrite {
    char sig[CMD_SIG_SIZE]; // "dgtwrt"
    int16_t pin;
    int16_t value;
} CmdDigitalWrite;

typedef struct CmdSpeech {
    char sig[CMD_SIG_SIZE]; // "speech"
    uint32_t code;
    char name[64];

} CmdSpeech;



#endif //#ifndef _COMMANDS_H_
