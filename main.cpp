#include <iostream>
#include <fstream>
#include <string>
#include <mutex>
#include <condition_variable>
#include <thread>
#include <vector>
#include <chrono>

std::mutex mtx;
std::condition_variable cv;
long long timestamp1, timestamp2;
std::vector<long long> timestamps;
bool finished1 = false, finished2 = false;


std::vector<std::string> split(const std::string &s, char delimiter) {
  std::vector<std::string> tokens;
  std::string token;
  for (char c : s) {
    if (c == delimiter) {
      tokens.push_back(token);
      token.clear();
    } else {
      token += c;
    }
  }
  tokens.push_back(token);
  return tokens;
}

void thread1() {
  std::ifstream file1("/home/alessio/Desktop/prove/Data/orderbook.csv");
  std::string line;
  while (std::getline(file1, line)) {
    std::vector<std::string> values = split(line, ',');
    long long current_timestamp = std::stoll(values[2]);
    {
      std::unique_lock<std::mutex> lock(mtx);
      while (timestamp2 >= current_timestamp) {
        cv.wait(lock);
      }
      timestamp1 = current_timestamp;
    }
    cv.notify_one();
  }
  {
    std::unique_lock<std::mutex> lock(mtx);
    finished1 = true;
  }
  cv.notify_one();
}

void thread2() {
  std::ifstream file2("/home/alessio/Desktop/prove/Data/trade_data.csv");
  std::string line;
  while (std::getline(file2, line)) {
    std::vector<std::string> values = split(line, ',');
    long long current_timestamp = std::stoll(values[4]);
    {
      std::unique_lock<std::mutex> lock(mtx);
      while (timestamp1 >= current_timestamp) {
        cv.wait(lock);
      }
      timestamp2 = current_timestamp;
    }
    cv.notify_one();
  }
  {
    std::unique_lock<std::mutex> lock(mtx);
    finished2 = true;
  }
  cv.notify_one();
}

void thread3() {
  while (!finished1 || !finished2) {
    std::unique_lock<std::mutex> lock(mtx);
    cv.wait(lock);
    if (finished1 && finished2) {
      break;
    }
    if (timestamp1 >= timestamp2) {
      timestamps.push_back(timestamp1);
      std::cout << timestamp1 <<"\n" << std::flush;
    } else {
      timestamps.push_back(timestamp2);
        std::cout << timestamp2 <<"\n" << std::flush;
    }
  }
}
#include <algorithm>


int main() {
  std::thread t1(thread1);
  std::thread t2(thread2);
  std::thread t3(thread3);

  t1.join();
  t2.join();
  t3.join();

  std::cout << std::is_sorted(timestamps.begin(),timestamps.end());



}
/*#include <vector>
#include <iostream>
#include <fstream>
#include <string>
#include <thread>
#include <mutex>
#include <condition_variable>

long timeStamp1 = 0;
long timeStamp2 = 0;
bool frozen1 = false;
bool frozen2 = false;
std::mutex mtx;
std::condition_variable cv1;
std::condition_variable cv2;

void thread1() {
    while (true) {
        std::unique_lock<std::mutex> lock(mtx);
        if (!frozen1) {
            // read timestamp from file1.csv and update timeStamp1
            // code to read and update timeStamp1 ...
        } else {
            // freeze the updating of timeStamp1
            cv1.wait(lock, [] { return timeStamp1 >= timeStamp2; });
            frozen1 = false;
        }
    }
}

void thread2() {
    while (true) {
        std::unique_lock<std::mutex> lock(mtx);
        if (!frozen2) {
            // read timestamp from file2.csv and update timeStamp2
            // code to read and update timeStamp2 ...
        } else {
            // freeze the updating of timeStamp2
            cv2.wait(lock, [] { return timeStamp2 >= timeStamp1; });
            frozen2 = false;
        }
    }
}

void thread3() {
    while (true) {
        std::unique_lock<std::mutex> lock(mtx);
        if (timeStamp1 < timeStamp2) {
            frozen1 = true;
            cv1.notify_one();
        }
        if (timeStamp2 < timeStamp1) {
            frozen2 = true;
            cv2.notify_one();
        }
    }
}

int main() {
    std::thread t1(thread1);
    std::thread t2(thread2);
    std::thread t3(thread3);

    t1.join();
    t2.join();
    t3.join();

    return 0;
}






/*
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
        split_csv(line,line_out,',');
        timestamp = std::stol(line_out[1]);
        add_to_timestamps(timestamp);
        ts_ob.set(timestamp);
        long valuetr = ts_tr.get();
        long valueob = ts_ob.get();
        std::cout << "ORDER BOOK" <<"\n" << std::flush;
        std::cout << timestamp<<"\n" << std::flush;
        std::cout << valuetr <<"\n" << std::flush;
        std::cout << valueob <<"\n" << std::flush;

        while (timestamp > valuetr)
        {

        }
        
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
        
        long valuetr = ts_tr.get();
        long valueob= ts_ob.get();
        std::cout << "TRADE\n" << std::flush;
        std::cout << timestamp<<"\n" << std::flush;
        std::cout << valuetr <<"\n" << std::flush;
        std::cout << valueob <<"\n" << std::flush;
        while (timestamp >= value)
        {
        }
    }
    fileB.close();
}

int main() {
  std::thread t1(thread1);
  std::thread t2(thread2);
  std::thread t3(thread3);

  t1.join();
  t2.join();
  t3.join();

  return 0;
}*/