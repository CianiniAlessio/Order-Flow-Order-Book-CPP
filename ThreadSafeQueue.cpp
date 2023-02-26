
#include "ThreadSafeQueue.h"




ThreadSafeQueue::ThreadSafeQueue() {

}


void ThreadSafeQueue::push(std::string item) {
    std::lock_guard<std::mutex> lock(m_mutex);
    m_queue->push(item);
}


void ThreadSafeQueue::pop() {
    std::lock_guard<std::mutex> lock(m_mutex);
    m_queue->pop();

}



std::string ThreadSafeQueue::front() {
    std::lock_guard<std::mutex> lock(m_mutex);
    std::string item = m_queue->front();
    return item;
}


bool ThreadSafeQueue::is_empty() {
    std::lock_guard<std::mutex> lock(m_mutex);
    return m_queue->empty();
}


size_t ThreadSafeQueue::size() {
    std::lock_guard<std::mutex> lock(m_mutex);
    return m_queue->size();
}
