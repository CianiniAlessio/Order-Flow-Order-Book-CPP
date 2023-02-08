#include <iostream>
#include <fstream>
#include <string>
#include <queue>
#include <thread>
#include <chrono>
#include <mutex>

std::queue<std::string> namesQueue, addressesQueue;
std::mutex queueMutex;

void processFiles()
{
    std::fstream names("/home/alessio/Desktop/prove/Data/orderbook.csv");
    std::fstream add("/home/alessio/Desktop/prove/Data/trade_data.csv");
    std::string n, a;
    while (std::getline(names, n) && std::getline(add, a))
    {
        std::unique_lock<std::mutex> lock(queueMutex);
        namesQueue.push(n);
        addressesQueue.push(a);
        lock.unlock();
    }
}

void readFromQueues()
{
    std::this_thread::sleep_for(std::chrono::seconds(5));
    while (true)
    {
        std::unique_lock<std::mutex> lock(queueMutex);
        if (namesQueue.empty() || addressesQueue.empty())
        {
            lock.unlock();
            break;
        }
        std::string name = namesQueue.front();
        std::string address = addressesQueue.front();
        namesQueue.pop();
        addressesQueue.pop();
        lock.unlock();
        std::cout << name << " " << address << std::endl;
    }
}

int main()
{
    std::thread processThread(processFiles);
    std::thread readThread(readFromQueues);

    processThread.join();
    readThread.join();

    return 0;
}
