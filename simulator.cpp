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
	ulong blockSize = 512;
	L1Exclusive L1 = L1Exclusive(config.L1CacheSize / blockSize, blockSize);
	L2Exclusive L2 = L2Exclusive(config.L2CacheSize / blockSize, blockSize);
	DeadBlockLRUCache L3 = DeadBlockLRUCache(config.L3CacheSize / blockSize, blockSize);
	Memory memory = Memory();
	L1.setUpperCache(&L2);
	L2.setUpperCache(&L3);
	L2.setLowerCache(&L1);
	L3.setUpperCache(&memory);
	L3.setLowerCache(&L2);
	
	cout << blockSize << endl;
	cout << config.L1CacheSize << endl;

	while(cin >> dec >> cmd >> hex >> address)
	{
		ulong lineAddress = (address / blockSize) * blockSize;
		//if(cmd != 2) cout << cmd << " " << lineAddress << endl;
		switch(cmd)
		{
			case 0: L1.read(lineAddress); break;
			case 1: L1.write(lineAddress); break;
		}
	}
	
	cout << "L1 " << L1 << endl;
	cout << "L2 " << L2 << endl;
	cout << "L3 " << L3 << endl;
	cout << "Memory " << memory << endl;
}