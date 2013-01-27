GXX = g++
DEBUG = -g
OPT = -O3

all: cache-sim

cache-sim: cache-sim.o
	$(GXX) $(DEBUG) cache-sim.o -o cache-sim

cache-sim.o: cache-sim.cpp
	$(GXX) $(DEBUG) -c cache-sim.cpp

clean:
	rm -f *.o *~ core cache-sim


