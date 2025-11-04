#include <stdio.h>      // printf
#include <unistd.h>     // fork, getpid, usleep, execl
#include <sys/types.h>  // pid_t
#include <stdlib.h>     // exit, EXIT_FAILURE
#include <sys/wait.h>   // wait

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
        execl("./q3_child", "q3_child", NULL);
        
        // If execl returns, it failed
        perror("execl failed");
        exit(EXIT_FAILURE);
    } else {
        // Parent process: process 1 (incrementing, multiples of 3)
        printf("Process 1 (parent, PID %d) started\n", getpid());
        printf("Process 1 waiting for Process 2 (PID %d) to start...\n", pid);

        // Give Process 2 a moment to start
        sleep(0.1);  // 0.1s delay

        long cycle = 0;
        int status;
        pid_t result;
        
        while (1) {
            // Check if Process 2 has finished 
            result = waitpid(pid, &status, WNOHANG);
            
            if (result > 0) {
                // Process 2 has finished
                printf("\nProcess 1: Process 2 (PID %d) has finished!\n", pid);
                if (WIFEXITED(status)) {
                    printf("Process 1: Process 2 exited with status %d\n", WEXITSTATUS(status));
                }
                printf("Process 1: Ending now...\n");
                break;
            } else if (result < 0) {
                perror("waitpid failed");
                break;
            }
            
            // Continue counting
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