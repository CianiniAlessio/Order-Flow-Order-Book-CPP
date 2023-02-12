#include <iostream>
#include <fstream>
#include <string>
#include <queue>
#include <thread>
#include <chrono>
#include <vector>
#include <mutex>
#include <map>
#include <algorithm>
#include <unordered_map>
#include <map>

#define SIZE_OF_QUOTES 9
#define TIMESTAMP_TRADE_POS 4
#define TIMESTAMP_ORDER_BOOK_POS 1
#define SIDE_ORDER_BOOK_POS 4
#define PRICE_ORDER_BOOK_POS 6
#define QUANTITY_ORDER_BOOK_POS 7
std::queue<std::string>* Orderbook_queue = new std::queue<std::string>();
std::queue<std::string>* Trades_queue = new std::queue<std::string>();
std::map<double, double>* ask = new std::map<double, double>();  
std::map<double, double, std::greater<double>>* bid = new std::map<double, double,std::greater<double>>();  // no sense to retrieve all the orderbook everytime
std::mutex queueMutexOB,queueMutexTR;

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

void processFilesTR()
{
    std::fstream* trade = new std::fstream("/home/alessio/Desktop/prove/Data/trade_data.csv");
    std::string n, a;

    while (trade->good())
    {
        std::getline(*trade, a);

        if (trade->eof())
        {
            break;
        }

        
        std::unique_lock<std::mutex> lock(queueMutexTR);
        if (!a.empty())
        {
            Trades_queue->push(a);
        }
        
    }
}

void processFilesOB()
{
    std::fstream* quotes = new std::fstream("/home/alessio/Desktop/prove/Data/orderbook.csv");
    std::string n, a;

    while (quotes->good())
    {
        std::getline(*quotes, n);

        if (quotes->eof() )
        {
            break;
        }

        
        std::unique_lock<std::mutex> lock(queueMutexOB);
        if (!n.empty())
        {
            Orderbook_queue->push(n);
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
        std::unique_lock<std::mutex> lockOB(queueMutexOB, std::try_to_lock);
        if (!lockOB.owns_lock())
        {
            std::this_thread::sleep_for(std::chrono::milliseconds(100));
            continue;
        }
        std::unique_lock<std::mutex> lockTR(queueMutexTR, std::try_to_lock);
        
      
        if (!lockTR.owns_lock())
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
            if(quotes.size() == SIZE_OF_QUOTES)
            {

                
                if(quotes[SIDE_ORDER_BOOK_POS]=="a") // need to add destroyer in case qty = 0 to save memory adn remove the price
                {
                        if(std::stod(quotes[QUANTITY_ORDER_BOOK_POS]) == 0) (*ask).erase(std::stod(quotes[PRICE_ORDER_BOOK_POS]));
                        else (*ask)[std::stod(quotes[PRICE_ORDER_BOOK_POS])] = std::stod(quotes[QUANTITY_ORDER_BOOK_POS]);
                }
                else
                {
                        if(std::stod(quotes[QUANTITY_ORDER_BOOK_POS])==0) (*bid).erase(std::stod(quotes[PRICE_ORDER_BOOK_POS]));
                        else  (*bid)[std::stod(quotes[PRICE_ORDER_BOOK_POS])] = std::stod(quotes[QUANTITY_ORDER_BOOK_POS]);
                }
            }
                
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





int main()
{


    ///home/acianini/Desktop/Thesis/trial
    std::thread processThreadOB(processFilesOB);
    std::thread processThreadTR(processFilesTR);
    
    std::thread readThread(readFromQueues);
    
    processThreadOB.join();
    processThreadTR.join();

    std::this_thread::sleep_for(std::chrono::seconds(10));
    readThread.join();
    auto it1 = (*ask).begin();
    auto it2 = (*ask).begin();
    auto end1 = (*bid).end();
    auto end2 = (*bid).end();

    while (it1 != end1 && it2 != end2) {
        std::cout << "ASK " << it1->second << ", " << it1->first << " | ";

        std::cout << it2->first << " , " << it2->second << " BID" << std::endl;

        ++it1;
        ++it2;
    }



    
   
    return 0;
}
