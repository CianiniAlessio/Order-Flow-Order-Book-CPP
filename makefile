CXX = g++
CXXFLAGS = -Wall -Wextra -Wpedantic

main: main.o asm 
	$(CXX) $(CXXFLAGS) main.o -o main

main.o: main.cpp
	$(CXX) $(CXXFLAGS) -c main.cpp

asm: main.cpp
	$(CXX) $(CXXFLAGS) -save-temps -c main.cpp
.PHONY: clean

clean:
	rm -f main main.o main.s
