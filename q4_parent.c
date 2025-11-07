/**
 *
 * @file q4_parent.c
 * @author Emma Wong (101297761)
 * @author Eshal Kashif (101297950)
 *
 */

#include <stdio.h>      // printf
#include <unistd.h>     // fork, getpid, usleep, execl
#include <sys/types.h>  // pid_t, key_t
#include <sys/wait.h>   // wait
#include <sys/ipc.h>    // IPC_CREAT, IPC_RMID
#include <sys/shm.h>    // shmget, shmat, shmdt, shmctl
#include <stdlib.h>     // exit, EXIT_FAILURE

// Shared memory structure
typedef struct {
    int multiple;    // The multiple to check 
    int counter;     // Shared counter value
} SharedData;

int main(void) {
    key_t key;
    int shmid;
    SharedData *shared_mem;
    
    // Create a unique key for shared memory
    key = ftok(".", 'M');
    if (key == -1) {
        perror("ftok failed");
        exit(EXIT_FAILURE);
    }
    
    // Create shared memory segment
    shmid = shmget(key, sizeof(SharedData), IPC_CREAT | 0666);
    if (shmid == -1) {
        perror("shmget failed");
        exit(EXIT_FAILURE);
    }
    
    // Attach shared memory
    shared_mem = (SharedData *)shmat(shmid, NULL, 0);
    if (shared_mem == (SharedData *)-1) {
        perror("shmat failed");
        exit(EXIT_FAILURE);
    }
    
    // Initialize shared memory
    shared_mem->multiple = 3;   // Change this value to test different multiples
    shared_mem->counter = 0;
    
    printf("Process 1 (parent, PID %d) started\n", getpid());
    printf("Process 1: Shared memory initialized\n");
    printf("Process 1: Multiple set to %d\n", shared_mem->multiple);
    printf("Process 1: Counter starts at %d\n", shared_mem->counter);
    
    pid_t pid = fork();   // create a new process

    if (pid < 0) {
        // fork failed
        perror("fork failed");
        shmdt(shared_mem);
        shmctl(shmid, IPC_RMID, NULL);
        exit(EXIT_FAILURE);
    }

    if (pid == 0) {
        // Child process: detach and execute process2
        shmdt(shared_mem);  // Detach before exec
        
        printf("Process 1: Launching Process 2...\n");
        execl("./q4_child", "q4_child", NULL);
        
        // If execl returns, it failed
        perror("execl failed");
        exit(EXIT_FAILURE);
    } else {
        // Parent process: process 1 (incrementing)
        int status;
        pid_t result;
        
        while (1) {
            // Check if Process 2 has finished 
            result = waitpid(pid, &status, WNOHANG);
            
            if (result > 0) {
                // Process 2 has finished
                printf("\nProcess 1: Process 2 (PID %d) has finished!\n", pid);
                printf("Process 1: Final counter value: %d\n", shared_mem->counter);
                printf("Process 1: Ending now...\n");
                break;
            } else if (result < 0) {
                perror("waitpid failed");
                break;
            }
            
            // Check if counter exceeds 500
            if (shared_mem->counter > 500) {
                printf("\nProcess 1: Counter (%d) exceeded 500!\n", shared_mem->counter);
                printf("Process 1: Ending now...\n");
                break;
            }
            
            // Display cycle information
            if (shared_mem->counter % shared_mem->multiple == 0) {
                printf("Process 1 - Cycle: %d - %d is a multiple of %d (shared memory)\n", 
                       shared_mem->counter, shared_mem->counter, shared_mem->multiple);
            } else {
                printf("Process 1 - Cycle: %d\n", shared_mem->counter);
            }
            
            // Increment the shared counter
            shared_mem->counter++;
            
            sleep(0.5);   // sleep 0.5s to slow the output
        }
        
        // Detach and remove shared memory
        shmdt(shared_mem);
        shmctl(shmid, IPC_RMID, NULL);
        printf("Process 1: Shared memory cleaned up\n");
    }

    return 0;
}