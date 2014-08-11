//
// robot pet
//
// http://robocraft.ru
//

//#define USE_KEYBOARD_INPUT 1

#include <stdio.h>
#include <stdlib.h>

#include "pet.h"

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

int main(int argc, char* argv[])
{
    printf("[i] Start...\n");

    register_signal_handlers();

    Pet pet;
    if( pet.init() ) {
        fprintf(stderr, "[!] Error: pets init!\n");
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

       pet.make();

    } //while( !terminated ) {

    printf("[i] End.\n");

    return 0;
}
