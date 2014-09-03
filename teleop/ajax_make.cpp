//
// process ajax request for teleoperation
//
// http://robocraft.ru
//

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

int main(int argc, char* argv[])
{
    register_signal_handlers();

    int res = 0;
    char message[BUF_SIZE] = {0};
    int message_size = 0;

    int error_code = 0;
    char error_message[BUF_SIZE] = {0};
    int error_message_size = 0;

    roboipc::CommunicatorClient teleoperation;
    if( teleoperation.connect(TELEOPERATION_SOCKET_NAME) ) {
        fprintf(stderr, "[!] Error: cant create communication: %s!\n", TELEOPERATION_SOCKET_NAME);
        //return -1;
    }

    char* request_method = getenv("REQUEST_METHOD");
    char* content_type = getenv("CONTENT_TYPE");
    char* content_length = getenv("CONTENT_LENGTH");
    char* query_string = getenv("QUERY_STRING");

#if 1
    //printf("Content-Type:text/html\n\n");

    //printf("[i] Parse data...\n");
    if(request_method) {
        printf("REQUEST_METHOD: %s\n", request_method);
    }
    if(content_type) {
        printf("CONTENT_TYPE: %s\n", content_type);
    }
    if(content_length) {
        printf("CONTENT_LENGTH: %s\n", content_length);
    }
    if(query_string) {
        printf("%s\n", query_string);
    }
    else {
        fprintf(stderr, "[!] Error: cant get QUERY_STRING!\n");
    }
#endif

    if(!query_string) {
        error_code = 1;
        error_message_size = snprintf(error_message, BUF_SIZE, "[!] error: cant get QUERY_STRING!");
        goto exit;
    }

    if(teleoperation.sockfd != SOCKET_ERROR) {
        res = teleoperation.write(query_string, strlen(query_string));
    }

    if(teleoperation.available(50)) {
        message_size = teleoperation.read(message, sizeof(message));
    }

exit:
    printf("Content-type: application/json\n\n");
    if(error_code) {
        printf("{\"error\":%d, \"error_message\": \"%s\"}\n", error_code, error_message);
    }
    else {
        printf("%s\n", message);
    }

    return 0;
}
