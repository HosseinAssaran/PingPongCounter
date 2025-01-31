#include <iostream>
#include <unistd.h>
#include <sys/mman.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <semaphore.h>
#include <cstring>
#include "Logger.h"

#define SHM_NAME "/shared_counter"
#define SEM_INIT_NAME "/sem_init"
#define SEM_RECEIVE_NAME "/sem_receive"

int main()
{
    Logger logger("program_log.txt"); // Create a logger instance to log to file and stdout

    // Open shared memory object
    int shm_fd = shm_open(SHM_NAME, O_CREAT | O_RDWR, 0666);
    if (shm_fd == -1)
    {
        logger.log("Failed to open shared memory.");
        return 1;
    }

    // Set the size of the shared memory region
    if (ftruncate(shm_fd, sizeof(int)) == -1)
    {
        logger.log("Failed to truncate shared memory.");
        return 1;
    }

    // Map shared memory
    int *counter = (int *)mmap(nullptr, sizeof(int), PROT_READ | PROT_WRITE, MAP_SHARED, shm_fd, 0);
    if (counter == MAP_FAILED)
    {
        logger.log("Failed to map shared memory.");
        return 1;
    }

    // Initialize the counter
    *counter = 0;

    // Open semaphores
    sem_t *sem_init = sem_open(SEM_INIT_NAME, O_CREAT, 0666, 0);       // Initial semaphore (initialized to 0)
    sem_t *sem_receive = sem_open(SEM_RECEIVE_NAME, O_CREAT, 0666, 0); // Receiver semaphore (initialized to 0)

    if (sem_init == SEM_FAILED || sem_receive == SEM_FAILED)
    {
        logger.log("Failed to open semaphores.");
        return 1;
    }

    while (*counter < 10)
    {
        // Send the current counter value to the receiver
        logger.log("Initiator sends value: " + std::to_string(*counter));

        // Wake up the receiver by posting on the init semaphore
        sem_post(sem_init);

        // Wait for the receiver to update the counter by waiting on the receive semaphore
        sem_wait(sem_receive);

        // Increment the counter
        (*counter)++;
    }
    
    sem_post(sem_init);

    logger.log("Initiator process finished. Counter reached 10.");

    // Clean up semaphores
    sem_close(sem_init);
    sem_close(sem_receive);
    sem_unlink(SEM_INIT_NAME);
    sem_unlink(SEM_RECEIVE_NAME);

    // Clean up shared memory
    shm_unlink(SHM_NAME);

    return 0;
}
