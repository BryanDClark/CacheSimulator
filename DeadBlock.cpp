#include "LRU.hpp"
#include <map>

using namespace std;

class DeadBlockLRUCache : public LRUCache
{
public:
	static const unsigned int MaxMiss = 3;
	std::map<ulong, ulong> accessCounter;
	DeadBlockLRUCache(ulong cacheSize, ulong blockSize) : LRUCache(cacheSize, blockSize) {}

	void onMiss(ulong address, cacheState state)
	{
		if (accessCounter.find(address) != accessCounter.end())
		{
			accessCounter[address]++;
			if(accessCounter[address] == 4)
				return; //blocks is now dead
		}
		else
			accessCounter[address] = 1;
			
		LRUCache::onMiss(address, state);
	}

	void onHit(ulong address, cacheState state)
	{
		if (accessCounter.find(address) != accessCounter.end()) 
			accessCounter[address]--;
		else
			accessCounter[address] = 0;	
		LRUCache::onHit(address, state);
	}

	bool isDeadBlock(ulong address)
	{
		return accessCounter.find(address) != accessCounter.end() && accessCounter[address] > MaxMiss;
	}

	void read(ulong address)
	{
		if (isDeadBlock(address))
			upperCache->read(address);
		else
			LRUCache::read(address);
	}
	void write(ulong address)
	{
		if (isDeadBlock(address))
			upperCache->write(address);
		else
			LRUCache::write(address);
	}
};

int main()
{}