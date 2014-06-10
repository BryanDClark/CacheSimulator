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
	ulong blockSize = config.blockSize;
	L1Exclusive L1 = L1Exclusive(config.L1CacheSize / blockSize, blockSize);
	L2ExclusivePrefetch L2 = L2ExclusivePrefetch(config.L2CacheSize / blockSize, blockSize);
	DeadBlockLRUCache L3 = DeadBlockLRUCache(config.L3CacheSize / blockSize, blockSize);
	Memory memory = Memory();
	L1.setUpperCache(&L2);
	L2.setUpperCache(&L3);
	L2.setLowerCache(&L1);
	L3.setUpperCache(&memory);
	L3.setLowerCache(&L2);
	
	L1Exclusive L1n2 = L1Exclusive(config.L1CacheSize / blockSize, blockSize);
	L2Exclusive L2n2 = L2Exclusive(config.L2CacheSize / blockSize, blockSize);
	DeadBlockLRUCache L3n2 = DeadBlockLRUCache(config.L3CacheSize / blockSize, blockSize);
	Memory memoryn2 = Memory();
	L1n2.setUpperCache(&L2n2);
	L2n2.setUpperCache(&L3n2);
	L2n2.setLowerCache(&L1n2);
	L3n2.setUpperCache(&memoryn2);
	L3n2.setLowerCache(&L2n2);
		
	L1Exclusive L1n3 = L1Exclusive(config.L1CacheSize / blockSize, blockSize);
	L2Exclusive L2n3 = L2Exclusive(config.L2CacheSize / blockSize, blockSize);
	LRUCache L3n3 = LRUCache(config.L3CacheSize / blockSize, blockSize);
	Memory memoryn3 = Memory();
	L1n3.setUpperCache(&L2n3);
	L2n3.setUpperCache(&L3n3);
	L2n3.setLowerCache(&L1n3);
	L3n3.setUpperCache(&memoryn3);
	L3n3.setLowerCache(&L2n3);
	
	cout << blockSize << endl;
	cout << config.L1CacheSize << endl;

	while(cin >> dec >> cmd >> hex >> address)
	{
		ulong lineAddress = (address / blockSize) * blockSize;
		//if(cmd != 2) cout << cmd << " " << lineAddress << endl;
		switch(cmd)
		{
			case 0: L1.read(lineAddress); L1n2.read(lineAddress); L1n3.read(lineAddress); break;
			case 1: L1.write(lineAddress); L1n2.write(lineAddress); L1n3.write(lineAddress); break;
		}
	}
	
	cout << "L1 " << L1 << endl;
	cout << "L2 " << L2 << endl;
	cout << "L3 " << L3 << endl;
	cout << "Memory " << memory << endl << endl;
	
	cout << "L1n2 " << L1n2 << endl;
	cout << "L2n2 " << L2n2 << endl;
	cout << "L3n2 " << L3n2 << endl;
	cout << "Memoryn2 " << memoryn2 << endl << endl;	
	
	cout << "L1n3 " << L1n3 << endl;
	cout << "L2n3 " << L2n3 << endl;
	cout << "L3n3 " << L3n3 << endl;
	cout << "Memoryn3 " << memoryn3 << endl << endl;
}