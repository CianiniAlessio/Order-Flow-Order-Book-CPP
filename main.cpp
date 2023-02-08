#include <iostream>
#include <fstream>
#include <string>
#include <queue>
#include <thread>
#include <chrono>
#include <mutex>

std::queue<std::string> Orderbook_queue, Trades_queue;
std::mutex queueMutex;

void processFiles()
{
    std::fstream quotes("/home/alessio/Desktop/prove/Data/orderbook.csv");
    std::fstream trade("/home/alessio/Desktop/prove/Data/trade_data.csv");
    std::string n, a;
    while (std::getline(quotes, n) && std::getline(trade, a))
    {
        std::unique_lock<std::mutex> lock(queueMutex);
        Orderbook_queue.push(n);
        Trades_queue.push(a);
        lock.unlock();
    }
}

void readFromQueues()
{
    std::this_thread::sleep_for(std::chrono::seconds(5));
    while (true)
    {
        std::unique_lock<std::mutex> lock(queueMutex);
        if (Orderbook_queue.empty() || Trades_queue.empty())
        {
            lock.unlock();
            break;
        }
        std::string _quote = Orderbook_queue.front();
        std::string _trade = Trades_queue.front();
        Orderbook_queue.pop();
        Trades_queue.pop();
        lock.unlock();
        std::cout << _quote << " " << _trade << std::endl;
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
