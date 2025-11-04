#include <stdio.h>      // printf
#include <unistd.h>     // fork, getpid, usleep
#include <sys/types.h>  // pid_t
#include <stdlib.h>     // exit, EXIT_FAILURE

int main(void) {
    pid_t pid = fork();   // create a new process

    if (pid < 0) {
        // fork failed
        perror("fork failed");
        exit(EXIT_FAILURE);
    }

    if (pid == 0) {
        // Child process: process 2 
        long counter = 0;
        while (1) {
            printf("Process 2 (child, PID %d): counter = %ld\n", getpid(), counter);
            counter++;
            usleep(500000);   // sleep 500ms = 0.5s to slow the output
        }
    } else {
        //  Parent process: process 1
        long counter = 0;
        while (1) {
            printf("Process 1 (parent, PID %d): counter = %ld\n", getpid(), counter);
            counter++;
            usleep(500000);   // sleep 500ms = 0.5s
        }
    }

    return 0;
}
