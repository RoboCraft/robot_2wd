//
// definitions for robopet
//
//
// http://robocraft.ru
//

#ifndef _PET_H_
#define _PET_H_

#include "commands.h"
#include "robopet.h"

#include "orcp2/orcp2.h"
#include "lib/serial.h"
#include "lib/times.h"
#include "lib/console.h"
#include "orcp2/robot_2wd.h"

#include "lib/roboipc.h"

#define US_DIST_NUMBER 2

class Pet
{
public:
    Pet();
    ~Pet();

    int init();
    int make();

    enum { ST_NA=0,
           ST_INIT,
           ST_MOVE_FORWARD, ST_MOVE_LEFT, ST_MOVE_RIGHT, ST_MOVE_BACKWARD, ST_STOP,
           ST_SEARCH
         };
    int state;

    int make_state();

protected:

    roboipc::CommunicatorClient robodriver;
    roboipc::CommunicatorClient speecher;

    CmdDrive2WD cmd_drive_2wd;
    CmdTelemetry2WD cmd_telemetry_2wd;
    CmdSpeech cmd_speech;
    CmdAcknowledgment cmd_ack;

    char cmd_buf[256];
    int cmd_buf_size;

    float us_measurements[US_DIST_NUMBER];
    int us_counter;
    float us_distance;
    bool is_us_valid_data;

    bool is_speech_end;
};

#endif //#ifndef _PET_H_
