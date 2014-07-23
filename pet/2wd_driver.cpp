//
// driver for 2WD robot (use ORCP2 protocol)
//
// http://robocraft.ru
//

//#define USE_KEYBOARD_INPUT 1

#include <stdio.h>
#include <stdlib.h>

#include "robopet.h"

#include "orcp2/orcp2.h"
#include "orcp2/robot_2wd.h"

#include "lib/serial.h"
#include "lib/times.h"
#include "lib/console.h"

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

#if 1

#if defined(WIN32)
    char _port[]="COM21";
#elif defined(LINUX)
    char _port[]="/dev/ttyUSB0";
#endif
    int _rate = 57600;

    char* port = _port;
    int rate = _rate;

    if(argc >= 4) {
        port = argv[1];
        rate = atoi(argv[2]);
    }
    else if(argc > 1) {
        port = argv[1];
    }
    else if(argc <= 1) {
        printf("Usage: \n");
        printf("program <port name> <baud rate>\n\n");
    }

    printf("[i] port: %s\n", port);
    printf("[i] rate: %d\n", rate);

    Serial serial;
    if( serial.open(port, rate) ) {
        fprintf(stderr, "[!] Error: cant open port: %s:%d!\n", port, rate);
        return -1;
    }

    roboipc::CommunicatorServer communicator;
    roboipc::CommunicatorClient client;

    if( communicator.init(DRIVER_2WD_SOCKET_NAME) ) {
        fprintf(stderr, "[!] Error: cant create communication: %s!\n", DRIVER_2WD_SOCKET_NAME);
        return -1;
    }

    int res = 0;
    TBuff<uint8_t> buff;
    buff.resize(2048);

    orcp2::packet pkt;
    pkt.message.resize(256);

    if(!buff.data || !pkt.message.data) {
        fprintf(stderr, "[!] Error: cant allocate memory!\n");
        serial.close();
        return -1;
    }

    orcp2::ORCP2 orcp(serial);

    Robot_2WD robot_data;

    int val = 0;
    int counter = 0;
    int speed = 100;

#if defined(USE_KEYBOARD_INPUT) && defined(LINUX)
    // Use termios to turn off line buffering
    termios term;
    tcgetattr(STDIN_FILENO, &term);
    term.c_lflag &= ~ICANON;
    tcsetattr(STDIN_FILENO, TCSANOW, &term);
    setbuf(stdin, NULL);
#endif

    CmdDrive2WD cmd_drive_2wd;
    CmdTelemetry2WD cmd_telemetry_2wd;

    memset(&cmd_drive_2wd, 0, sizeof(cmd_drive_2wd));
    memset(&cmd_telemetry_2wd, 0, sizeof(cmd_telemetry_2wd));
    char cmd_buf[128]={0};
    int cmd_buf_size = 0;

    serial.discardInput();

    while( !terminated ) {

#if defined(USE_KEYBOARD_INPUT)
        int key = console::waitKey(30);
        if(key != 0 ) printf( "[i] Key: %c (%d)\n", key ,key );
        if(key == 27) { //ESC
            break;
        }
        else if(key == 32) { // SPACE
            printf("[i] stop\n");
            orcp.motorsWrite(0);

        }
        else if(key == 'w' || key == 'W') {
            printf("[i] forward %d\n", speed);
            orcp.motorsWrite(speed);
        }
        else if(key == 's' || key == 'S') {
            printf("[i] backward %d\n", speed);
            orcp.motorsWrite(-speed);
        }
        else if(key == 'a' || key == 'A') {
            printf("[i] left %d\n", speed);
            orcp.drive_2wd(-speed, speed);
        }
        else if(key == 'd' || key == 'D') {
            printf("[i] right %d\n", speed);
            orcp.drive_2wd(speed, -speed);
        }
#endif //#if defined(USE_KEYBOARD_INPUT)

        if( res = serial.waitInput(500) ) {
            //printf("[i] waitInput: %d\n", res);
            if( (res = serial.available()) > 0 ) {
                //printf("[i] available: %d\n", res);
                //res = res > (buff.real_size-buff.size) ? (buff.real_size-buff.size) : res ;
                if( (res = serial.read(buff.data+buff.size, buff.real_size-buff.size)) > 0 ) {
                    buff.size += res;

                    // get packet from data
                    while ( (res = orcp2::get_packet(buff.data, buff.size, &pkt)) > 0) {
                        printf("[i] res: %02d message: %04X size: %04d CRC: %02X buff.size: %02d\n",
                               res, pkt.message_type, pkt.message_size, pkt.checksum, buff.size);

                        uint8_t crc = pkt.calc_checksum();
                        if(crc == pkt.checksum) {
                            printf("[i] Good message CRC: %02X\n", pkt.checksum);

                            switch(pkt.message_type) {
                            case ORCP2_SEND_STRING:
                                pkt.message.data[pkt.message.size]=0;
                                printf("[i] String: %s\n", pkt.message.data);
                                //printf("[i] counter: %d\n", counter);
                                if(++counter > 50) {
                                    //printf("[i] digitalWrite: %d\n", val);
                                    val = !val;
                                    orcp.digitalWrite(13, val);
                                    counter = 0;
                                }
                                break;
#if 1
                            case ORCP2_MESSAGE_ROBOT_2WD_TELEMETRY:
                                deserialize_robot_2wd(pkt.message.data, pkt.message.size, &robot_data);
                                printf( "[i] Robot2WD Telemetry: bmp: %d PWM: [%d %d] US: %d IR: %d V: %d\n",
                                        robot_data.Bamper,
                                        robot_data.PWM[0], robot_data.PWM[1],
                                        robot_data.US[0],
                                        robot_data.IR[0],
                                        robot_data.Voltage );

                                // send telemetry
                                if(client.sockfd != SOCKET_ERROR) {
                                    strncpy(cmd_telemetry_2wd.sig, "tlmtry", CMD_SIG_SIZE);
                                    cmd_telemetry_2wd.Bamper = robot_data.Bamper;
                                    cmd_telemetry_2wd.pwm[0] = robot_data.PWM[0];
                                    cmd_telemetry_2wd.pwm[1] = robot_data.PWM[1];
                                    cmd_telemetry_2wd.US = robot_data.US[0];
                                    cmd_telemetry_2wd.IR = robot_data.IR[0];
                                    cmd_telemetry_2wd.Voltage = robot_data.Voltage;

                                    res = client.write(&cmd_telemetry_2wd, sizeof(cmd_telemetry_2wd));
                                    printf( "[i] Send robot2WD telemetry (%d)...\n", res);
                                }

                                break;
#endif
                            default:
                                printf("[i] Unknown message type: %04X!\n", pkt.message_type);
                                break;
                            }

                        }
                        else {
                            printf("[!] Bad message CRC: %02X vs: %02X\n", crc, pkt.checksum);
                        }
                    }
                }
                else {
                    printf("[!] too much data: %d (%d)\n", res, buff.size);
                    buff.size = 0;
                }
            }
            else {
                printf("[!] no available: %d\n", res);
            }
        }
        else {
            printf("[!] waitInput timeout: %d\n", res);
        }

#if 1
        SOCKET sockfd = SOCKET_ERROR;
        if( (sockfd = communicator.connected(10)) != SOCKET_ERROR ) {
            printf("[i] Client connected...\n");
            client.close();
            client.sockfd = sockfd;
        }
        if(client.available(50) > 0) {
            printf("[i] Client data available...\n");
            if( (cmd_buf_size = client.read(cmd_buf, sizeof(cmd_buf)))>0 ) {
                if(cmd_buf_size > CMD_SIG_SIZE) {
                    if( !strncmp(cmd_buf, "drive2", CMD_SIG_SIZE) && cmd_buf_size >= sizeof(cmd_drive_2wd) ) {
                        memcpy(&cmd_drive_2wd, cmd_buf, sizeof(cmd_drive_2wd));
                        printf("[i] Clinet command 2WD drive: %d %d\n", cmd_drive_2wd.pwm[0], cmd_drive_2wd.pwm[1]);
                        orcp.drive_2wd(cmd_drive_2wd.pwm[0], cmd_drive_2wd.pwm[1]);
                    }
                }
            }
            else {
                printf("[i] Connection closed...\n");
                client.close();
            }
        }
#endif
    } // while( !terminated ) {

    printf("[i] stop\n");
    orcp.motorsWrite(0);

    serial.close();

#endif

    printf("[i] End.\n");

    return 0;
}
