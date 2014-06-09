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

		pageNode *newPage = new pageNode(address, state);
		newPage->next = pageHead;
		pageTable[address] = newPage;

		if(cacheSize_ == currentSize_)
		{
			//something has to be replaced
			//before doing LRU, see if any blocks are considered dead.
			bool foundDeadBlock = false;
			pageNode *current = pageHead;
			while (current != NULL)
			{
				if ((foundDeadBlock = isDeadBlock(current->thisPage.addr)))
				{
					evict(current->thisPage.addr);
					break;
				}
				current = current->next;
			}
			if (!foundDeadBlock)
			{
				//no dead blocks, fallback to LRU
				evict(pageTail->thisPage.addr);	
			}
			pageHead->prev = newPage;
			pageHead = newPage;		
		}
		else
		{
			if(pageHead == NULL)
			{
				pageHead = pageTail = newPage;
			}
			else
			{
				pageHead->prev = newPage;
				pageHead = newPage;	
			}				
		}
	}
};