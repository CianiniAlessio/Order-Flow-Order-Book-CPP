#include <iostream>
#include <fstream>
#include <string>
#include <queue>
#include <thread>
#include <chrono>
#include <vector>
#include <mutex>
#include <map>

#define TIMESTAMP_TRADE_POS 4
#define TIMESTAMP_ORDER_BOOK_POS 1
std::queue<std::string>* Orderbook_queue = new std::queue<std::string>();
std::queue<std::string>* Trades_queue = new std::queue<std::string>();
std::map<long,std::string>* data = new std::map<long, std::string>();
std::mutex queueMutex;

std::vector<std::string> splitLine(const std::string& line, char delimiter)
{
    std::vector<std::string> elements;
    std::string element;
    for (const char& c : line)
    {
        if (c == delimiter)
        {
            elements.push_back(element);
            element.clear();
        }
        else
        {
            element += c;
        }
    }
    elements.push_back(element);
    return elements;
}

void processFiles()
{
    std::fstream* quotes = new std::fstream("/home/alessio/Desktop/prove/Data/orderbook.csv");
    std::fstream* trade = new std::fstream("/home/alessio/Desktop/prove/Data/trade_data.csv");
    std::string n, a;

    while (quotes->good() || trade->good())
    {
        std::getline(*quotes, n);
        std::getline(*trade, a);

        if (quotes->eof() && trade->eof())
        {
            break;
        }

        {
            std::unique_lock<std::mutex> lock(queueMutex);
            if (!n.empty())
            {
                Orderbook_queue->push(n);
            }
            if (!a.empty())
            {
                Trades_queue->push(a);
            }
        }
    }
}
int i = 0;
std::vector<std::string>* temp = new std::vector<std::string>();
void readFromQueues()
{
    std::this_thread::sleep_for(std::chrono::seconds(5));
    std::vector<std::string>  quotes, executed;
    while (true)
    {
        std::unique_lock<std::mutex> lock(queueMutex, std::try_to_lock);
        //std::cout << i++ << std::endl;
        if (!lock.owns_lock())
        {
            std::this_thread::sleep_for(std::chrono::milliseconds(100));
            continue;
        }

        if (Orderbook_queue->empty() && Trades_queue->empty())
        {
            break;
        }
        if (!Orderbook_queue->empty())
        {
            quotes = splitLine(Orderbook_queue->front(),',');
        }
        if (!Trades_queue->empty())
        {
            executed = splitLine(Trades_queue->front(),',');
        }

        if (quotes.size()!= 9)
        {
            if(executed.size() == 6)
            {
                    temp->push_back(executed[TIMESTAMP_TRADE_POS]);
                    Trades_queue->pop();
            }
            else{
              break;
            }
        }
        else
        {
            if (executed.size()!= 6)
            {
                    temp->push_back(quotes[TIMESTAMP_ORDER_BOOK_POS]);
                    Orderbook_queue->pop();
            }
            else{
                  if(std::stol(executed[TIMESTAMP_TRADE_POS]) <= std::stol(quotes[TIMESTAMP_ORDER_BOOK_POS]))
                  {
                    temp->push_back(executed[TIMESTAMP_TRADE_POS]);
                    Trades_queue->pop();
                  }
                  else
                  {
                    temp->push_back(quotes[TIMESTAMP_ORDER_BOOK_POS]);
                    Orderbook_queue->pop();
                  }
            }
        }
        
        
    }
}



#include <algorithm>


int main()
{
    std::thread processThread(processFiles);
    
    std::thread readThread(readFromQueues);
    
    processThread.join();

    std::this_thread::sleep_for(std::chrono::seconds(10));
    readThread.join();
    
    
    std::cout << " finished ";
    return 0;
}
