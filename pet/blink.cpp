//
// blink via ORCP2 protocol
//

#include <stdio.h>
#include <stdlib.h>

#include "orcp2/orcp2.h"
#include "lib/serial.h"
#include "lib/times.h"

#include "orcp2/robot_2wd.h"
#include "lib/console.h"

int main(int argc, char* argv[])
{
    printf("[i] Start...\n");

#if 1

#if defined(WIN32)
    char _port[]="COM21";
#elif defined(LINUX)
    char _port[]="/dev/ttyUSB0";
#endif
    int _rate = 57600;
    int _speed = 100;

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
        printf("program <port name> <baud rate> <speed>\n\n");
    }

    printf("[i] port: %s\n", port);
    printf("[i] rate: %d\n", rate);

    Serial serial;
    if( serial.open(port, rate) ) {
        //return -1;
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

    int val=0;
    int counter=0;

#if defined(LINUX)
    // Use termios to turn off line buffering
    termios term;
    tcgetattr(STDIN_FILENO, &term);
    term.c_lflag &= ~ICANON;
    tcsetattr(STDIN_FILENO, TCSANOW, &term);
    setbuf(stdin, NULL);
#endif

    unsigned int i = 0;

    DWORD blink_time = 0;
    unsigned int blink_counter = 0;

    while( 1 ) {

        int key = console::waitKey(30);
        if(key != 0 ) printf( "[i] Key: %c (%d)\n", key ,key );
        if(key == 27) { //ESC
            break;
        }

        i++;

        if(orv::time::is_time(blink_time, 1000)) {
            blink_counter++;
            int val = blink_counter%2;
            printf("%04d %d\n", i, val);
            orcp.digitalWrite(13, val);
        }
        //orv::time::sleep(500);
#if 0
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
                                printf( "[i] Robot2WD Telemetry: bmp: %d PWM: [%d %d] US: %d IR: [%d %d] V: %d\n",
                                        robot_data.Bamper,
                                        robot_data.PWM[0], robot_data.PWM[1],
                                        robot_data.US[0],
                                        robot_data.IR[0],  robot_data.IR[1],
                                        robot_data.Voltage );
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

#endif
    }

    serial.close();

#endif

    printf("[i] End.\n");

    return 0;
}
