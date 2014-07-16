//
// robot pet
//
// http://robocraft.ru
//

#include <stdio.h>
#include <stdlib.h>

#include "robopet.h"

#include "orcp2/orcp2.h"
#include "lib/serial.h"
#include "lib/times.h"
#include "lib/console.h"
#include "orcp2/robot_2wd.h"

#include "lib/roboipc.h"

#if defined(LINUX)
# include <unistd.h>
# include <sys/types.h>
# include <sys/wait.h>
# include <signal.h>
#endif

#if defined(LINUX)

volatile sig_atomic_t terminated = 0;
volatile sig_atomic_t is_sigpipe = 0;

void signal_handler(int sig)
{
    if(sig == SIGINT) {
        terminated = 1;
    }
    if(sig == SIGPIPE) {
        is_sigpipe = 1;
    }
}
#endif //#if defined(LINUX)

void register_signal_handlers()
{
#if defined(LINUX)
    // register signal handler for <CTRL>+C in order to clean up
    if (signal(SIGINT, signal_handler) == SIG_ERR) {
        printf("[i] Error: register signal handler!\n");
    }

    if (signal(SIGPIPE, signal_handler) == SIG_ERR) {
        printf("[i] Error: register signal handler!\n");
    }
#endif //#if defined(LINUX)
}

int main(int argc, char* argv[])
{
    printf("[i] Start...\n");

    register_signal_handlers();

    robolibrary::Communicator robodriver;
    if( robodriver.init(DRIVER_2WD_SOCKET_NAME, robolibrary::Communicator::CLIENT) ) {
        fprintf(stderr, "[!] Error: cant create communication: %s!\n", DRIVER_2WD_SOCKET_NAME);
        return -1;
    }

    robolibrary::Communicator speecher;
    if( speecher.init(SPEECHER_SOCKET_NAME, robolibrary::Communicator::CLIENT) ) {
        fprintf(stderr, "[!] Error: cant create communication: %s!\n", SPEECHER_SOCKET_NAME);
        return -1;
    }

#if defined(LINUX)
    // Use termios to turn off line buffering
    termios term, cooked;
    tcgetattr(STDIN_FILENO, &term);
    memcpy(&cooked, &term, sizeof(term));
    term.c_lflag &= ~(ICANON | ECHO);
    term.c_cc[VEOL] = 1;
    term.c_cc[VEOF] = 2;
    tcsetattr(STDIN_FILENO, TCSANOW, &term);
    setbuf(stdin, NULL);
#endif

    int res = 0;

    CmdDrive2WD cmd_drive_2wd;
    CmdTelemetry2WD cmd_telemetry_2wd;

    CmdSpeech cmd_speech;
    CmdAcknowledgment cmd_ack;

    memset(&cmd_drive_2wd, 0, sizeof(cmd_drive_2wd));
    memset(&cmd_telemetry_2wd, 0, sizeof(cmd_telemetry_2wd));

    memset(&cmd_speech, 0, sizeof(cmd_speech));
    memset(&cmd_ack, 0, sizeof(cmd_ack));

    char cmd_buf[256]={0};
    int cmd_buf_size = 0;

    bool is_speech_end = true;

    while( 1 ) {

        int key = console::waitKey(30);
        if(key != 0 ) printf( "[i] Key: %c (%d)\n", key ,key );
        if(key == 27) { //ESC
            break;
        }

        if(robodriver.cli_sockfd != SOCKET_ERROR) {
            if(robodriver.available(50, robodriver.cli_sockfd)) {
                printf("[i] Client data available...\n");
                if( (cmd_buf_size = robodriver.read(cmd_buf, sizeof(cmd_buf)))>0 ) {
                    if(cmd_buf_size > CMD_SIG_SIZE) {
                        if( !strncmp(cmd_buf, "tlmtry", CMD_SIG_SIZE) && cmd_buf_size >= sizeof(cmd_telemetry_2wd) ) {
                            memcpy(&cmd_telemetry_2wd, cmd_buf, sizeof(cmd_telemetry_2wd));
                            printf("[i] 2WD telemetry: US: %d PWM: [%d %d]\n", cmd_telemetry_2wd.US,
                                   cmd_telemetry_2wd.pwm[0], cmd_telemetry_2wd.pwm[1]);


                            if(is_speech_end && cmd_telemetry_2wd.US < 30) {

                                if(speecher.cli_sockfd != SOCKET_ERROR) {
                                    is_speech_end = false;

                                    strncpy(cmd_speech.sig, "speech", CMD_SIG_SIZE);
                                    cmd_speech.code = 0;
                                    strncpy(cmd_speech.name, "./snd/dog_growl.wav", sizeof(cmd_speech.name));

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
                    robodriver.cli_close();
                }
            }

        }

        if(speecher.cli_sockfd != SOCKET_ERROR) {
            if(speecher.available(50, speecher.cli_sockfd)) {
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
                    speecher.cli_close();
                }
            }
        }

    }

#if defined(LINUX)
    tcsetattr(STDIN_FILENO, TCSANOW, &cooked);
#endif

    printf("[i] End.\n");

    return 0;
}
