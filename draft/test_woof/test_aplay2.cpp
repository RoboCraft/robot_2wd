//
// test
//

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#include <sys/types.h>
#include <sys/wait.h>

int main(int argc, char* argv[])
{
    printf("[i] Start...\n");

    int return_value;

    printf("[i] main program process ID is %d\n", (int)getpid());

    int counter=0;
    while(counter++<10) {
        printf("[i] counter %d\n", counter);

        int child_status = -1;
        pid_t child_pid;

        child_pid = fork();
        if(child_pid == 0) {
            //printf("[i] child process, with id %d\n", (int)getpid());

            return_value = execl("/usr/bin/aplay", "aplay", "dog_woof.wav");
            //printf("[i] return: %d\n", return_value);

            return 0;
        }
        else {
            printf("[i] parent process, with id %d\n", (int) getpid());
            printf("[i] childs process ID is %d\n", (int) child_pid);
        }

        wait( &child_status );
        printf("[i] status: %d\n", child_status);

        if (WIFEXITED (child_status)) {
            printf ("the child process exited normally, with exit code %d\n", WEXITSTATUS (child_status));
        }
        else {
            printf ("the child process exited abnormally\n");
        }
    }

    printf("[i] End.\n");

    return 0;
}
