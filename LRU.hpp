#include "cache.hpp"
#include <map>

struct pageNode
{
	page thisPage;
	pageNode *next;
	pageNode *prev;
	
	pageNode(ulong address, cacheState initialState) : thisPage(address, initialState), next(NULL), prev(NULL) {}
	pageNode(){}
};

class LRUCache : public Cache
{
private:
	pageNode *pageHead;
	pageNode *pageTail;
	
public:
	std::map<ulong, pageNode*> pageTable;
	LRUCache(ulong cacheSize, ulong blockSize) : Cache(cacheSize, blockSize), pageHead(NULL), pageTail(NULL) {}
	
	virtual void onMiss(ulong )	{}
	virtual void onHit(ulong ) {}
	
	virtual void handleEviction(Cache *upperCache, Cache *lowerCache, page replacedPage)
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

	void evict(ulong address)
	{
		pageNode *replacedPage = pageTable[address];
		pageTable.erase(replacedPage->thisPage.addr);
		handleEviction(upperCache, lowerCache, replacedPage->thisPage);		
		
		if(replacedPage == pageTail)
		{
			pageTail->prev->next = NULL;
			pageTail = pageTail->prev;
		}
		else if(replacedPage == pageHead)
		{
			pageHead->next->prev = NULL;
			pageHead = pageHead->next;
		}
		else
		{
			replacedPage->prev->next = replacedPage->next;
			replacedPage->next->prev = replacedPage->prev;
		}		
		delete replacedPage;
	}
	
	void insert(ulong address, cacheState state)
	{
		pageNode *newPage = new pageNode(address, state);
		newPage->next = pageHead;
		pageTable[address] = newPage;

		if(cacheSize_ == currentSize_)
		{
			//something has to be replaced
			evict(pageTail->thisPage.addr);	
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
	bool replace(ulong address, cacheState state)
	{
		if(pageTable.find(address) == pageTable.end() || pageTable[address]->thisPage.state == INVALID)
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
			pageNode *accessedPage = pageTable[address];
			if(accessedPage != pageHead)
			{
				if(accessedPage == pageTail)
				{
					pageTail = pageTail->prev;
				}
				else
				{
					accessedPage->next->prev = accessedPage->prev;
				}
				accessedPage->prev->next = accessedPage->next;
				accessedPage->prev = NULL;
				accessedPage->next = pageHead;
				pageHead->prev = accessedPage;
				pageHead = accessedPage;
			}
			
			return true;
		}
	}
	virtual void read(ulong address)
	{
		readCount++;
		replace(address, CLEAN);
	}
	virtual void write(ulong address)
	{
		writeCount++;
		replace(address, MESSY);
	}
};