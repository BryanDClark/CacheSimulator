#include "LRU.hpp"
#include <map>

using namespace std;

class DeadBlockLRUCache : public LRUCache
{
public:
	const unsigned int MaxMiss = 3;
	std::map<ulong, ulong> accessCounter;
	DeadBlockLRUCache(ulong cacheSize, ulong blockSize) : LRUCache(cacheSize, blockSize) {}

	void onMiss(ulong address)
	{
		if (accessCounter.find(address) != accessCounter.end())
		{
			accessCounter[address]++;
			if(accessCounter[address] == 4)
				return; //blocks is now dead
		}
		else
			accessCounter[address] = 1;
			
		LRUCache::onMiss(address);
	}

	void onHit(ulong address)
	{
		if (accessCounter.find(address) != accessCounter.end()) 
			accessCounter[address]--;
		else
			accessCounter[address] = 0;	
		LRUCache::onHit(address);
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
	void write()
	{
		if (isDeadBlock(address))
			upperCache->write(address);
		else
			LRUCache::write(address);
	}
};