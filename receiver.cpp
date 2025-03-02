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
    sem_close(sem_receive);
    exit(sig);
}

int main()
{
    Logger logger("program_log.txt"); // Create a logger instance to log to file and stdout

    // Setup signal handlers
    signal(SIGINT, cleanup);
    signal(SIGTERM, cleanup);

    // Open shared memory with retry logic
    int retry_count = 0;
    const int max_retries = 5;

    while (retry_count < max_retries)
    {
        shm_fd = shm_open(SHM_NAME, O_RDWR, 0666);
        if (shm_fd != -1)
            break;

        logger.log("Waiting for initiator to create shared memory... (attempt " +
                   std::to_string(retry_count + 1) + "/" + std::to_string(max_retries) + ")");
        sleep(1);
        retry_count++;
    }

    if (shm_fd == -1)
    {
        logger.log("Failed to open shared memory after maximum retries. Please run the initiator first.");
        return 1;
    }

    // Map shared memory
    int *counter = (int *)mmap(nullptr, sizeof(int), PROT_READ | PROT_WRITE, MAP_SHARED, shm_fd, 0);
    if (counter == MAP_FAILED)
    {
        logger.log("Failed to map shared memory.");
        close(shm_fd);
        return 1;
    }

    // Open semaphores
    sem_t *sem_inititor = sem_open(SEM_INIT_NAME, 0);   // Initial semaphore (waiting for initiator)
    sem_t *sem_receive = sem_open(SEM_RECEIVE_NAME, 0); // Receiver semaphore

    if (sem_inititor == SEM_FAILED || sem_receive == SEM_FAILED)
    {
        logger.log("Failed to open semaphores.");
        cleanup(0);
        return 1;
    }

    while (*counter < 10)
    {
        // Wait for the initiator with timeout
        struct timespec ts;
        if (clock_gettime(CLOCK_REALTIME, &ts) == -1)
        {
            logger.log("Failed to get time.");
            cleanup(0);
            return 1;
        }
        ts.tv_sec += 5; // 5 second timeout

        if (sem_timedwait(sem_inititor, &ts) == -1)
        {
            if (errno == ETIMEDOUT)
            {
                logger.log("Timeout waiting for initiator. Exiting.");
                cleanup(0);
                return 1;
            }
            else
            {
                logger.log("Error waiting for semaphore.");
                cleanup(0);
                return 1;
            }
        }
        
        if (*counter == 10)
            break;

        // Increment the counter
        (*counter)++;

        logger.log("Receiver send value: " + std::to_string(*counter));

        // Wake up the initiator by posting on the receive semaphore
        sem_post(sem_receive);
    }

    logger.log("Receiver process finished. Counter reached 10.");

    cleanup(0);

    return 0;
}
