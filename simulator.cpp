#include "LRU.hpp"
#include "memory.hpp"
#include "Exclusive.cpp"
#include "DeadBlock.cpp"
#include <iostream>
#include "config.hpp"

using namespace std;

int main(int argc,char *argv[])
{
	int cmd;
	ulong address;

	//initialize the simulator
	Config config;
	config.initialize(argc, argv);
	
	Cache L1 = L1Exclusive(config.L1CacheSize, config.cacheLineSize);
	Cache L2 = L2Exclusive(config.L2CacheSize, config.cacheLineSize);
	Cache L3 = DeadBlockLRUCache(config.L3CacheSize, config.cacheLineSize);
	Cache memory = Memory();
	L1.setUpperCache(&L2);
	L2.setUpperCache(&L3);
	L2.setLowerCache(&L1);
	L3.setUpperCache(&memory);
	L3.setLowerCache(&L2);
	
	while(cin >> dec >> cmd >> hex >> address)
	{
		switch(cmd)
		{
			case 0: L1.read(address);
			case 1: L1.write(address);
		}
	}
}