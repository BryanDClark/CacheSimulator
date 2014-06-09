#include "cache.hpp"
#include <map>
#include <vector>
#include <queue>
#include <stdlib.h>

class SimpleCache : public Cache
{
public:
	std::map<ulong, page*> pageTable;
	SimpleCache(ulong cacheSize, ulong blockSize) : Cache(cacheSize, blockSize) {}
	
	void onMiss(ulong )	{}
	void onHit(ulong ) {}
	
	void handleEviction(Cache *upperCache, Cache *lowerCache, page replacedPage)
	{
		if(replacedPage.state == 'M')
		{
			//messy state, need to writeback
			if(upperCache)
				upperCache->write(replacedPage.addr);
			else
				memWriteCount++;	
		}
	}

	virtual void evict(ulong ) {}
	virtual void insert(ulong , cacheState ) {}

	bool replace(ulong address, cacheState state)
	{
		if(pageTable.find(address) == pageTable.end() || pageTable[address]->state == INVALID)
		{
			//not currently in the cache
			missCount++;
			onMiss(address);
			insert(address, state);
			return false;
		}
		else
		{
			hitCount++;
			onHit(address);
			return true;
		}
	}

	void read(ulong address)
	{
		readCount++;
		replace(address, CLEAN);
	}

	void write(ulong address)
	{
		writeCount++;
		replace(address, MESSY);
	}
};

class RandomCache : public SimpleCache
{
private:
	std::vector<page*> pageVector;

public:
	RandomCache(ulong cacheSize, ulong blockSize) : SimpleCache(cacheSize, blockSize)
	{
		srand(time(NULL));
	}

	void evict(ulong index)
	{
		page *replacedPage = pageVector[index];
		pageVector[index] = NULL; //no need to erase from the vector; this is a swap
		pageTable.erase(replacedPage->addr);		
		handleEviction(upperCache, lowerCache, *replacedPage);		
		delete replacedPage;
	}

	void insert(ulong address, cacheState state)
	{
		page *newPage = new page(address, state);
		pageTable[address] = newPage;

		if(cacheSize_ == currentSize_)
		{
			//something has to be replaced
			ulong index = rand() % cacheSize_;
			evict(index);
			pageVector[index] = newPage;
		}
		else
		{
			pageVector.push_back(newPage);			
		}
	}
};

class FIFOCache : public SimpleCache
{
private:
	std::queue<page*> pageQueue;

public:
	FIFOCache(ulong cacheSize, ulong blockSize) : SimpleCache(cacheSize, blockSize) {}

	void evict(ulong )
	{
		page *replacedPage = pageQueue.front();
		pageQueue.pop();		
		pageTable.erase(replacedPage->addr);		
		handleEviction(upperCache, lowerCache, *replacedPage);		
		delete replacedPage;
	}

	void insert(ulong address, cacheState state)
	{
		page *newPage = new page(address, state);
		pageTable[address] = newPage;

		if(cacheSize_ == currentSize_)
		{
			//something has to be replaced
			evict(0);
		}
		pageQueue.push(newPage);
	}
};