//
// test speecher
//
// http://robocraft.ru
//

#include <stdio.h>
#include <stdlib.h>

#include "robopet.h"

#include "lib/times.h"
#include "lib/console.h"

#include "lib/roboipc.h"

int main(int argc, char* argv[])
{
    printf("[i] Start...\n");

    robolibrary::Communicator communicator;
    if( communicator.init(SPEECHER_SOCKET_NAME, robolibrary::Communicator::CLIENT) ) {
        fprintf(stderr, "[!] Error: cant create communication: %s!\n", SPEECHER_SOCKET_NAME);
        return -1;
    }

    int res = 0;

#if defined(LINUX)
    // Use termios to turn off line buffering
    termios term;
    tcgetattr(STDIN_FILENO, &term);
    term.c_lflag &= ~ICANON;
    tcsetattr(STDIN_FILENO, TCSANOW, &term);
    setbuf(stdin, NULL);
#endif

    CmdSpeech cmd_speech;
    CmdAcknowledgment cmd_ack;

    memset(&cmd_speech, 0, sizeof(cmd_speech));
    memset(&cmd_ack, 0, sizeof(cmd_ack));

    char cmd_buf[128]={0};
    int cmd_buf_size = 0;

    while( 1 ) {

        int key = console::waitKey(30);
        if(key != 0 ) printf( "[i] Key: %c (%d)\n", key ,key );

        if(key == 27) { //ESC
            break;
        }
        else if(key == 32) { // SPACE
            cmd_speech.code = 0;
            strncpy(cmd_speech.name, "./snd/dog_woof.wav", sizeof(cmd_speech.name));

            if(communicator.cli_sockfd != SOCKET_ERROR) {
                strncpy(cmd_speech.sig, "speech", CMD_SIG_SIZE);

                res = communicator.write(&cmd_speech, sizeof(cmd_speech));
                printf( "[i] Send speech command (%d)...\n", res);
                printf( "[i] Data: %d %s\n", cmd_speech.code, cmd_speech.name);
            }
        }

        if(communicator.cli_sockfd != SOCKET_ERROR) {
            if(communicator.available(50, communicator.cli_sockfd)) {
                printf("[i] Client action...\n");
                if( (cmd_buf_size = communicator.read(cmd_buf, sizeof(cmd_buf)))>0 ) {
                    printf("[i] Data size: %d\n", cmd_buf_size);
                    if(cmd_buf_size > CMD_SIG_SIZE) {
                        if( !strncmp(cmd_buf, "ackmnt", CMD_SIG_SIZE) && cmd_buf_size >= sizeof(cmd_ack) ) {
                            memcpy(&cmd_ack, cmd_buf, sizeof(cmd_ack));
                            printf("[i] Clinet ACK: %d\n", cmd_ack.code);
                        }
                    }
                }
                else {
                    printf("[i] Connection closed...\n");
                    communicator.cli_close();
                }
            }
        }
    } // while( 1 ) {

    printf("[i] End.\n");

    return 0;
}
