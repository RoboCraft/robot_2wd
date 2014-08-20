//
// robot pet
//
// http://robocraft.ru
//

#include "pet.h"

#include <stdio.h>
#include <stdlib.h>

Pet::Pet()
{
    state = State_NA;
#if defined(LINUX)
    srandom(time(0));
#endif //#if defined(LINUX)
}

Pet::~Pet()
{
    state = State_deactivate;
    stop();
}

int Pet::init()
{
    printf("[i][Pet][init]\n");

    if( robodriver.connect(DRIVER_2WD_SOCKET_NAME) ) {
        fprintf(stderr, "[!] Error: cant create communication: %s!\n", DRIVER_2WD_SOCKET_NAME);
        return -1;
    }

    if( speecher.connect(SPEECHER_SOCKET_NAME) ) {
        fprintf(stderr, "[!] Error: cant create communication: %s!\n", SPEECHER_SOCKET_NAME);
        //return -1;
    }

    memset(&cmd_drive_2wd, 0, sizeof(cmd_drive_2wd));
    memset(&cmd_telemetry_2wd, 0, sizeof(cmd_telemetry_2wd));
    memset(&cmd_digital_write, 0, sizeof(cmd_digital_write));

    memset(&cmd_speech, 0, sizeof(cmd_speech));
    memset(&cmd_ack, 0, sizeof(cmd_ack));

    memset(cmd_buf, 0, sizeof(cmd_buf));
    cmd_buf_size = 0;

    is_speech_end = true;

    state = State_init;
    emotion = 0;

    blink_time = 0;
    blink_counter = 0;

    return 0;
}

int Pet::make()
{
    printf("[i][Pet][make]\n");

    int res = 0;

    if(robodriver.available(50) > 0) {
        printf("[i] Client data available...\n");
        if( (cmd_buf_size = robodriver.read(cmd_buf, sizeof(cmd_buf))) > 0 ) {
            if(cmd_buf_size > CMD_SIG_SIZE) {
                if( !strncmp(cmd_buf, "tlmtry", CMD_SIG_SIZE) && cmd_buf_size >= sizeof(cmd_telemetry_2wd) ) {
                    memcpy(&cmd_telemetry_2wd, cmd_buf, sizeof(cmd_telemetry_2wd));
                    printf("[i] 2WD telemetry: Bmp: %d US: %d IR: [%d %d] PWM: [%d %d]\n",
                           cmd_telemetry_2wd.Bumper,
                           cmd_telemetry_2wd.US,
                           cmd_telemetry_2wd.IR[0], cmd_telemetry_2wd.IR[1],
                           cmd_telemetry_2wd.pwm[0], cmd_telemetry_2wd.pwm[1]);

                    us_measurement.add( cmd_telemetry_2wd.US );
                    if(us_measurement.is_valid) {
                        printf("[i] us_distance: %.2f\n", us_measurement.value);
                    }

                    ir_measurement_left.add(cmd_telemetry_2wd.IR[0]);
                    ir_measurement_right.add(cmd_telemetry_2wd.IR[1]);
                    if(ir_measurement_left.is_valid) {
                        printf("[i] ir_distance: %.2f %.2f\n", ir_measurement_left.value, ir_measurement_right.value);
                    }

                    if(is_speech_end && us_measurement.is_valid && us_measurement.value < US_NEAR_DISTANCE) {
                        if(speecher.sockfd != SOCKET_ERROR) {
                            is_speech_end = false;

                            strncpy(cmd_speech.sig, "speech", CMD_SIG_SIZE);
                            cmd_speech.code = 0;
                            strncpy(cmd_speech.name, SOUND_FOR_DETECTION, sizeof(cmd_speech.name));

                            if(us_measurement.value < US_MIN_DISTANCE) {
                                strncpy(cmd_speech.name, SOUND_FOR_NEAR, sizeof(cmd_speech.name));
                            }

                            res = speecher.write(&cmd_speech, sizeof(cmd_speech));
                            printf("[i] Send speech command (%d)...\n", res);
                            printf("[i] Data: %d %s\n", cmd_speech.code, cmd_speech.name);
                        }
                    }
                }

            }
        }
        else {
            printf("[i] Connection closed...\n");
            robodriver.close();
        }
    }

    if(speecher.available(50) > 0) {
        printf("[i] Speecher action...\n");
        if( (cmd_buf_size = speecher.read(cmd_buf, sizeof(cmd_buf))) > 0 ) {
            printf("[i] Data size: %d\n", cmd_buf_size);
            if(cmd_buf_size > CMD_SIG_SIZE) {
                if( !strncmp(cmd_buf, "ackmnt", CMD_SIG_SIZE) && cmd_buf_size >= sizeof(cmd_ack) ) {
                    memcpy(&cmd_ack, cmd_buf, sizeof(cmd_ack));
                    printf("[i] Clinet ACK: %d\n", cmd_ack.code);
                    is_speech_end = true;
                }
            }
        }
        else {
            printf("[i] Connection closed...\n");
            speecher.close();
        }
    }

    make_blink();

    make_state();

    return 0;
}

int Pet::make_blink()
{
    int res = 0;

    // Blink
    if(orv::time::is_time(blink_time, 1000)) {
        blink_counter++;
        int val = blink_counter%2;
        printf("[i] Blink: %d\n", val);

        if(robodriver.sockfd != SOCKET_ERROR) {
            strncpy(cmd_digital_write.sig, "dgtwrt", CMD_SIG_SIZE);

            cmd_digital_write.pin = 13; // L
            cmd_digital_write.value = (int16_t)val;

            res = robodriver.write(&cmd_digital_write, sizeof(cmd_digital_write));
        }
    }
    return res;
}

int Pet::make_state()
{
    printf("[i][Pet][make_state] State: %d\n", state);
    switch (state) {
    case State_NA:
        break;
    case State_init:
        state = State_activate;
        break;
    case State_activate:
        // after activation
        state = State_search;
        search_state = ST_MOVE_FORWARD;
        break;
    case State_search:
        make_search_state();
        break;
    default:
        printf("[i][Pet][make_state] Unknown state!\n");
        break;
    }
    return 0;
}

int Pet::make_search_state()
{
    printf("[i][Pet][make_search_state] State: %d\n", search_state);

    if( search_state != ST_MOVE_BACKWARD && search_state != ST_STOP ) {
        if( cmd_telemetry_2wd.Bumper ||
            (us_measurement.is_valid && us_measurement.value < US_CLOSE_DISTANCE) ||
            (ir_measurement_left.is_valid && ir_measurement_left.value <= IR_MIN_DISTANCE) ||
            (ir_measurement_right.is_valid && ir_measurement_right.value <= IR_MIN_DISTANCE) ) {
            printf("[i][Pet][make_search_state] STOP!\n");
            stop();
            search_state = ST_STOP;
        }
    }

    speed = 70;

    switch (search_state) {
    case ST_NA:
        break;
    case ST_MOVE_FORWARD:
        printf("[i][Pet][make_search_state] Forward!\n");
        forward(speed);
        break;
    case ST_STOP:
        printf("[i][Pet][make_search_state] Stop!\n");
        stop();
        time_mark = orv::time::millis();
        search_state = ST_MOVE_BACKWARD;
        break;
    case ST_MOVE_BACKWARD:
        printf("[i][Pet][make_search_state] Backward!\n");
        backward(speed);
        //printf("[i][Pet][make_search_state] %u %u\n", current_time.tv_sec, time_mark.tv_sec);
        if( (ir_measurement_left.is_valid && ir_measurement_left.value > IR_MIN_DISTANCE) &&
            (ir_measurement_right.is_valid && ir_measurement_right.value > IR_MIN_DISTANCE) ) {
            if(us_measurement.is_valid && us_measurement.value < US_CLOSE_DISTANCE) {
                break;
            }
            time_mark = orv::time::millis();
            search_state = ST_MOVE_RIGHT;
            if(rand() % 100 > 50) {
                search_state = ST_MOVE_LEFT;
            }
        }
        break;
    case ST_MOVE_RIGHT:
        printf("[i][Pet][make_search_state] Right!\n");
        right(speed);
        if( orv::time::is_time(time_mark, 1000) ) {
            time_mark = orv::time::millis();
            if(rand() % 100 > 90) {
                search_state = ST_MOVE_RIGHT;
            }
            else {
                search_state = ST_MOVE_FORWARD;
            }
        }
        break;
    case ST_MOVE_LEFT:
        printf("[i][Pet][make_search_state] Left!\n");
        left(speed);
        if( orv::time::is_time(time_mark, 1000) ) {
            time_mark = orv::time::millis();
            if(rand() % 100 > 90) {
                search_state = ST_MOVE_LEFT;
            }
            else {
                search_state = ST_MOVE_FORWARD;
            }
        }
        break;
    default:
        printf("[i][Pet][make_search_state] Unknown state!\n");
        break;
    }
    return 0;
}

int Pet::drive(int16_t pwm1, int16_t pwm2)
{
    int res = 0;

    if(robodriver.sockfd != SOCKET_ERROR) {
        strncpy(cmd_drive_2wd.sig, "drive2", CMD_SIG_SIZE);

        cmd_drive_2wd.pwm[0] = pwm1;
        cmd_drive_2wd.pwm[1] = pwm2;

        res = robodriver.write(&cmd_drive_2wd, sizeof(cmd_drive_2wd));
        //printf("[i] Send drive command (%d)...\n", res);
        //printf("[i] Set pwm to: [%d %d]\n", cmd_drive_2wd.pwm[0], cmd_drive_2wd.pwm[1]);
    }

    return res;
}

int Pet::stop()
{
    return drive(0, 0);
}

int Pet::forward(int16_t speed)
{
    return drive(speed, speed);
}

int Pet::backward(int16_t speed)
{
    return drive(-speed, -speed);
}

int Pet::left(int16_t speed)
{
    return drive(-speed, speed);
}

int Pet::right(int16_t speed)
{
    return drive(speed, -speed);
}
