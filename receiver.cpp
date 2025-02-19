#include <iostream>
#include <unistd.h>
#include <sys/mman.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <semaphore.h>
#include <cstring>
#include "Logger.h"

#define SHM_NAME "/shared_counter"
#define SEM_INIT_NAME "/sem_inititor"
#define SEM_RECEIVE_NAME "/sem_receive"

int main()
{
    Logger logger("program_log.txt"); // Create a logger instance to log to file and stdout

    // Open shared memory object
    int shm_fd = shm_open(SHM_NAME, O_RDWR, 0666);
    if (shm_fd == -1)
    {
        logger.log("Failed to open shared memory. Please first run the inititor.");
        return 1;
    }

    // Map shared memory
    int *counter = (int *)mmap(nullptr, sizeof(int), PROT_READ | PROT_WRITE, MAP_SHARED, shm_fd, 0);
    if (counter == MAP_FAILED)
    {
        logger.log("Failed to map shared memory.");
        return 1;
    }

    // Open semaphores
    sem_t *sem_inititor = sem_open(SEM_INIT_NAME, 0);       // Initial semaphore (waiting for initiator)
    sem_t *sem_receive = sem_open(SEM_RECEIVE_NAME, 0); // Receiver semaphore

    if (sem_inititor == SEM_FAILED || sem_receive == SEM_FAILED)
    {
        logger.log("Failed to open semaphores.");
        return 1;
    }

    // Wait for the initiator to send the counter value
    sem_wait(sem_inititor);

    while (*counter < 10)
    {
        // Increment the counter
        (*counter)++;

        logger.log("Receiver send value: " + std::to_string(*counter));

        // Wake up the initiator by posting on the receive semaphore
        sem_post(sem_receive);
        // Wait for the initiator to send the counter value
        sem_wait(sem_inititor);
    }

    logger.log("Receiver process finished. Counter reached 10.");

    // Clean up semaphores
    sem_close(sem_inititor);
    sem_close(sem_receive);

    return 0;
}
