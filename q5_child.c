/**
 *
 * @file q5_child.c
 * @author Emma Wong (101297761)
 * @author Eshal Kashif (101297950)
 *
 */

#include <stdio.h>      // printf
#include <unistd.h>     // getpid, usleep
#include <sys/types.h>  // pid_t, key_t
#include <sys/ipc.h>    // IPC_CREAT
#include <sys/shm.h>    // shmget, shmat, shmdt
#include <sys/sem.h>    // semget, semop, semctl
#include <stdlib.h>     // exit

// Shared memory structure 
typedef struct {
    int multiple;    // The multiple to check
    int counter;     // Shared counter value
} SharedData;

// Semaphore operations
void sem_wait(int semid) {
    struct sembuf sb = {0, -1, 0};  // Wait (P operation)
    if (semop(semid, &sb, 1) == -1) {
        perror("Process 2: semop wait failed");
        exit(EXIT_FAILURE);
    }
}

void sem_signal(int semid) {
    struct sembuf sb = {0, 1, 0};   // Signal (V operation)
    if (semop(semid, &sb, 1) == -1) {
        perror("Process 2: semop signal failed");
        exit(EXIT_FAILURE);
    }
}

int main(void) {
    key_t key_shm, key_sem;
    int shmid, semid;
    SharedData *shared_mem;
    
    printf("Process 2 (child, PID %d) started\n", getpid());
    
    // Get the same keys used by Process 1
    key_shm = ftok(".", 'M');
    key_sem = ftok(".", 'S');
    if (key_shm == -1 || key_sem == -1) {
        perror("Process 2: ftok failed");
        exit(EXIT_FAILURE);
    }
    
    // Access the existing shared memory segment
    shmid = shmget(key_shm, sizeof(SharedData), 0666);
    if (shmid == -1) {
        perror("Process 2: shmget failed");
        exit(EXIT_FAILURE);
    }
    
    // Access the existing semaphore
    semid = semget(key_sem, 1, 0666);
    if (semid == -1) {
        perror("Process 2: semget failed");
        exit(EXIT_FAILURE);
    }
    
    // Attach shared memory
    shared_mem = (SharedData *)shmat(shmid, NULL, 0);
    if (shared_mem == (SharedData *)-1) {
        perror("Process 2: shmat failed");
        exit(EXIT_FAILURE);
    }
    
    printf("Process 2: Shared memory attached\n");
    printf("Process 2: Semaphore accessed\n");
    printf("Process 2: Waiting for counter to reach 100...\n");
    
    // Wait until counter exceeds 100 (protected access)
    int current_counter;
    while (1) {
        sem_wait(semid);
        current_counter = shared_mem->counter;
        sem_signal(semid);
        
        if (current_counter > 100) {
            break;
        }
        sleep(0.1);  // Check every 0.1s
    }
    
    sem_wait(semid);
    int multiple = shared_mem->multiple;
    sem_signal(semid);
    
    printf("\nProcess 2: Counter is now %d (> 100), starting operation!\n", current_counter);
    printf("Process 2: Monitoring multiples of %d\n\n", multiple);
    
    // Monitor the shared counter
    int last_counter = current_counter;
    
    while (1) {
        // Read shared memory (protected)
        sem_wait(semid);
        current_counter = shared_mem->counter;
        int current_multiple = shared_mem->multiple;
        sem_signal(semid);
        
        // Check if counter exceeds 500
        if (current_counter > 500) {
            printf("\nProcess 2: Counter (%d) exceeded 500!\n", current_counter);
            printf("Process 2: Exiting now...\n");
            break;
        }
        
        // Only display when counter changes
        if (current_counter != last_counter) {
            // Check if it's a multiple
            if (current_counter % current_multiple == 0) {
                printf("Process 2 - Cycle: %d – %d is a multiple of %d (protected shared memory)\n", 
                       current_counter, current_counter, current_multiple);
            } else {
                printf("Process 2 - Cycle: %d\n", current_counter);
            }
            
            last_counter = current_counter;
        }
        
        sleep(0.1);  // Check every 0.1s
    }
    
    // Detach shared memory
    shmdt(shared_mem);
    printf("Process 2: Shared memory detached\n");
    
    return 0;
}