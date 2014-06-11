CacheSimulator
==============
This program is a cache simulator which simulates 3 levels of caches on a single processor, using an inheritable Cache class. It is easily extensible to plug different caching algorithms or different block sizes.
This program only simulates data cache access. Instruction fetches are typically implemented as a seperate cache and don't vary much.

To build the program, simply type "make" in the terminal.

To run the program, use: simulator (configFile) < (inputFile)
A default configuration file is supplied in config.txt. Sample input files are provided in the test folder

Simulator.cpp contains all of the class hierarchies. It is easily extensible to add new cache configurations. 
It can even be expanded to support new types of cache, though they will have to be added to the program as another source file. As long as the cache inherits from our base Cache class, it will be supported by the simulation.

The config file takes a blocksize, and a cache size for all 3 levels of cache. Note that for an exclusive cache, L1 size must be the same as L2 size.