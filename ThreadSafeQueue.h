#pragma once
#ifndef QUEUETHREAD_H
#define QUEUETHREAD_H
#include <queue>
#include <mutex>
#include <string>

class ThreadSafeQueue {
private:
    std::queue<std::string> m_queue;
    std::mutex m_mutex;

public:
    ThreadSafeQueue();
    void push(std::string item);
    void pop();
    std::string front();
    bool is_empty();
    size_t size();
};

#endif 