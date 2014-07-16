//
//
// robopet commands
//
// http://robocraft.ru
//

#ifndef _ROBOPET_COMMANDS_H_
#define _ROBOPET_COMMANDS_H_

#include "lib/commands.h"

typedef struct CmdDrive2WD {
    char sig[CMD_SIG_SIZE]; // "drive2"
    int16_t pwm[2];

} CmdDrive2WD;

typedef struct CmdSpeech {
    char sig[CMD_SIG_SIZE]; // "speech"
    uint32_t code;
    char name[64];

} CmdSpeech;

typedef struct CmdTelemetry2WD {
    char sig[CMD_SIG_SIZE]; // "tlmtry"
    uint8_t Bamper;
    int16_t pwm[2];
    uint32_t US;
    uint32_t IR;
    uint32_t Voltage;

} CmdTelemetry2WD;

#endif //#ifndef _ROBOPET_COMMANDS_H_
