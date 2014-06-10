#ifndef CACHE
#define CACHE

#include <iostream>

typedef unsigned long ulong;

enum cacheState {
	MESSY = 'M', 
	CLEAN = 'C', 
	INVALID = 'I'}; //for single-processor, cache state can either be messy or clean (maybe invalid depending on the algorithms used)

struct page
{
	ulong addr;
	cacheState state;
	
	page(ulong address, cacheState initialState) : addr(address), state(initialState) {}
	page(){}
};

class Cache
{	
public:
	ulong cacheSize_;
	ulong blockSize_;
	ulong currentSize_;
	ulong hitCount, missCount, readCount, writeCount;
	Cache *upperCache;
	Cache *lowerCache;
	Cache() : hitCount(0), missCount(0), readCount(0), writeCount(0) {}
	Cache(ulong cacheSize, ulong blockSize) : cacheSize_(cacheSize), blockSize_(blockSize), hitCount(0), missCount(0), readCount(0), writeCount(0), currentSize_(0) {}
	
	void setUpperCache(Cache *cache)
	{
		upperCache = cache;
	}
	void setLowerCache(Cache *cache)
	{
		lowerCache = cache;
	}
	virtual bool inCache(ulong ){return false;}
	virtual void insert(ulong , cacheState ) {}
	virtual page evict(ulong ) {return page();}
	virtual void read(ulong ) {}
	virtual void write(ulong ) {}	
	virtual void instruction_fetch(ulong ) {} //this may be unused
};

std::ostream& operator<<(std::ostream& stream, const Cache& cache) 
{
	stream << "hits: " << cache.hitCount << ", misses: " << cache.missCount << ", reads: " << cache.readCount << ", writes: " << cache.writeCount;
	return stream;
}

#endif