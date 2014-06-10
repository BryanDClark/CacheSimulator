#include "LRU.hpp"
#include "memory.hpp"
#include "Exclusive.cpp"
#include "DeadBlock.cpp"
#include <iostream>

using namespace std;

int main()
{
	int cmd;
	ulong address;
	
	Cache L1 = L1Exclusive(0,0);
	Cache L2 = L2Exclusive(0,0);
	Cache L3 = DeadBlockLRUCache(0,0);
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