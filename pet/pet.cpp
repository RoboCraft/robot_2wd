//
// robot pet
//
// http://robocraft.ru
//

#include "pet.h"

#include <stdio.h>

Pet::Pet()
{
    state = ST_NA;
}

Pet::~Pet()
{

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

    memset(&cmd_speech, 0, sizeof(cmd_speech));
    memset(&cmd_ack, 0, sizeof(cmd_ack));

    memset(cmd_buf, 0, sizeof(cmd_buf));
    cmd_buf_size = 0;

    is_speech_end = true;

    memset(us_measurements, 0, sizeof(us_measurements));
    us_counter = 0;
    us_distance = 0;
    is_us_valid_data = false;

    state = ST_INIT;

    return 0;
}

int Pet::make()
{
    printf("[i][Pet][make]\n");

    int res = 0;

    if(robodriver.available(50) > 0) {
        printf("[i] Client data available...\n");
        if( (cmd_buf_size = robodriver.read(cmd_buf, sizeof(cmd_buf)))>0 ) {
            if(cmd_buf_size > CMD_SIG_SIZE) {
                if( !strncmp(cmd_buf, "tlmtry", CMD_SIG_SIZE) && cmd_buf_size >= sizeof(cmd_telemetry_2wd) ) {
                    memcpy(&cmd_telemetry_2wd, cmd_buf, sizeof(cmd_telemetry_2wd));
                    printf("[i] 2WD telemetry: US: %d IR: [%d %d] PWM: [%d %d]\n", cmd_telemetry_2wd.US,
                           cmd_telemetry_2wd.IR[0], cmd_telemetry_2wd.IR[1],
                            cmd_telemetry_2wd.pwm[0], cmd_telemetry_2wd.pwm[1]);

                    if(us_counter < US_DIST_NUMBER) {
                        us_measurements[us_counter] = cmd_telemetry_2wd.US;
                        us_counter++;
                    }
                    else {
                        us_counter = 0;
                        is_us_valid_data = true;
                    }
                    if(is_us_valid_data) {
                        float summ = 0;
                        for(int i=0; i<US_DIST_NUMBER; i++) {
                            summ += us_measurements[i];
                        }
                        us_distance = summ/US_DIST_NUMBER;
                        printf("[i] us_distance: %.2f\n", us_distance);
                    }

                    if(is_speech_end && is_us_valid_data && us_distance < 50) {
                        if(speecher.sockfd != SOCKET_ERROR) {
                            is_speech_end = false;

                            strncpy(cmd_speech.sig, "speech", CMD_SIG_SIZE);
                            cmd_speech.code = 0;
                            strncpy(cmd_speech.name, SOUND_FOR_DETECTION, sizeof(cmd_speech.name));

                            if(us_distance < 30) {
                                strncpy(cmd_speech.name, SOUND_FOR_NEAR, sizeof(cmd_speech.name));
                            }

                            res = speecher.write(&cmd_speech, sizeof(cmd_speech));
                            printf( "[i] Send speech command (%d)...\n", res);
                            printf( "[i] Data: %d %s\n", cmd_speech.code, cmd_speech.name);
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
        if( (cmd_buf_size = speecher.read(cmd_buf, sizeof(cmd_buf)))>0 ) {
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

    make_state();

    return 0;
}

int Pet::make_state()
{
    printf("[i][Pet][make_state] State: %d\n", state);
    switch (state) {
    case ST_NA:
        break;
    case ST_INIT:
        state = ST_SEARCH;
        break;
    default:
        printf("[i][Pet][make_state] Unknown state!\n");
        break;
    }
    return 0;
}
