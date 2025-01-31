#include <iostream>
#include <fstream>
#include <sys/socket.h>
#include <sys/un.h>
#include <unistd.h>
#include <cstdlib>
#include <cstring>
#include <thread>
#include <chrono>

const char* SOCKET_PATH = "/tmp/process_communication.sock";
const int LIMIT = 10;
std::ofstream log_file("process_log.txt", std::ios::app);

void log_message(const std::string& message) {
    log_file << message << std::endl;
    std::cout << message << std::endl;
}

void initiator() {
    log_message("Initiator: Starting process.");
    
    int sockfd;
    struct sockaddr_un addr;
    sockfd = socket(AF_UNIX, SOCK_STREAM, 0);
    if (sockfd == -1) {
        log_message("Initiator: Socket creation failed.");
        return;
    }
    
    addr.sun_family = AF_UNIX;
    std::strncpy(addr.sun_path, SOCKET_PATH, sizeof(addr.sun_path) - 1);
    
    while (connect(sockfd, (struct sockaddr*)&addr, sizeof(addr)) == -1) {
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
    }
    
    int counter = 1;
    while (counter <= LIMIT) {
        log_message("Initiator: Sending " + std::to_string(counter));
        send(sockfd, &counter, sizeof(counter), 0);
        recv(sockfd, &counter, sizeof(counter), 0);
        log_message("Initiator: Received " + std::to_string(counter));
        counter++;
    }
    
    close(sockfd);
    log_message("Initiator: Process complete.");
}

void receiver() {
    log_message("Receiver: Starting process.");
    
    int sockfd, clientfd;
    struct sockaddr_un addr;
    
    sockfd = socket(AF_UNIX, SOCK_STREAM, 0);
    if (sockfd == -1) {
        log_message("Receiver: Socket creation failed.");
        return;
    }
    
    addr.sun_family = AF_UNIX;
    std::strncpy(addr.sun_path, SOCKET_PATH, sizeof(addr.sun_path) - 1);
    unlink(SOCKET_PATH);
    
    if (bind(sockfd, (struct sockaddr*)&addr, sizeof(addr)) == -1) {
        log_message("Receiver: Bind failed.");
        close(sockfd);
        return;
    }
    
    if (listen(sockfd, 1) == -1) {
        log_message("Receiver: Listen failed.");
        close(sockfd);
        return;
    }
    
    clientfd = accept(sockfd, nullptr, nullptr);
    if (clientfd == -1) {
        log_message("Receiver: Accept failed.");
        close(sockfd);
        return;
    }
    
    int counter;
    while (recv(clientfd, &counter, sizeof(counter), 0) > 0) {
        log_message("Receiver: Received " + std::to_string(counter));
        counter++;
        send(clientfd, &counter, sizeof(counter), 0);
        log_message("Receiver: Sent " + std::to_string(counter));
    }
    
    close(clientfd);
    close(sockfd);
    unlink(SOCKET_PATH);
    log_message("Receiver: Process complete.");
}

int main() {
    pid_t pid = fork();
    if (pid == -1) {
        log_message("Error: Fork failed.");
        return EXIT_FAILURE;
    }
    
    if (pid == 0) {
        receiver();
    } else {
        std::this_thread::sleep_for(std::chrono::milliseconds(500));
        initiator();
    }
    
    return EXIT_SUCCESS;
}
