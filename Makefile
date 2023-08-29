# LDLIBS = -L/usr/lib/
# LDHASHER = -L./smhaher
LDLIBS =
LDHASHER =

CXX=g++
CXXFLAGS=-std=c++17 -Wall -Wextra
LDFLAGS=-lstdc++fs -pthread 

all: deduplicate clean

deduplicate: main.o Hasher.o text.o MurmurHash3.o simdjson.o ThreadPool.o dedup.o
	$(CXX) $(CXXFLAGS) -o deduplicate main.o Hasher.o text.o MurmurHash3.o simdjson.o ThreadPool.o dedup.o $(LDFLAGS) $(LDLIBS) $(LDHASHER)

main.o: main.cpp
	$(CXX) $(CXXFLAGS) -c main.cpp

Hasher.o: Hasher.cpp Hasher.hpp
	$(CXX) $(CXXFLAGS) -c Hasher.cpp

text.o: text.cpp text.hpp
	$(CXX) $(CXXFLAGS) -c text.cpp

dedup.o: dedup.cpp dedup.hpp
	$(CXX) $(CXXFLAGS) -c dedup.cpp

MurmurHash3.o:
	$(CXX) -c smhasher/src/MurmurHash3.cpp

simdjson.o: simdjson.cpp simdjson.h
	$(CXX) -c ./simdjson.cpp

ThreadPool.o: ThreadPool.cpp ThreadPool.hpp
	$(CXX) -c ./ThreadPool.cpp

clean:
	rm -f *.o

clean_debug:
	rm ./output/*
	rm ./blacklists/*


test: Hasher.o text.o MurmurHash3.o dedup.o
	g++ -std=c++11 HasherTest.cpp -lgtest -lgtest_main -pthread -o test Hasher.o text.o MurmurHash3.o dedup.o
