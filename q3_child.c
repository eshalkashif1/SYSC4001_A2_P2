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
        cycle--;
        sleep(0.5);   // sleep 0.5s to slow the output
    }

    return 0;
}