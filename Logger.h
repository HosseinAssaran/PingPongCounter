#ifndef LOGGER_H
#define LOGGER_H

#include <iostream>
#include <fstream>
#include <string>
#include <mutex>

class Logger
{
public:
    Logger(const std::string &log_filename) : log_file(log_filename, std::ios::app)
    {
        if (!log_file.is_open())
        {
            std::cerr << "Failed to open log file." << std::endl;
            exit(1);
        }
    }

    ~Logger()
    {
        if (log_file.is_open())
        {
            log_file.close();
        }
    }

    void log(const std::string &message)
    {
        std::lock_guard<std::mutex> lock(mutex); // Ensure thread safety
        std::cout << message << std::endl;       // Output to stdout
        log_file << message << std::endl;        // Append to log file
    }

private:
    std::ofstream log_file;
    std::mutex mutex; // Protects log file access
};

#endif // LOGGER_H
