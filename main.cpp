#include <iostream>
#include <fstream>
#include <string>
#include <thread>
#include <atomic>
#include <chrono>
#include <vector>
#include <mutex>
#include <algorithm>


std::mutex _mtx;
std::vector<int> timestamps;

void add_to_timestamps(int timestamp)
{
    std::unique_lock<std::mutex> lock(_mtx);
    timestamps.push_back(timestamp);
};

class TimeStampThreadSafe {
private:
    int value;
    std::mutex mtx;

public:
    TimeStampThreadSafe() : value(0) {}

    int get() {
        std::unique_lock<std::mutex> lock(mtx);
        return value;
    }

    void set(int val) {
        std::unique_lock<std::mutex> lock(mtx);
        value = val;
    }
};


void split_csv(
	std::string& line_in,
    std::vector<std::string>& line_out,
	char tosplit
)
{
    std::mutex mtx;
    std::unique_lock<std::mutex> lock(mtx);
	std::string topush;

	for(int i= 0; i< line_in.size();++i)
	{
		if (line_in[i] != tosplit)
		{
			topush.push_back(line_in[i]);
		}
		else
		{
			
            line_out.push_back(topush);
            topush.clear();
		}
		if (i == line_in.size() - 1 && topush != "")
		{
            line_out.push_back(topush);
            topush.clear();
		}
	}
};




void read_file_OrderBook(TimeStampThreadSafe& ts_ob, TimeStampThreadSafe& ts_tr)
{
    std::ifstream fileA("/home/alessio/Desktop/prove/Data/orderbook.csv");
    std::string line;
    long timestamp, start, end, up_type, price, qty, pu;
    std::string symbol, update_type;
    char side;
    std::vector<std::string> line_out;
    int i = 0;
    while (std::getline(fileA, line))
    {       
        std::cout << "inside" << std::endl;
        split_csv(line,line_out,',');
        timestamp = std::stol(line_out[1]);
        add_to_timestamps(timestamp);
        ts_ob.set(timestamp);
        long value = ts_tr.get();
        std::cout << "Trade "  << std::endl;
        /*while (timestamp > value)
        {

        }*/
        
    }
    fileA.close();
}

void read_file_Trades(TimeStampThreadSafe& ts_ob, TimeStampThreadSafe& ts_tr)
{

    
    std::ifstream fileB("/home/alessio/Desktop/prove/Data/trade_data.csv");
    std::string line;
    long id, timestamp;
    double price, quantity, nominal;
    std::string isMatcher;
    std::vector<std::string> line_out;
    while (std::getline(fileB, line))
    {

        split_csv(line,line_out,',');

        timestamp = std::stol(line_out[4]);
        add_to_timestamps(timestamp);
        ts_tr.set(timestamp);
        long value = ts_ob.get();
        std::cout << "OR B " << value << std::endl;
        /*while (timestamp >= value)
        {
        }*/
    }
    fileB.close();
}

int main()
{
    TimeStampThreadSafe ts_ob; 
    TimeStampThreadSafe ts_tr;
    std::thread thread_ob(read_file_OrderBook,ts_ob, ts_tr);
    std::thread thread_trades(read_file_Trades,ts_ob, ts_tr);

    thread_ob.join();
    thread_trades.join();

    long value = -1;
    int n = 0 ;
    for(long i = 0;i< timestamps.size(); i++)
    {
        long temp = timestamps[i];
            if (temp < value) 
            {
                n++;
            }
        value = temp;
    }
    std::cout << n << "\n";
    std::cout << timestamps.size();
    return 0;
}