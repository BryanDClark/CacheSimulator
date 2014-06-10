all:	cacheCoherence.cpp 
	g++ -O3 -std=c++0x -o P2 config.cpp cacheCoherence.cpp -s 
debug:	cacheCoherence.cpp
	g++ -Wall -Wextra -pedantic -std=c++0x -g -o P2 config.cpp cacheCoherence.cpp
project:
	g++ -Wall -Wextra -pedantic -std=c++0x config.cpp simulator.cpp
deadblock:
	g++ -Wall -Wextra -pedantic -std=c++0x DeadBlock.cpp
clean:
	rm -f *.o *~ P2 core
