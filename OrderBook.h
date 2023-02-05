#ifndef ORDERBOOK_h
#define ORDERBOOK_h
#include <fstream>
#include <string>
#include <iostream>
#include <vector>
#include <map>
#include <thread>
#include <chrono> 

class OrderBook
{

    private:

        OrderBook();

        enum OBIndexes
        {
            TIMESTAMP = 0, SIDE, PRICE, QTY
        };
        
    public:
        static void SetPriceQuantity(std::map<char, std::map<double, double>>& order_book, double price, double quantity, char side);

};

#endif