CXX = g++
CXXFLAGS = -Wall -Wextra -Wpedantic

main: main.o 
	$(CXX) $(CXXFLAGS) main.o -o main

main.o: main.cpp
	$(CXX) $(CXXFLAGS) -c main.cpp




.PHONY: clean

clean:
	rm -f main main.o  
	
