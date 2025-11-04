#include <stdio.h>      // printf
#include <unistd.h>     // getpid, usleep
#include <sys/types.h>  // pid_t
#include <stdlib.h>     // abs

// PROCESS 2: Child process


int main(void) {
    printf("Process 2 (child, PID %d) started\n", getpid());
    long cycle = 0;
    
    while (1) {
        // Check if cycle is a multiple of 3
        if (cycle % 3 == 0) {
            printf("Process 2 - Cycle number: %ld - %ld is a multiple of 3\n", cycle, cycle);
        } else {
            printf("Process 2 - Cycle number: %ld\n", cycle);
        }

        // Check if we've reached the exit condition
        if (cycle <= -500) {
            printf("\nProcess 2: Reached %ld (lower than -500)\n", cycle);
            printf("Process 2: Exiting now...\n");
            exit(0);  // Exit with status 0 (success)
        }

        cycle--;
        sleep(0.5);   // sleep 0.5s to slow the output
    }

    return 0;
}