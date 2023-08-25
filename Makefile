# LDLIBS = -L/usr/lib/
# LDHASHER = -L./smhaher
LDLIBS =
LDHASHER =

CXX=g++
CXXFLAGS=-std=c++17 -Wall -Wextra -pthread
# CXXFLAGS = -O3 -std=c++17 -march=native -mtune=native
LDFLAGS=-lstdc++fs

all: deduplicate clean

deduplicate: main.o Hasher.o text.o MurmurHash3.o simdjson.o
	$(CXX) $(CXXFLAGS) -o deduplicate main.o Hasher.o text.o MurmurHash3.o simdjson.o $(LDFLAGS) $(LDLIBS) $(LDHASHER)

main.o: main.cpp
	$(CXX) $(CXXFLAGS) -c main.cpp

Hasher.o: Hasher.cpp Hasher.hpp
	$(CXX) $(CXXFLAGS) -c Hasher.cpp

text.o: text.cpp text.hpp
	$(CXX) $(CXXFLAGS) -c text.cpp

MurmurHash3.o:
	$(CXX) -c smhasher/src/MurmurHash3.cpp

simdjson.o: simdjson.cpp simdjson.h
	$(CXX) -c ./simdjson.cpp

clean:
	rm -f *.o