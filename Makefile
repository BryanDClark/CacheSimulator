all:	cacheCoherence.cpp 
	g++ -O3 -std=c++0x -o P2 cacheCoherence.cpp -s 
debug:	cacheCoherence.cpp
	g++ -Wall -Wextra -pedantic -std=c++0x -g -o P2 cacheCoherence.cpp
clean:
	rm -f *.o *~ P2 core
