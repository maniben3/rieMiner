VER    = 0.93a
CXX    = g++
M4     = m4
AS     = as
SED    = sed
CFLAGS = -Wall -Wextra -std=c++17 -O3 -fno-pie -no-pie
LIBS   = -lcurl -lcrypto -lgmpxx -lgmp

all: native

native: CFLAGS += -march=native -s
native: LIBS := -pthread $(LIBS)
ifeq ($(shell uname -m),x86_64)
native: rieMinerx64
else
native: rieMiner
endif

debug: CFLAGS += -march=native -g
debug: rieMiner

main.o: main.cpp main.hpp Miner.hpp Client.hpp Stats.hpp tools.hpp
	$(CXX) $(CFLAGS) -c -o $@ -DversionShort=\"$(VER)\" -DversionString=\"rieMiner$(VER)\" main.cpp

Miner.o: Miner.cpp Miner.hpp
	$(CXX) $(CFLAGS) -c -o $@ Miner.cpp

StratumClient.o: StratumClient.cpp
	$(CXX) $(CFLAGS) -c -o $@ StratumClient.cpp

GBTClient.o: GBTClient.cpp
	$(CXX) $(CFLAGS) -c -o $@ GBTClient.cpp

Client.o: Client.cpp
	$(CXX) $(CFLAGS) -c -o $@ Client.cpp

API.o: API.cpp
	$(CXX) $(CFLAGS) -c -o $@ API.cpp

Stats.o: Stats.cpp
	$(CXX) $(CFLAGS) -c -o $@ Stats.cpp

tools.o: tools.cpp
	$(CXX) $(CFLAGS) -c -o $@ tools.cpp

fermat.o: ispc/fermat.cpp
	$(CXX) $(CFLAGS) -c -o $@ ispc/fermat.cpp -Wno-unused-function -Wno-unused-parameter -Wno-strict-overflow


	
