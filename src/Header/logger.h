#ifndef LOGGER_H
#define LOGGER_H

#include <mutex>
#include <fstream>
#include <string>
#include <chrono>
#include <iostream>

class Logger {
public:
    Logger();
    ~Logger();
    void writeToLog(std::string message);
private:
    std::mutex lock;

};

#endif
