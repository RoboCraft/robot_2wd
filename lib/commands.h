//
//
// available commands
//
// robocraft.ru
//

#ifndef _COMMANDS_H_
#define _COMMANDS_H_

#include "types.h"

#define SIG_SIZE 6

typedef struct CmdAcknowledgment {
    char sig[SIG_SIZE]; // "ackmnt"
    int32_t code;

} CmdAcknowledgment;

typedef struct CmdDrive2WD {
    char sig[SIG_SIZE]; // "drive2"
    int16_t pwm[2];

} CmdDrive2WD;

typedef struct CmdSpeech {
    char sig[SIG_SIZE]; // "speech"
    uint32_t code;
    char name[64];

} CmdSpeech;

typedef struct CmdTelemetry2WD {
    char sig[SIG_SIZE]; // "tlmtry"
    uint8_t Bamper;
    int16_t pwm[2];
    uint32_t US;
    uint32_t IR;
    uint32_t Voltage;

} CmdTelemetry2WD;

#endif //#ifndef _COMMANDS_H_
