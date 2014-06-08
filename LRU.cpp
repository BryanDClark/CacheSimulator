#include "cache.hpp"
#include <map>
#include <list>

class LRUCache : public Cache
{
public:
	std::map<ulong, std::list<page>::iterator> pageTable;
	std::list<page> pageQueue;
	LRUCache(ulong cacheSize, ulong blockSize) : Cache(cacheSize, blockSize) {}
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
				page replacedPage = pageQueue.back();
				pageTable.erase(replacedPage.addr);
				if(replacedPage.state == 'M')
				{
					//messy state, need to writeback
					if(upperCache)
						upperCache->write(address);
					else
						memWriteCount++;
				}
			}
		}
		else
		{
			hitCount++;
			auto iter = pageTable[address];
			page accessedPage = *iter;
			pageQueue.erase(iter);
			pageQueue.push_front(accessedPage);
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