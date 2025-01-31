#include <iostream>
#include <unistd.h>
#include <sys/mman.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <semaphore.h>
#include <cstring>

#define SHM_NAME "/shared_counter"
#define SEM_INIT_NAME "/sem_init"
#define SEM_RECEIVE_NAME "/sem_receive"

int main()
{
    // Open shared memory object
    int shm_fd = shm_open(SHM_NAME, O_RDWR, 0666);
    if (shm_fd == -1)
    {
        std::cerr << "Failed to open shared memory. Please first run the inititor." << std::endl;
        return 1;
    }

    // Map shared memory
    int *counter = (int *)mmap(nullptr, sizeof(int), PROT_READ | PROT_WRITE, MAP_SHARED, shm_fd, 0);
    if (counter == MAP_FAILED)
    {
        std::cerr << "Failed to map shared memory." << std::endl;
        return 1;
    }

    // Open semaphores
    sem_t *sem_init = sem_open(SEM_INIT_NAME, 0);       // Initial semaphore (waiting for initiator)
    sem_t *sem_receive = sem_open(SEM_RECEIVE_NAME, 0); // Receiver semaphore

    if (sem_init == SEM_FAILED || sem_receive == SEM_FAILED)
    {
        std::cerr << "Failed to open semaphores." << std::endl;
        return 1;
    }

    while (*counter < 10)
    {
        // Increment the counter
        (*counter)++;

        std::cout << "Receiver receives and increments value: " << *counter << std::endl;

        // Wake up the initiator by posting on the receive semaphore
        sem_post(sem_receive);
        // Wait for the initiator to send the counter value
        sem_wait(sem_init);
    }

    std::cout << "Receiver process finished. Counter reached 10." << std::endl;

    // Clean up semaphores
    sem_close(sem_init);
    sem_close(sem_receive);

    return 0;
}
