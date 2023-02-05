#include "OrderBook.h"


OrderBook::OrderBook(){}

void OrderBook::SetPriceQuantity(std::map<char, std::map<double, double>>& order_book, double price, double quantity, char side)
{
    order_book[side][price] = quantity;
}