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
#include <memory>

#define SIZE_OF_QUOTES 9
#define SIZE_OF_TRADES 6
#define TIMESTAMP_TRADE_POS 4
#define TIMESTAMP_ORDER_BOOK_POS 1
#define SIDE_ORDER_BOOK_POS 4
#define PRICE_ORDER_BOOK_POS 6
#define QUANTITY_ORDER_BOOK_POS 7
Logger my_log;


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
    std::fstream* trade = new std::fstream("/home/alessio/Desktop/prove/Data/trade_data.csv");
    std::string a;

    my_log.writeToLog("[THREAD 1] Starting");
    int counter = 0;
    while (trade->good())//&& counter++ < 12000000)
    {
        std::getline(*trade, a);
        if (trade->eof())
        {

            my_log.writeToLog("[THREAD 1] End of Trade file");
            break;
        }


        if (!a.empty())
        {
            Trades_queue.push(a);
        }

    }

    my_log.writeToLog("[THREAD 1] RED TRADES and QUEUE LENGTH");
    delete trade;
}

void processFilesOB(ThreadSafeQueue& Orderbook_queue)
{
    std::fstream* quotes = new std::fstream("/home/alessio/Desktop/prove/Data/orderbook2.csv");
    std::string n;
    int counter = 0;
    my_log.writeToLog("[THREAD 2] Starting");
    int numberPush(0);
    while (quotes->good())// && counter++<12000000) 
    {
        std::getline(*quotes, n);

        if (quotes->eof())
        {
            my_log.writeToLog("[THREAD 2] Size");
            my_log.writeToLog("[THREAD 2] End of Order Book file and QUEUE LENGTH ");
            break;
        }


        if (!n.empty())
        {
            Orderbook_queue.push(n);
            numberPush++;
        }

    }

    my_log.writeToLog("[THREAD 2] RED ORDER BOOK");
    my_log.writeToLog("[THREAD 2] " + std::to_string(numberPush));
    delete quotes;

}

void readFromQueues(std::map<double, double>& ask, std::map<double, double, std::greater<double>>& bid, ThreadSafeQueue& Orderbook_queue, ThreadSafeQueue& Trades_queue)
{

    std::vector<std::string>  quotes, executed;
    my_log.writeToLog("[THREAD 3] Starting ");
    std::this_thread::sleep_for(std::chrono::seconds(20));
    int numDone(0);
    while (Orderbook_queue.is_empty() == false || Trades_queue.is_empty() == false)
    {



        //my_log.writeToLog("[THREAD 3] Continue [210]");
        quotes = (Orderbook_queue.size() != 0) ? splitLine(Orderbook_queue.front(), ',') : splitLine("", ',');
        executed = (Trades_queue.size() != 0) ? splitLine(Trades_queue.front(), ',') : splitLine("", ',');


        // HERE I HAVE TWO QUEUE FROM BEFORE WAITING TO BE RED, I CHECK FOR PROBLEMS IN THE DATA, IF THERE ARE PROBLEMS I POP THE DATA WITHOUT DOING NOTHING
        // IF EVERYTHING GOES SMOOTHLY I INSERT ALL MY DATA IN A VECTOR CALLED TEMP WHICH I WILL USE LATER I SUPPOSE. BUT I THINK IT WILL NOT BE NECESSARY 

        if (quotes.size() != SIZE_OF_QUOTES) // Mmeaning there is some problem in the quotes (maybe finished file?)
        {
            if (executed.size() == SIZE_OF_TRADES) // but i have an executed
            {

                /////////manage_trade_in_orderBook(ask,bid,executed);
                //my_log.writeToLog("[THREAD 3] pop trade" + std::to_string(Trades_queue.size()));
                Trades_queue.pop();
            }
            else {
                my_log.writeToLog("[THREAD 3] Continue [225]");
            }
        }
        else
        {
            if (executed.size() != SIZE_OF_TRADES) // but maybe i don't have trade 
            {

                set_price_quantity_orderBook(ask, bid, quotes);
                //printBook(ask, bid);
                numDone++;
                Orderbook_queue.pop();
            }
            else {  // or maybe i have trades
            // so i check the timestamp if i have both 
                //IF BOTH GOOD CHECK TIMESTAMP
                
                if (std::stoll(executed[TIMESTAMP_TRADE_POS]) <= std::stoll(quotes[TIMESTAMP_ORDER_BOOK_POS]))
                {
                    // I CHECK THE ORDERBOOK LEVELS AND SEE IF I CAN EXECUTE 

                    //////////manage_trade_in_orderBook(ask,bid,executed);
                    //my_log.writeToLog("[THREAD 3] pop trade " + std::to_string(Trades_queue.size()));
                    Trades_queue.pop();
                }
                else
                {

                    set_price_quantity_orderBook(ask, bid, quotes);
                    //my_log.writeToLog("[THREAD 3] [253] pop quotes " + std::to_string(Orderbook_queue.size()));
                    //printBook(ask, bid);
                    Orderbook_queue.pop();
                    numDone++;
                }
            }
        }
        //my_log.writeToLog("[THREAD 3] " + std::to_string(Orderbook_queue.size()) + " " + std::to_string(Trades_queue.size()));

    }
    my_log.writeToLog("[THREAD 3] DONE: " + std::to_string(numDone));


}




int main()
{

    std::unique_ptr<ThreadSafeQueue> Orderbook_queue = std::make_unique<ThreadSafeQueue>();
    std::unique_ptr<ThreadSafeQueue> Trades_queue = std::make_unique<ThreadSafeQueue>();
    std::map<double, double>* ask = new std::map<double, double>;
    std::map<double, double, std::greater<double>>* bid = new std::map<double, double, std::greater<double>>;


    my_log.writeToLog("Start threads");
    std::thread processThreadOB(processFilesOB, std::ref(*Orderbook_queue));
    std::thread processThreadTR(processFilesTR, std::ref(*Trades_queue));
    std::thread readThread(readFromQueues,std::ref(*ask),std::ref(*bid),std::ref(*Orderbook_queue),std::ref(*Trades_queue));
    my_log.writeToLog("Joining Threads");


    processThreadOB.join();
    processThreadTR.join();
    readThread.join();
    my_log.writeToLog("Finished the thread");
    my_log.writeToLog(std::to_string(Orderbook_queue->size()));
    my_log.writeToLog(std::to_string(Trades_queue->size()));

    auto ask_it = ask->begin();
    auto bid_it = bid->begin();
    int how = 1;

    while (ask_it != ask->end() && bid_it != bid->end()) {

        std::cout << how << ")" << std::fixed << std::setprecision(4) << bid_it->second << "  " << bid_it->first << " | ";
        std::cout << std::fixed << std::setprecision(4) << ask_it->first<< " " << ask_it ->second<< std::endl;
        bid_it++;
        ask_it++;
        how++;
    }

    std::cout << how << std::endl;
    return 0;



}
