#include "logger.h"

Logger::Logger() {
    
}

Logger::~Logger() {
    std::cout << std::flush;
}

void Logger::writeToLog(std::string message) {
    lock.lock();
    auto now = std::chrono::system_clock::now();
    //if (message.find("size")) std::cout << boost::stacktrace::stacktrace();
    std::time_t timestamp = std::chrono::system_clock::to_time_t(now);
    std::cout << "[" <<timestamp << "] " << message << std::endl << std::flush;
    lock.unlock();
}
 