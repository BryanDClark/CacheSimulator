#include "cache.hpp"
#include <map>
#include <list>

using namespace std;

class DBDCache : public Cache
{
public:
	std::map<ulong, std::list<page>::iterator> pageTable;
	std::list<page> pageQueue;
	std::map<ulong, ulong> accessCounter;
	DBDCache(ulong cacheSize, ulong blockSize) : Cache(cacheSize, blockSize) {}
	void replace(ulong address, cacheState state)
	{
		if(pageTable.find(address) == pageTable.end())
		{
			//not currently in the cache
			missCount++;

			page newPage(address, state);
			pageQueue.push_front(newPage);
			pageTable[address] = pageQueue.begin();	
			if(cacheSize_ == currentSize_)
			{
				//something has to be replaced
				auto iter = pageQueue.end();
				//check if this entry has been cached before and used only once
				//if so, don't bother caching it
				if(accessCounter.find(address) != accessCounter.end() && accessCounter[address] == 1)
					iter = pageQueue.begin();
				//re(set) the counter for the address to 1
				accessCounter[address] = 1;

				pageTable.erase(iter->addr);
				if(iter->state == 'M')
				{
					//messy state, need to writeback
					if(upperCache)
						upperCache->write(iter->addr);
					else
						memWriteCount++;
				}
				pageQueue.erase(iter);
			}
		}
		else
		{
			hitCount++;
			auto iter = pageTable[address];
			page accessedPage = *iter;
			pageQueue.erase(iter);
			pageQueue.push_front(accessedPage);

			if (accessCounter.find(address) != accessCounter.end()) 
				accessCounter[address]++;
			else
				accessCounter[address] = 1;
			//off-hand, I'm not sure how we'd get in that else clause state
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