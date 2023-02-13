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

#define SIZE_OF_QUOTES 9
#define SIZE_OF_TRADES 6
#define TIMESTAMP_TRADE_POS 4
#define TIMESTAMP_ORDER_BOOK_POS 1
#define SIDE_ORDER_BOOK_POS 4
#define PRICE_ORDER_BOOK_POS 6
#define QUANTITY_ORDER_BOOK_POS 7
 // no sense to retrieve all the orderbook everytime
std::mutex queueMutexOB,queueMutexTR;

// to implement this part 
// when both file are finished i stop the cycle, right now when the queue is finished i stop the cycle which is wrong
bool order_book_file_red{false}, trades_file_red{false};

void process_order(const std::string& side, double& price, std::map<double, double>& ask, std::map<double, double, std::greater<double>>& bid)
{
    if (side == "b") {
        
        auto it = ask.begin();
        while (it != ask.end() && it->first <= price) {
            it = ask.erase(it);
        }
    }
    else {
        
        auto it = bid.begin();
        while (it != bid.end() && it->first >= price) {
            it = bid.erase(it);
        }
    }
}

// i pass the set of the level as if it was an order and return the quantity not filled and put it in the book
double manage_trade_in_orderBook(std::map<double, double>& ask_map, std::map<double, double, std::greater<double>>& bid_map, double& price, double& qty, std::string side) {

    
    //  if bid i check the level and return the quantity not filled and set a price to that level with that quantity
    if (side == "b") { 
        auto it = ask_map.begin();
        while (it != ask_map.end() && qty > 0) {
            if (it->first <= price) {
                if (qty >= it->second) {
                    qty -= it->second;
                    it = ask_map.erase(it);
                } else {
                    ask_map[it->first] -= qty;
                    qty = 0;
                }
                if (it == ask_map.end()) {
                    break;
                }
                else ++it;
            } else {
                break;
            }
        }
    } 
    else { 
        auto it = bid_map.begin();
        while (it != bid_map.end() && qty > 0) {
            if (it->first >= price) {
                if (qty >= it->second) {
                    qty -= it->second;
                    it = bid_map.erase(it);
                } else {
                    bid_map[it->first] -= qty;
                    qty = 0;
                }
                if (it == bid_map.end()) {
                    break;
                }
                else ++it;
            } else {
                break;
            }
        }
    }
    return qty;
}

void set_price_quantity_orderBook(std::map<double, double>& ask, std::map<double, double, std::greater<double>>& bid, const std::vector<std::string>& quotes) {
    
    double price = std::stod(quotes[PRICE_ORDER_BOOK_POS]);
    double quantity = std::stod(quotes[QUANTITY_ORDER_BOOK_POS]);
    //process_order(quotes[SIDE_ORDER_BOOK_POS],price,ask,bid);
    std::string side = quotes[SIDE_ORDER_BOOK_POS];

    
    quantity = manage_trade_in_orderBook(ask,bid,price,quantity,side);
    if(quotes.size() == SIZE_OF_QUOTES) {
        if(side == "a") {
            if(quantity == 0) {
                ask.erase(price);
            }
            else {
                ask[price] = quantity;
            }
            
        }
        else {
            if(quantity == 0) {
                bid.erase(price);
            }
            else {
                bid[price] = quantity;
            }
        }        
    }
    
}





// i suppose useless 


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

void processFilesTR(std::queue<std::string>& Trades_queue)
{
    std::fstream* trade = new std::fstream("/home/alessio/Desktop/prove/Data/trade_data.csv");
    std::string a;

    while (trade->good()) // need to move std::getline(*trade, a); here so i don't count the last empty line ( to do ) 
    {
        std::getline(*trade, a);

        if (trade->eof())
        {
            break;
        }

        
        std::unique_lock<std::mutex> lock(queueMutexTR);
        if (!a.empty())
        {
            Trades_queue.push(a);
        }
        
    }
    // to implement
    trades_file_red = true;
}

void processFilesOB(std::queue<std::string>& Orderbook_queue)
{
    std::fstream* quotes = new std::fstream("/home/alessio/Desktop/prove/Data/orderbook2.csv");
    std::string n;

    while (quotes->good()) // need to move std::getline(*qutoes, n); here so i don't count the last empty line ( to do )
    {
        std::getline(*quotes, n);

        if (quotes->eof() )
        {
            break;
        }

        
        std::unique_lock<std::mutex> lock(queueMutexOB);
        if (!n.empty())
        {
            Orderbook_queue.push(n);
        }
        
    }
    // to implement
    order_book_file_red = true;
}

void readFromQueues(std::map<double, double>& ask, std::map<double, double, std::greater<double>>& bid,std::queue<std::string>& Orderbook_queue, std::queue<std::string>& Trades_queue)
{
    
    std::vector<std::string>  quotes, executed;
    while (true)
    {
        
    
        std::unique_lock<std::mutex> lockOB(queueMutexOB, std::try_to_lock);
        std::unique_lock<std::mutex> lockTR(queueMutexTR, std::try_to_lock);
        
        if (!lockOB.owns_lock() || !lockTR.owns_lock())
        {
            //std::this_thread::sleep_for(std::chrono::milliseconds(100));
            continue;
        }
        

        
        if(order_book_file_red || trades_file_red) 
        {
            if (Orderbook_queue.empty() || Trades_queue.empty())
            {
                break;
            }
        }
        
        
        

       // and then i remove these 2 if since if we arrived here that means the queue is not empty
        if (!Orderbook_queue.empty())
        {
            quotes = splitLine(Orderbook_queue.front(),',');                
        }    
        else{
            quotes = splitLine("",',');
        }   
        if (!Trades_queue.empty())
        {
            executed = splitLine(Trades_queue.front(),','); // change order book thanks to this
        }
        else{
            executed = splitLine("",',');
        }



        // HERE I HAVE TWO QUEUE FROM BEFORE WAITING TO BE RED, I CHECK FOR PROBLEMS IN THE DATA, IF THERE ARE PROBLEMS I POP THE DATA WITHOUT DOING NOTHING
        // IF EVERYTHING GOES SMOOTHLY I INSERT ALL MY DATA IN A VECTOR CALLED TEMP WHICH I WILL USE LATER I SUPPOSE. BUT I THINK IT WILL NOT BE NECESSARY 

        if (quotes.size()!= SIZE_OF_QUOTES) // Mmeaning there is some problem in the quotes (maybe finished file?)
        {
            if(executed.size() == SIZE_OF_TRADES) // but i have an executed
            {
                    
                    /////////manage_trade_in_orderBook(ask,bid,executed);
                    Trades_queue.pop();
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
                    Orderbook_queue.pop();
            }
            else{  // or maybe i have trades
            // so i check the timestamp if i have both 
                //IF BOTH GOOD CHECK TIMESTAMP
                  if(std::stol(executed[TIMESTAMP_TRADE_POS]) <= std::stol(quotes[TIMESTAMP_ORDER_BOOK_POS]))
                  {
                    // I CHECK THE ORDERBOOK LEVELS AND SEE IF I CAN EXECUTE 
                  
                    //////////manage_trade_in_orderBook(ask,bid,executed);
                    Trades_queue.pop();
                  }
                  else
                  {
                  
                    set_price_quantity_orderBook(ask,bid,quotes);
                    Orderbook_queue.pop();
                  }
            }
        }
        
        
    }
}





int main()
{

    auto Orderbook_queue = std::make_unique<std::queue<std::string>>();
    auto Trades_queue = std::make_unique<std::queue<std::string>>();    
    auto ask =  std::make_unique<std::map<double, double>>();  
    auto bid =  std::make_unique<std::map<double, double,std::greater<double>>>();  
    
    
    std::thread processThreadOB(processFilesOB,std::ref(*Orderbook_queue));
    std::thread processThreadTR(processFilesTR,std::ref(*Trades_queue));
    std::thread readThread(readFromQueues,std::ref(*ask),std::ref(*bid),std::ref(*Orderbook_queue),std::ref(*Trades_queue));
    
    processThreadOB.join();
    processThreadTR.join();
    
    //ls
    //std::this_thread::sleep_for(std::chrono::seconds(10));
    readThread.join();
    
    auto it2 = (*ask).begin();
    auto it1 = (*bid).begin();
    int count = 0;
    while (it1 != (*ask).end() && it2 != (*bid).end()  && count < 50) {
        std::cout  << std::fixed <<std::setprecision(2) << "BID " << it1->second << ", " << it1->first << " | ";

        std::cout << std::fixed << std::setprecision(2) << it2->first << " , " << it2->second << " ASK" << std::endl;
        count++;
        ++it1;
        ++it2;
        
    }   
    std::cout << "alessio" << std::endl;

   
    return 0;
}
