#include "DataManager.h"
// CONSTANTS
#define ASK 'a'
#define BID 'b'
int main() {

    std::map<char, std::map<double, double>> order_book;
    std::string path = "/home/alessio/Desktop/prove/Data/BTCUSDT_T_DEPTH_2021-10-30_depth_snap.csv";
    //read_file(path,order_book);
   	DataManager::read_order_book(path,order_book);
    int min{100000}, max{0};
    for (const auto &[key, value] : order_book[ASK]) 
    {
        std::cout << key << ": " << value << std::endl;
        if (key < min) min= key;
        if (key > max) max= key;
    }

    std::cout << "min: " << min << "\nmax: " << max;
    return 0;
}



// FUNCTIONS 



