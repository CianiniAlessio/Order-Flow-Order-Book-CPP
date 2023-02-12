#include <iomanip>
#include <cmath>
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
#define SIZE_OF_TRADES 6
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


void set_price_quantity_orderBook(std::map<double, double>* ask, std::map<double, double, std::greater<double>>* bid, const std::vector<std::string>& quotes) {
    
    
    if(quotes.size() == SIZE_OF_QUOTES) {
        if(quotes[SIDE_ORDER_BOOK_POS] == "a") {
            if(std::stod(quotes[QUANTITY_ORDER_BOOK_POS]) == 0) {
                (*ask).erase(std::stod(quotes[PRICE_ORDER_BOOK_POS]));
            }
            else {
                (*ask)[std::stod(quotes[PRICE_ORDER_BOOK_POS])] = std::stod(quotes[QUANTITY_ORDER_BOOK_POS]);
            }
        }
        else {
            if(std::stod(quotes[QUANTITY_ORDER_BOOK_POS]) == 0) {
                (*bid).erase(std::stod(quotes[PRICE_ORDER_BOOK_POS]));
            }
            else {
                (*bid)[std::stod(quotes[PRICE_ORDER_BOOK_POS])] = std::stod(quotes[QUANTITY_ORDER_BOOK_POS]);
            }
        }
    }
}



void manage_trade_in_orderBook(std::map<double, double>* ask_map, std::map<double, double, std::greater<double>>* bid_map, const std::vector<std::string>& order) {
    double price = std::stod(order[1]);
    double qty = std::stod(order[2]);
    std::string timestamp = order[4];
    bool isBuyerMaker = (order[5] == "true");
    
    //  if false then means they hit a sell limit, therefore market order buy, i'll check the ask
    if (!isBuyerMaker) { 
        auto it = ask_map->begin();
        while (it != ask_map->end() && qty > 0) {
            if (it->first <= price) {
                if (qty >= it->second) {
                    qty -= it->second;
                    it = ask_map->erase(it);
                } else {
                    (*ask_map)[it->first] -= qty;
                    qty = 0;
                }
                auto end_map = ask_map->end(); 
                if (it == end_map) {
                    break;
                }
                else ++it;
            } else {
                break;
            }
        }
    } 
    // if true the buyer is maker, that means they hit a limitorder buy, therefore sell was at market price (taker)
    else { 
        auto it = bid_map->begin();
        while (it != bid_map->end() && qty > 0) {
            if (it->first >= price) {
                if (qty >= it->second) {
                    qty -= it->second;
                    it = bid_map->erase(it);
                } else {
                    (*bid_map)[it->first] -= qty;
                    qty = 0;
                }
                auto end_map = bid_map->end(); 
                if (it == end_map) {
                    break;
                }
                else ++it;
            } else {
                break;
            }
        }
    }
}

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
    std::string a;

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
    std::string n;

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

        if (Orderbook_queue->empty() || Trades_queue->empty())
        {
            break;
        }

        // HERE I CREATE THE ORDER BOOK
        if (!Orderbook_queue->empty())
        {
            quotes = splitLine(Orderbook_queue->front(),',');
                
        }

        // HERE I SHOULD CHANGE THE ORDERBOOK BASED ON THE TRADES (ONGOING)
        if (!Trades_queue->empty())
        {
            executed = splitLine(Trades_queue->front(),','); // change order book thanks to this
        }



        // HERE I HAVE TWO QUEUE FROM BEFORE WAITING TO BE RED, I CHECK FOR PROBLEMS IN THE DATA, IF THERE ARE PROBLEMS I POP THE DATA WITHOUT DOING NOTHING
        // IF EVERYTHING GOES SMOOTHLY I INSERT ALL MY DATA IN A VECTOR CALLED TEMP WHICH I WILL USE LATER I SUPPOSE. BUT I THINK IT WILL NOT BE NECESSARY 

        if (quotes.size()!= SIZE_OF_QUOTES) // Mmeaning there is some problem in the quotes (maybe finished file?)
        {
            if(executed.size() == SIZE_OF_TRADES) // but i have an executed
            {
                    
                    manage_trade_in_orderBook(ask,bid,executed);
                    Trades_queue->pop();
            }
            else{
              break;
            }
        }
        else
        {  // quotes is ok
            if (executed.size()!= SIZE_OF_TRADES) // but maybe i don't have trade 
            {
                    
                    // so i change the orderbook
                    
                    set_price_quantity_orderBook(ask,bid,quotes);
                    Orderbook_queue->pop();
            }
            else{  // or maybe i have trades
            // so i check the timestamp if i have both 
                //IF BOTH GOOD CHECK TIMESTAMP
                  if(std::stol(executed[TIMESTAMP_TRADE_POS]) <= std::stol(quotes[TIMESTAMP_ORDER_BOOK_POS]))
                  {
                    // I CHECK THE ORDERBOOK LEVELS AND SEE IF I CAN EXECUTE 
                  
                    manage_trade_in_orderBook(ask,bid,executed);
                    Trades_queue->pop();
                  }
                  else
                  {
                  
                    set_price_quantity_orderBook(ask,bid,quotes);
                    Orderbook_queue->pop();
                  }
            }
        }
        
        
    }
}





int main()
{

    std::cout << " starting " << std::endl;
    std::thread processThreadOB(processFilesOB);
    std::thread processThreadTR(processFilesTR);
    std::cout << " reading " << std::endl;

    std::thread readThread(readFromQueues);
    
    processThreadOB.join();
    processThreadTR.join();

    std::cout << " join third " << std::endl;
    std::this_thread::sleep_for(std::chrono::seconds(3));
    readThread.join();
    std::cout << "finished all " << std::endl;
    auto it1 = (*ask).begin();
    auto it2 = (*bid).begin();
    int count = 0;
    while (it1 != (*ask).end() && it2 != (*bid).end() ) {
        std::cout  << std::fixed <<std::setprecision(2) << "ASK " << it1->second << ", " << it1->first << " | ";

        std::cout << std::fixed << std::setprecision(2) << it2->first << " , " << it2->second << " BID" << std::endl;
        count++;
        ++it1;
        ++it2;
        
    }   

   
    return 0;
}
