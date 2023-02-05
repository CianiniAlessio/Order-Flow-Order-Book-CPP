CXX = g++
CXXFLAGS = -Wall -Wextra -Wpedantic

main:  OrderBook.o DataManager.o main.o 
	$(CXX) $(CXXFLAGS) DataManager.o OrderBook.o main.o -o main

main.o: main.cpp
	$(CXX) $(CXXFLAGS) -c main.cpp

OrderBook.o: OrderBook.cpp OrderBook.h
	$(CXX) $(CXXFLAGS) -c OrderBook.cpp

DataManager.o: DataManager.cpp DataManager.h
	$(CXX) $(CXXFLAGS) -c DataManager.cpp


.PHONY: clean

clean:
	rm -f main main.o main.s main.ii DataManager.o DataManager.s
	
