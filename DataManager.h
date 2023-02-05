#ifndef DataManager_h
#define DataManager_h
#include <fstream>
#include <string>
#include <iostream>
#include <vector>
#include <map>
#include <thread>
#include <chrono> 
#include "OrderBook.h"

class DataManager
{

    private:

        DataManager();

        enum PositionsData
        {
            SYMBOL = 0, TIMESTAMP = 1, ORDER_ID_START = 2, ORDER_ID_END = 3, SIDE = 4, UPDATE_TYPE = 5, PRICE = 6, QTY = 7, PU = 8
        };
        

    public:
        //  Initialized as static because is one less operation to call a static function in assembly code compared
        //  to call a function of an object. (more fast)
        static void split_csv_order_book(std::string& line_in, std::vector<std::string>& line_out, char to_split);


        static void read_order_book(std::string fileName,std::map<char, std::map<double, double>>& order_book);

};

#endif