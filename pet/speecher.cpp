//
// speecher
//
// http://robocraft.ru
//

//#define USE_KEYBOARD_INPUT 1

#include <stdio.h>
#include <stdlib.h>

#include "robopet.h"

#include "lib/times.h"
#include "lib/console.h"

#include "lib/roboipc.h"

#if defined(LINUX)
# include <unistd.h>
# include <sys/types.h>
# include <sys/wait.h>
# include <signal.h>
#endif

char* get_filename_for_speak(CmdSpeech &cmd) {
    switch(cmd.code) {
    case 0:
        return cmd.name;
        break;
    case 1:
        return "./snd/dog_bark2.wav";
        break;
    default:
        break;
    }

    return 0;
}

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

    roboipc::CommunicatorServer communicator;
    roboipc::CommunicatorClient client;

    if( communicator.init(SPEECHER_SOCKET_NAME) ) {
        fprintf(stderr, "[!] Error: cant create communication: %s!\n", SPEECHER_SOCKET_NAME);
        return -1;
    }

    communicator.is_auto_close = false;
    client.is_auto_close = false;

    int res = 0;

#if defined(USE_KEYBOARD_INPUT) && defined(LINUX)
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

    int return_value;
    int child_status = -1;
    pid_t child_pid;

    while( !terminated ) {

#if defined(USE_KEYBOARD_INPUT)
        int key = console::waitKey(30);
        if(key != 0 ) printf( "[i] Key: %c (%d)\n", key ,key );
        if(key == 27) { //ESC
            break;
        }
#endif //#if defined(USE_KEYBOARD_INPUT)

#if 1
        SOCKET sockfd = SOCKET_ERROR;
        if( (sockfd = communicator.connected(200)) != SOCKET_ERROR ) {
            printf("[i] Client connected...\n");
            client.close();
            client.sockfd = sockfd;
        }

        if(client.available(50) > 0) {
            printf("[i] Client action...\n");
            if( (cmd_buf_size = client.read(cmd_buf, sizeof(cmd_buf)))>0 ) {
                printf("[i] Data size: %d\n", cmd_buf_size);
                if(cmd_buf_size > CMD_SIG_SIZE) {
                    if( !strncmp(cmd_buf, "speech", CMD_SIG_SIZE) && cmd_buf_size >= sizeof(cmd_speech) ) {
                        memcpy(&cmd_speech, cmd_buf, sizeof(cmd_speech));
                        printf("[i] Clinet command speech: %d %s\n", cmd_speech.code, cmd_speech.name);

                        child_pid = fork();
                        if(child_pid == 0) {
                            // child

                            char filename[sizeof(cmd_speech.name)] = {0};
                            if(get_filename_for_speak(cmd_speech)) {
                                strncpy(filename, get_filename_for_speak(cmd_speech), sizeof(filename));
                            }
                            printf("[i] play file: %s\n", filename);

                            //return_value = execl("/usr/bin/aplay", "aplay", "./snd/dog_woof.wav");
                            return_value = execl("/usr/bin/aplay", "aplay", filename, "-D", "default:CARD=ALSA", 0); //return_value = execl("/usr/bin/aplay", "aplay", filename, 0);
                            return return_value;
                        }
                        else {
                            // parent
                            printf("[i] parent process, with id %d\n", (int) getpid());
                            printf("[i] childs process ID is %d\n", (int) child_pid);
                        }

#if 1
                        wait( &child_status );
                        printf("[i] status: %d\n", child_status);

                        if (WIFEXITED (child_status)) {
                            printf ("[i] the child process exited normally, with exit code %d\n", WEXITSTATUS (child_status));
                        }
                        else {
                            printf ("[i] the child process exited abnormally\n");
                        }

                        printf( "[i] Send ACK...\n");

                        if(client.sockfd != SOCKET_ERROR) {
                            strncpy(cmd_ack.sig, "ackmnt", CMD_SIG_SIZE);
                            cmd_ack.code = 0;

                            res = client.write(&cmd_ack, sizeof(cmd_ack));
                            printf( "[i] Send ACK (%d)...\n", res);
                        }
#endif
                    }
                }
            }
            else {
                printf("[i] Connection closed...\n");
                client.close();
            }
        }

#endif
    }  // while( !terminated ) {

    communicator.close();

    printf("[i] End.\n");

    return 0;
}
