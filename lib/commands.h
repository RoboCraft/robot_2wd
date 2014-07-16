//
//
// available commands
//
// http://robocraft.ru
//

#ifndef _COMMANDS_H_
#define _COMMANDS_H_

#include "types.h"

#define CMD_SIG_SIZE 6

typedef struct CmdAcknowledgment {
    char sig[CMD_SIG_SIZE]; // "ackmnt"
    int32_t code;

} CmdAcknowledgment;

#endif //#ifndef _COMMANDS_H_
