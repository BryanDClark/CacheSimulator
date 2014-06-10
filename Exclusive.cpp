#ifndef EXCLUSIVE
#define EXCLUSIVE

#include "LRU.hpp"

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
		if(!inCache(address))
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
		if(!inCache(address))
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
	virtual void read(ulong address)
	{
		if(!inCache(address))
		{
			//read from next level
			upperCache->read(address);
			replace(address, CLEAN);
		}
		else
		{
			readCount++;
			evict(address);
			lowerCache->insert(address, CLEAN);
		}
	}
	virtual void write(ulong address)
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

class L2ExclusivePrefetch : public L2Exclusive
{
public:
	L2ExclusivePrefetch(ulong cacheSize, ulong blockSize) : L2Exclusive(cacheSize, blockSize) {}
	void read(ulong address)
	{
		bool cacheFound = inCache(address);
		L2Exclusive::read(address);
		//for prefetch, we grab the next cache line to prepare for L1
		if(cacheFound)
			replace(address+blockSize_, CLEAN);
	}
};
#endif