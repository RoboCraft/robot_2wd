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

#define US_MIN_DISTANCE 30

#define IR_MIN_DISTANCE 4

class Pet
{
public:

    // pet states:
    enum
    {
        State_NA = -1,
        State_deactivate,
        State_init,
        State_activate,
        State_look,
        State_search
    };

    enum { ST_NA = -1,
           ST_MOVE_FORWARD, ST_MOVE_LEFT, ST_MOVE_RIGHT, ST_MOVE_BACKWARD, ST_STOP
         };


    // emotions
    enum
    {
        Emotion_NA = -1,
        Emotion_calmness,
        Emotion_smiling,
        Emotion_yawning,
        Emotion_sniffing, // фырканье
        Emotion_angry,
        Emotion_fatigue, // усталость
        Emotion_attention,
        Emotion_sleepy
    };

    // behaviour
    enum
    {
        Behaviour_NA = -1,
        Behaviour_sleepy,
        Behaviour_calmness,
        Behaviour_curiosity,
        Behaviour_playfulness,
        Behaviour_joy,
        Behaviour_caution,
        Behaviour_amazement,
        Behaviour_tiredness,
        Behaviour_fear,
        Behaviour_anger,
        Behaviour_indisposition,
        Behaviour_hunger,
        Behaviour_philanthropy
    };

    Pet();
    ~Pet();

    int init();
    int make();

    int state;
    int emotion;

    int search_state;
    DWORD time_mark;

    DWORD blink_time;
    unsigned int blink_counter;

    int speed;

    int make_blink();
    int make_state();
    int make_search_state();

    int drive(int16_t pwm1, int16_t pwm2);
    int stop();
    int forward(int16_t speed);
    int backward(int16_t speed);
    int left(int16_t speed);
    int right(int16_t speed);

protected:

    roboipc::CommunicatorClient robodriver;
    roboipc::CommunicatorClient speecher;

    CmdDrive2WD cmd_drive_2wd;
    CmdTelemetry2WD cmd_telemetry_2wd;
    CmdDigitalWrite cmd_digital_write;
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
