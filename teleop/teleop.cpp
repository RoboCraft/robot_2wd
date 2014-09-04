//
// teleop
//
// http://robocraft.ru
//

//#define USE_KEYBOARD_INPUT 1

#include <stdio.h>
#include <stdlib.h>

#include "teleop.h"

#include "orcp2/orcp2.h"
#include "lib/times.h"
#include "lib/console.h"
#include "orcp2/robot_2wd.h"
#include "lib/roboipc.h"

#if defined(LINUX)
# include <unistd.h>
# include <sys/types.h>
# include <sys/wait.h>
# include <signal.h>
# include <sys/stat.h> // chmod()
#endif

#define BUF_SIZE 1024

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
#else
volatile int terminated = 0;
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

class Teleoperation
{
public:
    enum { ST_NA = -1,
           ST_MOVE_FORWARD, ST_MOVE_LEFT, ST_MOVE_RIGHT, ST_MOVE_BACKWARD, ST_STOP
         };

    Teleoperation() {
        state = ST_NA;
    }
    ~Teleoperation() {
        stop();
    }

    int init()
    {
        printf("[i][init]\n");

        if( robodriver.connect(DRIVER_2WD_SOCKET_NAME) ) {
            fprintf(stderr, "[!] Error: cant create communication: %s!\n", DRIVER_2WD_SOCKET_NAME);
            return -1;
        }

        if( communicator.init(TELEOPERATION_SOCKET_NAME) ) {
            fprintf(stderr, "[!] Error: cant create communication: %s!\n", TELEOPERATION_SOCKET_NAME);
            return -1;
        }

#if defined(LINUX)
        // for ajax connection (chmod 777)
        if( chmod(TELEOPERATION_SOCKET_NAME, S_IRWXU|S_IRWXG|S_IRWXO) ) {
            fprintf(stderr, "[!] Error: cant set permissions for: %s!\n", TELEOPERATION_SOCKET_NAME);
            return -1;
        }
#endif

        memset(&cmd_drive_2wd, 0, sizeof(cmd_drive_2wd));
        memset(&cmd_telemetry_2wd, 0, sizeof(cmd_telemetry_2wd));
        memset(&cmd_digital_write, 0, sizeof(cmd_digital_write));
        memset(&cmd_ack, 0, sizeof(cmd_ack));

        memset(cmd_buf, 0, sizeof(cmd_buf));
        cmd_buf_size = 0;

        memset(answer_buf, 0, sizeof(answer_buf));
        answer_buf_size = 0;

        blink_time = 0;
        blink_counter = 0;

        speed = 70;

        return 0;
    }

    int make() {
        printf("[i][make]\n");

        int res = 0;

        if(robodriver.available(50) > 0) {
            printf("[i] Robot data available...\n");
            if( (cmd_buf_size = robodriver.read(cmd_buf, sizeof(cmd_buf))) > 0 ) {
                if(cmd_buf_size > CMD_SIG_SIZE) {
                    if( !strncmp(cmd_buf, "tlmtry", CMD_SIG_SIZE) && cmd_buf_size >= sizeof(cmd_telemetry_2wd) ) {
                        memcpy(&cmd_telemetry_2wd, cmd_buf, sizeof(cmd_telemetry_2wd));
                        printf("[i] 2WD telemetry: Bmp: %d US: %d IR: [%d %d] PWM: [%d %d]\n",
                               cmd_telemetry_2wd.Bumper,
                               cmd_telemetry_2wd.US,
                               cmd_telemetry_2wd.IR[0], cmd_telemetry_2wd.IR[1],
                               cmd_telemetry_2wd.pwm[0], cmd_telemetry_2wd.pwm[1]);

                        if( cmd_telemetry_2wd.Bumper &&
                            state != ST_STOP &&
                            state != ST_MOVE_BACKWARD) {
                                state = ST_STOP;
                                stop();
                        }
                    }
                }
            }
            else {
                printf("[i] Connection closed...\n");
                robodriver.close();
            }
        }

        SOCKET sockfd = SOCKET_ERROR;
        if( (sockfd = communicator.connected(100)) != SOCKET_ERROR ) {
            printf("[i] Client connected...\n");
            client.close();
            client.sockfd = sockfd;
        }

        if(client.available(100) > 0) {
            printf("[i] Client action...\n");
            if( (cmd_buf_size = client.read(cmd_buf, sizeof(cmd_buf)))>0 ) {
                cmd_buf[cmd_buf_size] = 0;
                printf("[i] Data (%d): %s\n", cmd_buf_size, cmd_buf);

                int buf_shift = 4; // skip "cmd="
                if(cmd_buf_size > buf_shift) {
                    if(!strncmp(cmd_buf+buf_shift, "forward", cmd_buf_size-buf_shift)) {
                        if(!cmd_telemetry_2wd.Bumper) {
                            state = ST_MOVE_FORWARD;
                        }
                        else {
                            state = ST_STOP;
                        }
                    }
                    else if(!strncmp(cmd_buf+buf_shift, "back", cmd_buf_size-buf_shift)) {
                        state = ST_MOVE_BACKWARD;
                    }
                    else if(!strncmp(cmd_buf+buf_shift, "left", cmd_buf_size-buf_shift)) {
                        if(!cmd_telemetry_2wd.Bumper) {
                            state = ST_MOVE_LEFT;
                        }
                        else {
                            state = ST_STOP;
                        }
                    }
                    else if(!strncmp(cmd_buf+buf_shift, "right", cmd_buf_size-buf_shift)) {
                        if(!cmd_telemetry_2wd.Bumper) {
                            state = ST_MOVE_RIGHT;
                        }
                        else {
                            state = ST_STOP;
                        }
                    }
                    else if(!strncmp(cmd_buf+buf_shift, "stop", cmd_buf_size-buf_shift)) {
                        state = ST_STOP;
                    }
                    else {
                        //state = ST_STOP;
                    }
                }
                else {
                    state = ST_STOP;
                }
            }
            else {
                printf("[i] Connection closed...\n");
                client.close();
                state = ST_STOP;
            }

            printf("[i] Send telemetry...\n");
            if(client.sockfd != SOCKET_ERROR) {
                answer_buf_size = snprintf(answer_buf, sizeof(answer_buf)-1,
                        "{ \"bamper\" : %d,  \"us\" : %d, \"ir0\" : %d, \"ir1\" : %d, \"pwm0\" : %d, \"pwm1\" : %d,  \"voltage\" : %d}",
                        cmd_telemetry_2wd.Bumper,
                        cmd_telemetry_2wd.US,
                        cmd_telemetry_2wd.IR[0], cmd_telemetry_2wd.IR[1],
                        cmd_telemetry_2wd.pwm[0], cmd_telemetry_2wd.pwm[1],
                        cmd_telemetry_2wd.Voltage);

                if(answer_buf_size > 0) {
                    res = client.write(&answer_buf, answer_buf_size);
                    printf("[i] Send telemetry (%d)...\n", res);
                }
                else {
                    fprintf(stderr, "[!] Error: make message!\n");
                }
            }
        }

        switch (state) {
        case ST_MOVE_FORWARD:
            forward(speed);
            break;
        case ST_MOVE_BACKWARD:
            backward(speed);
            break;
        case ST_MOVE_LEFT:
            left(speed);
            break;
        case ST_MOVE_RIGHT:
            right(speed);
            break;
        case ST_STOP:
        default:
            stop();
            break;
        }

        return 0;
    }

    int drive(int16_t pwm1, int16_t pwm2)
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

    int stop()
    {
        return drive(0, 0);
    }

    int forward(int16_t speed)
    {
        return drive(speed, speed);
    }

    int backward(int16_t speed)
    {
        return drive(-speed, -speed);
    }

    int left(int16_t speed)
    {
        return drive(-speed, speed);
    }

    int right(int16_t speed)
    {
        return drive(speed, -speed);
    }

    int state;
    DWORD blink_time;
    unsigned int blink_counter;
    int speed;

protected:
    roboipc::CommunicatorClient robodriver;

    roboipc::CommunicatorServer communicator;
    roboipc::CommunicatorClient client;

    CmdDrive2WD cmd_drive_2wd;
    CmdTelemetry2WD cmd_telemetry_2wd;
    CmdDigitalWrite cmd_digital_write;
    CmdAcknowledgment cmd_ack;

    char cmd_buf[BUF_SIZE];
    int cmd_buf_size;

    char answer_buf[BUF_SIZE];
    int answer_buf_size;
};

int main(int argc, char* argv[])
{
    printf("[i] Start...\n");

    register_signal_handlers();

    // init
    Teleoperation teleop;

    if( teleop.init() ) {
        fprintf(stderr, "[!] Error: teleop init!\n");
        return -1;
    }

#if defined(USE_KEYBOARD_INPUT) && defined(LINUX)
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

    while( !terminated ) {

#if defined(USE_KEYBOARD_INPUT)
        int key = console::waitKey(30);
        if(key != 0 ) printf( "[i] Key: %c (%d)\n", key ,key );
        if(key == 27) { //ESC
            break;
        }
#endif //#if defined(USE_KEYBOARD_INPUT)

        // process
        teleop.make();

    } //while( !terminated ) {

    printf("[i] End.\n");

    return 0;
}
