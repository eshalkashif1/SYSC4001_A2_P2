
#include <stdio.h>      // printf
#include <unistd.h>     // fork, getpid, usleep, execl
#include <sys/types.h>  // pid_t, key_t
#include <sys/wait.h>   // wait
#include <sys/ipc.h>    // IPC_CREAT, IPC_RMID
#include <sys/shm.h>    // shmget, shmat, shmdt, shmctl
#include <sys/sem.h>    // semget, semop, semctl
#include <stdlib.h>     // exit, EXIT_FAILURE

// Shared memory structure
typedef struct {
    int multiple;    // The multiple to check 
    int counter;     // Shared counter value
} SharedData;

// Union for semctl (required on some systems)
union semun {
    int val;
    struct semid_ds *buf;
    unsigned short *array;
};

// Semaphore operations
void sem_wait(int semid) {
    struct sembuf sb = {0, -1, 0};  // Wait (P operation)
    if (semop(semid, &sb, 1) == -1) {
        perror("semop wait failed");
        exit(EXIT_FAILURE);
    }
}

void sem_signal(int semid) {
    struct sembuf sb = {0, 1, 0};   // Signal (V operation)
    if (semop(semid, &sb, 1) == -1) {
        perror("semop signal failed");
        exit(EXIT_FAILURE);
    }
}

int main(void) {
    key_t key_shm, key_sem;
    int shmid, semid;
    SharedData *shared_mem;
    union semun sem_union;
    
    // Create unique keys
    key_shm = ftok(".", 'M');
    key_sem = ftok(".", 'S');
    if (key_shm == -1 || key_sem == -1) {
        perror("ftok failed");
        exit(EXIT_FAILURE);
    }
    
    // Create shared memory segment
    shmid = shmget(key_shm, sizeof(SharedData), IPC_CREAT | 0666);
    if (shmid == -1) {
        perror("shmget failed");
        exit(EXIT_FAILURE);
    }
    
    // Create semaphore
    semid = semget(key_sem, 1, IPC_CREAT | 0666);
    if (semid == -1) {
        perror("semget failed");
        shmctl(shmid, IPC_RMID, NULL);
        exit(EXIT_FAILURE);
    }
    
    // Initialize semaphore to 1 (available)
    sem_union.val = 1;
    if (semctl(semid, 0, SETVAL, sem_union) == -1) {
        perror("semctl SETVAL failed");
        shmctl(shmid, IPC_RMID, NULL);
        semctl(semid, 0, IPC_RMID);
        exit(EXIT_FAILURE);
    }
    
    // Attach shared memory
    shared_mem = (SharedData *)shmat(shmid, NULL, 0);
    if (shared_mem == (SharedData *)-1) {
        perror("shmat failed");
        semctl(semid, 0, IPC_RMID);
        shmctl(shmid, IPC_RMID, NULL);
        exit(EXIT_FAILURE);
    }
    
    // Initialize shared memory (protected by semaphore)
    sem_wait(semid);
    shared_mem->multiple = 3;   // Change this value to test different multiples
    shared_mem->counter = 0;
    sem_signal(semid);
    
    printf("Process 1 (parent, PID %d) started\n", getpid());
    printf("Process 1: Shared memory initialized\n");
    printf("Process 1: Semaphore created and initialized\n");
    printf("Process 1: Multiple set to %d\n", shared_mem->multiple);
    printf("Process 1: Counter starts at %d\n", shared_mem->counter);
    
    pid_t pid = fork();   // create a new process

    if (pid < 0) {
        // fork failed
        perror("fork failed");
        shmdt(shared_mem);
        semctl(semid, 0, IPC_RMID);
        shmctl(shmid, IPC_RMID, NULL);
        exit(EXIT_FAILURE);
    }

    if (pid == 0) {
        // Child process: detach and execute process2
        shmdt(shared_mem);  // Detach before exec
        
        printf("Process 1: Launching Process 2...\n");
        execl("./q5_child", "q5_child", NULL);
        
        // If execl returns, it failed
        perror("execl failed");
        exit(EXIT_FAILURE);
    } else {
        // Parent process: process 1 (incrementing)
        int status;
        pid_t result;
        int local_counter;
        int local_multiple;
        
        while (1) {
            // Check if Process 2 has finished (non-blocking)
            result = waitpid(pid, &status, WNOHANG);
            
            if (result > 0) {
                // Process 2 has finished
                sem_wait(semid);
                local_counter = shared_mem->counter;
                sem_signal(semid);
                
                printf("\nProcess 1: Process 2 (PID %d) has finished!\n", pid);
                printf("Process 1: Final counter value: %d\n", local_counter);
                printf("Process 1: Ending now...\n");
                break;
            } else if (result < 0) {
                perror("waitpid failed");
                break;
            }
            
            // Read shared memory (protected)
            sem_wait(semid);
            local_counter = shared_mem->counter;
            local_multiple = shared_mem->multiple;
            
            // Check if counter exceeds 500
            if (local_counter > 500) {
                sem_signal(semid);
                printf("\nProcess 1: Counter (%d) exceeded 500!\n", local_counter);
                printf("Process 1: Ending now...\n");
                break;
            }
            
            // Display cycle information
            if (local_counter % local_multiple == 0) {
                printf("Process 1 - Cycle: %d - %d is a multiple of %d (protected shared memory)\n", 
                       local_counter, local_counter, local_multiple);
            } else {
                printf("Process 1 - Cycle: %d\n", local_counter);
            }
            
            // Increment the shared counter
            shared_mem->counter++;
            sem_signal(semid);
            
            sleep(0.5);   // sleep 0.5s to slow the output
        }
        
        // Detach and remove shared memory and semaphore
        shmdt(shared_mem);
        semctl(semid, 0, IPC_RMID);
        shmctl(shmid, IPC_RMID, NULL);
        printf("Process 1: Shared memory and semaphore cleaned up\n");
    }

    return 0;
}