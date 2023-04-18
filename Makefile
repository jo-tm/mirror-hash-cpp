CXX = g++
CXXFLAGS = -std=c++11 -Wall -O2

all: mirror256

mirror256: main.o mirror256.o
	$(CXX) $(CXXFLAGS) -o mirror256 main.o mirror256.o

main.o: main.cpp mirror256.h
	$(CXX) $(CXXFLAGS) -c main.cpp

mirror256.o: mirror256.cpp mirror256.h
	$(CXX) $(CXXFLAGS) -c mirror256.cpp

clean:
	rm -f *.o mirror256
