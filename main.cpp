#include <iomanip>
#include <cmath>
#include <iostream>
#include <fstream>
#include <string>
#include <queue>
#include <thread>
#include <chrono>
#include <vector>
#include <map>
#include <algorithm>
#include <unordered_map>
#include "logger.h"
#include "ThreadSafeQueue.h"
#include "VPIN.h"
#include <memory>

#define SIZE_OF_QUOTES 9
#define SIZE_OF_TRADES 6
#define TIMESTAMP_TRADE_POS 4
#define TIMESTAMP_ORDER_BOOK_POS 1
#define SIDE_ORDER_BOOK_POS 4
#define PRICE_ORDER_BOOK_POS 6
#define QUANTITY_ORDER_BOOK_POS 7
// no sense to retrieve all the orderbook everytime
Logger my_log;
std::mutex m;
std::condition_variable cv;
bool ready1 = false, ready2 = false;

// i pass the set of the level as if it was an order and return the quantity not filled and put it in the book
void manage_trade_in_orderBook(std::map<double, double>& ask_map, std::map<double, double, std::greater<double>>& bid_map, double& price, double& qty, std::string side) {


    //  if bid i check the level and return the quantity not filled and set a price to that level with that quantity
    if (side == "b")
    {
        auto it = ask_map.begin();
        while (it != ask_map.end())
        {
            if (it->first <= price)
            {
                it = ask_map.erase(it);
            }
            else break;
        }
    }
    else if (side == "a")
    {
        auto it = bid_map.begin();
        while (it != bid_map.end())
        {
            if (it->first >= price)
            {
                it = bid_map.erase(it);
            }
            else break;
        }
    }
}

void set_price_quantity_orderBook(std::map<double, double>& ask, std::map<double, double, std::greater<double>>& bid, const std::vector<std::string>& quotes) {

    double price = std::stod(quotes[PRICE_ORDER_BOOK_POS]);
    double quantity = std::stod(quotes[QUANTITY_ORDER_BOOK_POS]);
    std::string side = quotes[SIDE_ORDER_BOOK_POS];

    manage_trade_in_orderBook(ask, bid, price, quantity, side);
    if (quotes.size() == SIZE_OF_QUOTES) {
        if (side == "a") {
            if (quantity == 0) {
                ask.erase(price);
            }
            else {
                ask[price] = quantity;
            }

        }
        else if (side == "b") {
            if (quantity == 0) {
                bid.erase(price);
            }
            else {
                bid[price] = quantity;
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

void processFilesTR(ThreadSafeQueue& Trades_queue)
{
    std::fstream* trade = new std::fstream("C:\\Users\\Administrator\\TR_20210512.csv");
    std::string line;

    my_log.writeToLog("[THREAD 1] Starting");
    int counter = 0;
    {
        std::unique_lock<std::mutex> lk(m);
        ready1 = true;
    }
    cv.notify_all();
    my_log.writeToLog("[THREAD 1] Notified All");
    while (trade->good())
    {
        std::getline(*trade, line);
        if (trade->eof())
        {

            my_log.writeToLog("[THREAD 1] End of Trade file");
            break;
        }
        if (!line.empty())
        {
            Trades_queue.push(line);
        }
    }

    my_log.writeToLog("[THREAD 1] RED TRADES and QUEUE LENGTH");
    delete trade;
}

void processFilesOB(ThreadSafeQueue& Orderbook_queue)
{
    std::fstream* quotes = new std::fstream("C:\\Users\\Administrator\\OB_20210512.csv");
    std::string line;
    int counter = 0;
    my_log.writeToLog("[THREAD 2] Starting");
    int numberPush(0);
    {
        std::unique_lock<std::mutex> lk(m);
        ready2 = true;
    }
    cv.notify_all();
    my_log.writeToLog("[THREAD 2] Notified All");
    while (quotes->good())
    {
        std::getline(*quotes, line);
        if (quotes->eof())
        {
            my_log.writeToLog("[THREAD 2] Size");
            my_log.writeToLog("[THREAD 2] End of Order Book file and QUEUE LENGTH ");
            break;
        }
        if (!line.empty())
        {
            Orderbook_queue.push(line);
            numberPush++;
        }
    }

    my_log.writeToLog("[THREAD 2] RED ORDER BOOK");
    my_log.writeToLog("[THREAD 2] " + std::to_string(numberPush));
    delete quotes;

}


void readFromQueues(std::map<double, double>& ask, std::map<double, double, std::greater<double>>& bid,
                    ThreadSafeQueue& Orderbook_queue, ThreadSafeQueue& Trades_queue,
                    VPIN& _vpin, std::vector<std::vector< double>>& vpin_results)
{

    std::vector<std::string>  quotes, executed;
    
    std::unique_lock<std::mutex> lk(m);
    cv.wait(lk, [] {return ready1 && ready2; });
    
    my_log.writeToLog("[THREAD 3] Starting ");
    int numDone(0);
    bool updated = false;
    std::this_thread::sleep_for(std::chrono::seconds(5));
    while (Orderbook_queue.is_empty() == false || Trades_queue.is_empty() == false )
    {
        quotes = (Orderbook_queue.size() != 0) ? splitLine(Orderbook_queue.front(), ',') : splitLine("", ',');
        executed = (Trades_queue.size() != 0) ? splitLine(Trades_queue.front(), ',') : splitLine("", ',');

        if (quotes.size() != SIZE_OF_QUOTES) // Mmeaning there is some problem in the quotes (maybe finished file?)
        {
            if (executed.size() == SIZE_OF_TRADES) // but i have an executed
            {
                updated = _vpin.update(executed);
                if (updated)
                {
                    if (updated) vpin_results.push_back(_vpin.get_both_cdf_vpin());
                }
                Trades_queue.pop();
            }
            else {
                my_log.writeToLog("[THREAD 3] Continue [225]");
                break;
            }
        }
        else
        {
            if (executed.size() != SIZE_OF_TRADES) // but maybe i don't have trade 
            {
                set_price_quantity_orderBook(ask, bid, quotes);
                Orderbook_queue.pop();
            }
            else 
            {  
                if (std::stoll(executed[TIMESTAMP_TRADE_POS]) <= std::stoll(quotes[TIMESTAMP_ORDER_BOOK_POS]))
                {
                    updated = _vpin.update(executed);
                    if(updated) vpin_results.push_back(_vpin.get_both_cdf_vpin());
                    Trades_queue.pop();
                }
                else
                {
                    set_price_quantity_orderBook(ask, bid, quotes);
                    Orderbook_queue.pop();
                }
            }
        }
    }
}




int main()
{
    VPIN _vpin(100, 100 , 0); 
    std::unique_ptr <std::vector<std::vector<double>>> vpin_results = std::make_unique<std::vector<std::vector<double>>>();
    std::unique_ptr<ThreadSafeQueue> Orderbook_queue = std::make_unique<ThreadSafeQueue>();
    std::unique_ptr<ThreadSafeQueue> Trades_queue = std::make_unique<ThreadSafeQueue>();
    std::map<double, double>* ask = new std::map<double, double>;
    std::map<double, double, std::greater<double>>* bid = new std::map<double, double, std::greater<double>>;


    my_log.writeToLog("Start threads");
    std::thread processThreadOB(processFilesOB, std::ref(*Orderbook_queue));
    std::thread processThreadTR(processFilesTR, std::ref(*Trades_queue));
    std::thread readThread(readFromQueues,std::ref(*ask),std::ref(*bid),
                           std::ref(*Orderbook_queue),std::ref(*Trades_queue), 
                            std::ref(_vpin), std::ref(*vpin_results));
    my_log.writeToLog("Joining Threads");


    processThreadOB.join();
    processThreadTR.join();
    readThread.join();
    my_log.writeToLog("Finished the thread");
    

    
    my_log.writeToLog(std::to_string(Orderbook_queue->size()));
    my_log.writeToLog(std::to_string(Trades_queue->size()));

    auto ask_it = ask->begin();
    auto bid_it = bid->begin();
    auto vpin_it = vpin_results->begin();
    int how = 1;
    /*
    while (ask_it != ask->end() && bid_it != bid->end()) {

        std::cout << how << ")" << std::fixed << std::setprecision(4) << bid_it->second << "  " << bid_it->first << " | ";
        std::cout << std::fixed << std::setprecision(8) << ask_it->first<< " " << ask_it ->second<< std::endl;
        bid_it++;
        ask_it++;
        how++;
    }*/
    std::cout << " Printing vpin " << std::endl;
    int c = 0;
    /*while (vpin_it != vpin_results->end())
    {
        std::cout << c++;
        for (auto& elem : *vpin_it)
        {
            std::cout  << " " << elem << " ";
        }
        std::cout << std::endl;
        vpin_it++;
    }*/
    std::ofstream outputFile("cdf.csv");
    std::cout << _vpin.get_cdfs().size() << std::endl;
    for (auto& element :_vpin.get_cdfs()) {
        outputFile << element;
        outputFile << ",";
    }
    // Close the output file
    outputFile.close();
    return 0;



}
