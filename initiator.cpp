#include <iostream>
#include <unistd.h>
#include <sys/mman.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <semaphore.h>
#include <cstring>
#include <signal.h>
#include "Logger.h"

#define SHM_NAME "/shared_counter"
#define SEM_INIT_NAME "/sem_inititor"
#define SEM_RECEIVE_NAME "/sem_receive"

// Global variables for cleanup in signal handler
int shm_fd = -1;
int *counter = nullptr;
sem_t *sem_inititor = nullptr;
sem_t *sem_receive = nullptr;

// Signal handler for graceful termination
void cleanup(int sig)
{
    munmap(counter, sizeof(int));
    close(shm_fd);
    sem_close(sem_inititor);
    sem_unlink(SEM_INIT_NAME);
    sem_close(sem_receive);
    sem_unlink(SEM_RECEIVE_NAME);
    shm_unlink(SHM_NAME);
    exit(sig);
}

int main()
{
    Logger logger("program_log.txt"); // Create a logger instance to log to file and stdout

    // Setup signal handlers
    signal(SIGINT, cleanup);
    signal(SIGTERM, cleanup);

    // Open shared memory object
    shm_fd = shm_open(SHM_NAME, O_CREAT | O_RDWR, 0666);
    if (shm_fd == -1)
    {
        logger.log("Failed to open shared memory.");
        return 1;
    }

    // Set the size of the shared memory region
    if (ftruncate(shm_fd, sizeof(int)) == -1)
    {
        logger.log("Failed to truncate shared memory.");
        close(shm_fd); // Clean up before exiting
        shm_unlink(SHM_NAME);
        return 1;
    }

    // Map shared memory
    counter = (int *)mmap(nullptr, sizeof(int), PROT_READ | PROT_WRITE, MAP_SHARED, shm_fd, 0);
    if (counter == MAP_FAILED)
    {
        logger.log("Failed to map shared memory.");
        close(shm_fd); // Clean up before exiting
        shm_unlink(SHM_NAME);
        return 1;
    }

    close(shm_fd); // No longer needed

    // Initialize the counter
    *counter = 0;

    // Open semaphores
    sem_inititor = sem_open(SEM_INIT_NAME, O_CREAT, 0666, 0);
    sem_receive = sem_open(SEM_RECEIVE_NAME, O_CREAT, 0666, 0);

    if (sem_inititor == SEM_FAILED || sem_receive == SEM_FAILED)
    {
        logger.log("Failed to open semaphores.");
        cleanup(0);
        return 1;
    }

    while (*counter < 10)
    {
        logger.log("Initiator sends value: " + std::to_string(*counter));
        sem_post(sem_inititor);

        // Wait with timeout (5 seconds)
        struct timespec ts;
        if (clock_gettime(CLOCK_REALTIME, &ts) == -1)
        {
            logger.log("Failed to get time.");
            cleanup(errno);
            return 1;
        }
        ts.tv_sec += 5; // 5 second timeout

        if (sem_timedwait(sem_receive, &ts) == -1)
        {
            if (errno == ETIMEDOUT)
            {
                logger.log("Timeout waiting for receiver. Exiting.");
                cleanup(errno);
                return 1;
            }
            else
            {
                logger.log("Error waiting for semaphore.");
                cleanup(errno);
                return 1;
            }
        }

        // Increment the counter
        (*counter)++;
    }
    
    sem_post(sem_inititor);

    logger.log("Initiator process finished. Counter reached 10.");

    // Clean up all resources
    cleanup(0);

    return 0;
}
