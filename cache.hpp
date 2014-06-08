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
};

class Cache
{	
public:
	ulong cacheSize_;
	ulong blockSize_;
	ulong currentSize_;
	ulong hitCount, missCount, readCount, writeCount, memWriteCount;
	Cache *upperCache;
	Cache *lowerCache;
	Cache(ulong cacheSize, ulong blockSize) : cacheSize_(cacheSize), blockSize_(blockSize) {}
	
	void setUpperCache(Cache *cache)
	{
		upperCache = cache;
	}
	void setLowerCache(Cache *cache)
	{
		lowerCache = cache;
	}
	
	virtual void read(ulong address) {}
	virtual void write(ulong address) {}	
	virtual void instruction_fetch(ulong address) {} //this may be unused
};