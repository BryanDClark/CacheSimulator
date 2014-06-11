CacheSimulator
==============
To build the program, simply type "make" in the terminal.

To run the program, use: simulator (configFile) < (inputFile)
A default configuration file is supplied in config.txt. Sample input files are provided in the test folder

Simulator.cpp contains all of the class hierarchies. It is easily extensible to add new cache configurations. 
It can even be expanded to support new types of cache, though they will have to be added to the program as another source file. As long as the cache inherits from our base Cache class, it will be supported by the simulation.

