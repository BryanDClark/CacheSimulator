#include "LRU.hpp"
#include <map>

class L1Exclusive : public LRUCache
{
public:
	L1Exclusive(ulong cacheSize, ulong blockSize) : LRUCache(cacheSize, blockSize) {}
	void handleEviction(Cache *upperCache, Cache *lowerCache, page replacedPage)
	{
		//evictions from L1 are put in L2, which will always have space
		upperCache->insert(replacedPage.addr, replacedPage.state);
	}
	void read(ulong address)
	{
		if(pageTable.find(address) == pageTable.end())
		{
			missCount++;
			upperCache->read(address);			
		}
		else
		{
			readCount++;
			replace(address, CLEAN);
		}
	}
	void write(ulong address)
	{
		if(pageTable.find(address) == pageTable.end())
		{
			missCount++;
			upperCache->write(address);
		}
		else
		{
			//if it's in L1, update the LRU and mark as messy
			writeCount++;
			replace(address, MESSY);
		}
	}
};

class L2Exclusive : public LRUCache
{
public:
	L2Exclusive(ulong cacheSize, ulong blockSize) : LRUCache(cacheSize, blockSize) {}
	void read(ulong address)
	{
		if(pageTable.find(address) == pageTable.end())
		{
			//if there is an upper cache defined, call its read, otherwise, read from memory
			//Maybe it would be better if main memory were a special case of Cache?
			if(upperCache)
				upperCache->read(address);
			else
				memReadCount++;
			replace(address, CLEAN);
		}
		else
		{
			readCount++;
			evict(address);
			lowerCache->insert(address, CLEAN);
		}
	}
	void write(ulong address)
	{
		if(pageTable.find(address) == pageTable.end())
		{
			writeCount++;
			replace(address, MESSY);
			//TODO: Invalidate L3 cache for this address if necessary
		}
		else
		{
			evict(address);
			lowerCache->insert(address, MESSY);
		}
	}
};

int main()
{}