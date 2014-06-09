#include "LRU.hpp"
#include <map>

using namespace std;

class DBDCache : public LRUCache
{
public:
	const unsigned int MaxMiss = 3;
	std::map<ulong, ulong> accessCounter;
	DBDCache(ulong cacheSize, ulong blockSize) : LRUCache(cacheSize, blockSize) {}

	void onMiss(ulong address)
	{
		if (accessCounter.find(address) != accessCounter.end()) 
			accessCounter[address]++;
		else
			accessCounter[address] = 1;
	}

	void onHit(ulong address)
	{
		if (accessCounter.find(address) != accessCounter.end()) 
			accessCounter[address]--;
		else
			accessCounter[address] = 0;		
	}

	bool isDeadBlock(ulong address)
	{
		return accessCounter.find(address) != accessCounter.end() && accessCounter[address] > MaxMiss;
	}

	void insert(ulong address, cacheState state)
	{
		//if counter has reached MaxMiss threshold, don't bother caching it
		if (cacheSize_ == currentSize_ && isDeadBlock(address))
			return;

		if(cacheSize_ == currentSize_)
		{
			//before doing LRU, pull out a dead block.
			for (map<ulong, pageNode*>::iterator it = pageTable.begin(); it != pageTable.end(); ++it)
			{
				if (isDeadBlock(it->first))
				{
					evict(it->first);
					break;
				}
			}
		}
		LRUCache::insert(address, state);
	}
};