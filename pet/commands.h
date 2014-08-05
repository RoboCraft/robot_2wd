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
    uint32_t IR[2];
    uint32_t Voltage;

} CmdTelemetry2WD;

#define PET_EYE_COLORS_COUNT 3

typedef struct PetEyeColorData {
    int id;
    int count;
    int center_x;
    int center_y;
} PetEyeColorData;

typedef struct CmdEyeData {
    char sig[CMD_SIG_SIZE]; // "eyedat"

    int counter;
    int colors_count;
    PetEyeColorData colors[PET_EYE_COLORS_COUNT];

} CmdEyeData;

#endif //#ifndef _COMMANDS_H_
