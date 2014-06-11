#ifndef LRU
#define LRU

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
protected:
	pageNode *pageHead;
	pageNode *pageTail;
	std::map<ulong, pageNode*> pageTable;
		
public:
	LRUCache(ulong cacheSize, ulong blockSize) : Cache(cacheSize, blockSize), pageHead(NULL), pageTail(NULL) {}
	
	virtual void onMiss(ulong address, cacheState state)	
	{
		missCount++;
		insert(address, state);
	}
	virtual void onHit(ulong, cacheState ) 
	{
		hitCount++;
	}
	
	virtual void handleEviction(Cache *upperCache, Cache* , page replacedPage)
	{
		if(replacedPage.state == 'M')
		{
			//messy state, need to writeback
			upperCache->write(replacedPage.addr);
		}		

	}
	
	bool inCache(ulong address)
	{
		return pageTable.find(address) != pageTable.end();
	}
	page evict(ulong address)
	{
		pageNode *replacedPage = pageTable[address];
		pageTable.erase(replacedPage->thisPage.addr);
		page returnPage = replacedPage->thisPage;	
		if(pageTail == pageHead)
		{
			pageHead=NULL;
			pageTail=NULL;
		}
		else if(replacedPage == pageTail)
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
		currentSize_--;
		delete replacedPage;
		return returnPage;
	}
	
	virtual void insert(ulong address, cacheState state)
	{
		pageNode *newPage = new pageNode(address, state);
		newPage->next = pageHead;
		pageTable[address] = newPage;
		if(cacheSize_ == currentSize_)
		{
			//something has to be replaced
			page victim = evict(pageTail->thisPage.addr);
			handleEviction(upperCache, lowerCache, victim);
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
		currentSize_++;
	}
	void replace(ulong address, cacheState state)
	{
		if(!inCache(address) || pageTable[address]->thisPage.state == INVALID)
		{
			//not currently in the cache
			//std::cout << "onMiss" << std::endl;
			onMiss(address, state);
		}
		else
		{
			//std::cout << "beforehit" << std::endl;
			onHit(address, state);
			//std::cout << "afterhit" << std::endl;
			pageNode *accessedPage = pageTable[address];
			if(state == MESSY)
				accessedPage->thisPage.state = state;
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
		}
	}
	virtual void read(ulong address)
	{
		readCount++;
		if(!inCache(address))
			upperCache->read(address);
		replace(address, CLEAN);
	}
	virtual void write(ulong address)
	{
		writeCount++;
		replace(address, MESSY);
	}
};

#endif