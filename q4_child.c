/**
 *
 * @file q4_child.c
 * @author Emma Wong (101297761)
 * @author Eshal Kashif (101297950)
 *
 */

#include <stdio.h>      // printf
#include <unistd.h>     // getpid, usleep
#include <sys/types.h>  // pid_t, key_t
#include <sys/ipc.h>    // IPC_CREAT
#include <sys/shm.h>    // shmget, shmat, shmdt
#include <stdlib.h>     // exit

// Shared memory structure (must match Process 1)
typedef struct {
    int multiple;    // The multiple to check 
    int counter;     // Shared counter value
} SharedData;

int main(void) {
    key_t key;
    int shmid;
    SharedData *shared_mem;
    
    printf("Process 2 (child, PID %d) started\n", getpid());
    
    // Get the same key used by Process 1
    key = ftok(".", 'M');
    if (key == -1) {
        perror("Process 2: ftok failed");
        exit(EXIT_FAILURE);
    }
    
    // Access the existing shared memory segment
    shmid = shmget(key, sizeof(SharedData), 0666);
    if (shmid == -1) {
        perror("Process 2: shmget failed");
        exit(EXIT_FAILURE);
    }
    
    // Attach shared memory
    shared_mem = (SharedData *)shmat(shmid, NULL, 0);
    if (shared_mem == (SharedData *)-1) {
        perror("Process 2: shmat failed");
        exit(EXIT_FAILURE);
    }
    
    printf("Process 2: Shared memory attached\n");
    printf("Process 2: Waiting for counter to reach 100...\n");
    
    // Wait until counter exceeds 100
    while (shared_mem->counter <= 100) {
        sleep(0.1);  // Check every 0.1s
    }
    
    printf("\nProcess 2: Counter is now %d (> 100), starting operation!\n", shared_mem->counter);
    printf("Process 2: Monitoring multiples of %d\n\n", shared_mem->multiple);
    
    // Monitor the shared counter
    int last_counter = shared_mem->counter;
    
    while (1) {
        // Check if counter exceeds 500
        if (shared_mem->counter > 500) {
            printf("\nProcess 2: Counter (%d) exceeded 500!\n", shared_mem->counter);
            printf("Process 2: Exiting now...\n");
            break;
        }
        
        // Only display when counter changes
        if (shared_mem->counter != last_counter) {
            // Check if it's a multiple
            if (shared_mem->counter % shared_mem->multiple == 0) {
                printf("Process 2 - Cycle: %d - %d is a multiple of %d (shared memory)\n", 
                       shared_mem->counter, shared_mem->counter, shared_mem->multiple);
            } else {
                printf("Process 2 - Cycle: %d\n", shared_mem->counter);
            }
            
            last_counter = shared_mem->counter;
        }
        
        sleep(0.1);  // Check every 0.1s
    }
    
    // Detach shared memory
    shmdt(shared_mem);
    printf("Process 2: Shared memory detached\n");
    
    return 0;
}