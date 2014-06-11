all:	
	g++ -Wall -Wextra -pedantic -std=c++0x -o simulator config.cpp simulator.cpp
clean:
	rm -f *.o *~ simulator core
