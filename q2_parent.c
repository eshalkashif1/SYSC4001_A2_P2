/**
 *
 * @file q2_parent.c
 * @author Emma Wong (101297761)
 * @author Eshal Kashif (101297950)
 *
 */

#include <stdio.h>      // printf
#include <unistd.h>     // fork, getpid, usleep, execl
#include <sys/types.h>  // pid_t
#include <stdlib.h>     // exit, EXIT_FAILURE

// PROCESS 1: Parent process

int main(void) {
    pid_t pid = fork();   // create a new process

    if (pid < 0) {
        // fork failed
        perror("fork failed");
        exit(EXIT_FAILURE);
    }

    if (pid == 0) {
        // Child process: execute process2
        printf("Process 1 launching Process 2...\n");
        execl("./q2_child", "q2_child", NULL);
        
        // If execl returns, it failed
        perror("execl failed");
        exit(EXIT_FAILURE);
    } else {
        // Parent process: process 1 (incrementing, multiples of 3)
        printf("Process 1 (parent, PID %d) started\n", getpid());
        long cycle = 0;
        
        while (1) {
            if (cycle % 3 == 0) {
                printf("Process 1 - Cycle number: %ld - %ld is a multiple of 3\n", cycle, cycle);
            } else {
                printf("Process 1 - Cycle number: %ld\n", cycle);
            }
            cycle++;
            sleep(0.5);   // sleep 0.5s to slow the output
        }
    }

    return 0;
}