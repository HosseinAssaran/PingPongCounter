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
    int shm_fd = shm_open(SHM_NAME, O_CREAT | O_RDWR, 0666);
    if (shm_fd == -1)
    {
        std::cerr << "Failed to open shared memory." << std::endl;
        return 1;
    }

    // Set the size of the shared memory region
    if (ftruncate(shm_fd, sizeof(int)) == -1)
    {
        std::cerr << "Failed to truncate shared memory." << std::endl;
        return 1;
    }

    // Map shared memory
    int *counter = (int *)mmap(nullptr, sizeof(int), PROT_READ | PROT_WRITE, MAP_SHARED, shm_fd, 0);
    if (counter == MAP_FAILED)
    {
        std::cerr << "Failed to map shared memory." << std::endl;
        return 1;
    }

    // Initialize the counter
    *counter = 0;

    // Open semaphores
    sem_t *sem_init = sem_open(SEM_INIT_NAME, O_CREAT, 0666, 0);       // Initial semaphore (initialized to 0)
    sem_t *sem_receive = sem_open(SEM_RECEIVE_NAME, O_CREAT, 0666, 0); // Receiver semaphore (initialized to 0)

    if (sem_init == SEM_FAILED || sem_receive == SEM_FAILED)
    {
        std::cerr << "Failed to open semaphores." << std::endl;
        return 1;
    }

    while (*counter < 10)
    {
        // Send the current counter value to the receiver
        std::cout << "Initiator sends value: " << *counter << std::endl;

        // Wake up the receiver by posting on the init semaphore
        sem_post(sem_init);

        // Wait for the receiver to update the counter by waiting on the receive semaphore
        sem_wait(sem_receive);

        // Increment the counter
        (*counter)++;
    }

    sem_post(sem_init);

    std::cout << "Initiator process finished. Counter reached 10." << std::endl;

    // Clean up semaphores
    sem_close(sem_init);
    sem_close(sem_receive);
    sem_unlink(SEM_INIT_NAME);
    sem_unlink(SEM_RECEIVE_NAME);

    // Clean up shared memory
    shm_unlink(SHM_NAME);

    return 0;
}
