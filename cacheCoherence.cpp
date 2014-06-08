#include <string>
#include <map>
#include <vector>
#include <iostream>
#include <sstream>
#include <stdlib.h>
#include <fstream>
#include <stdio.h>
#include <algorithm>
#include <functional>
#include <stdexcept>
#include "parser.hpp"

using namespace std;
typedef unsigned long ulong;
string cacheSizeError = "Error: Usage is: cacheSize([<chip_idx>,] <size> <KB|MB|GB>)";
string cacheAccessSpeedError = "Error: Usage is: cacheAccessSpeed([<chip_idx>,] <size> <us|ns>)";
enum sizeUnit {B, KB, MB, GB, INVAL};
enum cacheState {M, E, S, I};

string stateString(cacheState state)
{
	switch(state)
	{
		case M: return "M";
		case E: return "E";
		case S: return "S";
		case I: return "I";
	}
	return "0";
}

sizeUnit convertSize(string size)
{
	if(size == "GB")
		return GB;
	else if(size == "MB")
		return MB;
	else if(size == "KB")
		return KB;
	else if(size == "B")
		return B;
	return INVAL;
}

template<class T>
void checkArray(T array, size_t size)
{
	for(size_t i = 0; i < size; i++)
	{
		if(array[i] == 0)
			throw invalid_argument("Detected missing or zero initalized argument");
	}
}

template<class T>
string printArray(T array, size_t size)
{
	stringstream stream;
	for(size_t i = 0; i < size; i++)
	{
		stream << i << ": " << array[i] << ", ";
	}
	
	return stream.str();
}

ulong parseAddress(string hexAddr)
{
	ulong addr;
	string str;
	stringstream stream(hexAddr);
	
	getline(stream, str, 'x');
	if(str != "0")
		throw invalid_argument("Error: " + hexAddr + " is an improperly formatter hex address");
	stream >> hex >> addr;
		
	return addr;
}

ulong getCacheSize(string size)
{
	vector<string> cacheSizeStr = parseSize(size);
	ulong cacheSize = strtoul(cacheSizeStr[0].c_str(), NULL, 0) << 10;
	
	switch(convertSize(cacheSizeStr[1]))
	{
		case GB:
			cacheSize <<= 10;
		case MB:
			cacheSize <<= 10;
		case KB:
			break;
		default:
			throw invalid_argument(cacheSizeError);			
	}	
	return cacheSize;	
}

ulong parseSpeed(vector<string> arguments, string command)
{
	if(arguments.size() != 1)
		throw invalid_argument("Error: Usage is: " + command + "(<size> <us|ns>)");
	vector<string> speedStr = parseSize(arguments[0]);
	ulong speed = strtoul(speedStr[0].c_str(), NULL, 0);
	if(speedStr[1] == "us")
		speed *= 1000;
	else if(speedStr[1] != "ns")
		throw invalid_argument("Error: Usage is: " + command + "(<size> <us|ns>)");
		
	return speed;
}

ulong getCacheAccessSpeed(string size)
{
	vector<string> cacheAccessSpeedStr = parseSize(size);
	ulong cacheAccessSpeed = strtoul(cacheAccessSpeedStr[0].c_str(), NULL, 0);
	
	if(cacheAccessSpeedStr[1] == "us")
		cacheAccessSpeed *= 1000;
	else if(cacheAccessSpeedStr[1] != "ns")
		throw invalid_argument(cacheAccessSpeedError);
	
	return cacheAccessSpeed;	
}

bool isInit(string command)
{
	vector<string> commands {"cacheLineSize", "cacheSize", "cacheAccessSpeed", "replacementSpeed", "broadcastSpeed", "memoryAccessSpeed"};
	for(size_t i = 0; i < commands.size(); i++)
	{
		if(command == commands[i])
			return true;
	}
	return false;
}

struct page
{
	ulong addr;
	ulong index;
	ulong ownerChip;
	ulong ownerCore;
	cacheState state;
	page *prev;
	page *next;
};

//class for maintaining cache. Uses a map for quick lookup and linked list for LRU
class Cache
{
private:
	ulong size;
	page *pageHead;
	page *pageTail;
	ulong currIndex;
	
public:
	map<ulong, page*> pageTable;
	Cache(ulong cacheSize)
	{
		size = cacheSize;
		currIndex = 0;
		pageHead = NULL;
		pageTail = NULL;
	}
	//reports hit/miss on an address, keeps the cache structure up to date using LRU
	//returns the address of the entry that was replaced, if relevant
	ulong access(ulong addr, bool &hit, bool &replaced, cacheState &replacedState)
	{
		if(pageTable.find(addr) == pageTable.end())
		{
			//not currently in the cache
			hit = false;
			if(pageTable.size() == size)
			{
				//something has to be replaced
				replaced = true;
				replacedState = pageTail->state;
				page *newPage = new page;
				newPage->addr = addr;
				newPage->next = pageHead;
				newPage->prev = NULL;	
				newPage->index = pageTail->index;
				pageTable[addr] = newPage;
				pageHead->prev = newPage;
				pageHead = newPage;
				//remove last element
				page *removedPage = pageTail;
				ulong removedAddr = pageTail->addr;

				
				pageTail->prev->next = NULL;
				pageTail = pageTail->prev;
				pageTable.erase(removedPage->addr);	
				delete removedPage;
				
				return removedAddr;
			}
			else
			{
				//cache not full, nothing needs to be replaced
				replaced = false;
				page *newPage = new page;
				newPage->addr = addr;
				newPage->next = pageHead;
				newPage->prev = NULL;
				newPage->index = currIndex++;
				pageTable[addr] = newPage;
				
				if(pageHead != NULL)
					pageHead->prev = newPage;
				pageHead = newPage;
				
				if(pageTail == NULL)
					pageTail = pageHead;
			}
		}
		else
		{
			hit = true;
			replaced = false;
			page *accessedPage = pageTable[addr];
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
		return 0;
	}
};

int main(int argc,char *argv[])
{
	string configFilePath = "config.txt";
	if (argc == 2)
	{
		//assume this is the file config
		configFilePath = argv[1];
	}
	
	ifstream configFile(configFilePath);
	if (!configFile.is_open()) 
	{
		cerr << "Error: Configuration file could not be opened." << endl;
		return 0;
	}

	try
	{
		ulong memSize, numChips;
		ulong replacementSpeed = 0;
		ulong broadcastSpeed = 0;
		ulong memoryAccessSpeed = 0;
		ulong l3CacheSize = 0;
		ulong l3CacheAccessSpeed = 0;
		ulong cacheLineSize = 0;
		string command = getUntilDelim(configFile, '(');
		string argument = getUntilDelim(configFile, ')');
		
		vector<string> arguments = parseSize(argument);

		if(command != "memorySize")
			throw invalid_argument("Error: First command must be \"memorySize()\"");
		if(arguments.size() != 2 || (arguments[1] != "MB" && arguments[1] != "GB" && arguments[1] != "KB"))
			throw invalid_argument("Error: Usage is \"memorySize(<size> <KB|MB|GB>)\"");
			
		memSize = strtoul(arguments[0].c_str(), NULL, 0) << 10;
		if(arguments[1] == "MB")
			memSize <<= 10;	
		else if(arguments[1] == "GB")
			memSize <<= 20;	
		
		command = getUntilDelim(configFile, '(');
		if(command != "numOfChips" && command != "numOfCores")
			throw invalid_argument("Error: Second command must be numOfChips() or numOfCores()");
		if(command == "numOfChips")
		{
			argument = getUntilDelim(configFile, ')');
			numChips = strtoul(argument.c_str(), NULL, 0);		
			command = getUntilDelim(configFile, '(');
		}
		else
		{
			numChips = 1;
		}
		
		ulong* numCores = new ulong[numChips];
		ulong* cacheSizes = new ulong[numChips];
		ulong* cacheAccessSpeeds = new ulong[numChips];
		
		for(ulong i = 0; i < numChips; i++)
		{
			if(command != "numOfCores")
				throw invalid_argument("Error: Expected numOfCores()");
			arguments = splitUntilDelim(configFile, ',', ')');
			if(arguments.size() == 1)
			{
				//all chips have same core #
				for(ulong j = 0; j < numChips; j++)
				{
					numCores[j] = strtoul(arguments[0].c_str(), NULL, 0);
				}
				i = numChips; // end loop
			}
			else if(arguments.size() == 2)
			{
				ulong core = strtoul(arguments[0].c_str(), NULL, 0);
				numCores[core] = strtoul(arguments[1].c_str(), NULL, 0);
			}
			else
			{
				throw invalid_argument("Error: Usage is: numOfCores([<chip_idx>,] <size>)");
			}
			command = getUntilDelim(configFile, '(');
		}
		
		while(isInit(command))
		{
			//cout << command << endl;
			arguments = splitUntilDelim(configFile, ',', ')');
			
			if(command == "cacheLineSize")
			{
				vector<string> cacheLineSizeStr = parseSize(arguments[0]);
				cacheLineSize = strtoul(cacheLineSizeStr[0].c_str(), NULL, 0);
				if(cacheLineSizeStr[1] == "KB")
					cacheLineSize <<= 10;
				else if(cacheLineSizeStr[1] != "B")
					throw invalid_argument("Error: Usage is: cacheLineSize(<size> <B|KB>)");
			}
				
			else if(command == "cacheSize")
			{
				if(arguments.size() == 1)
				{
					if(numChips == 1)
					{
						//this is single L2 size
						cacheSizes[0] = getCacheSize(arguments[0]);
					}
					else
					{
						//this is L3 size
						l3CacheSize = getCacheSize(arguments[0]);
					}
				}
				else if(arguments.size() == 2)
				{
					ulong cacheSize = getCacheSize(arguments[1]);
					ulong chip = strtoul(arguments[0].c_str(), NULL, 0);
					cacheSizes[chip] = cacheSize;
				}
				else
				{
					throw invalid_argument(cacheSizeError);
				}
			}
				
			else if(command == "cacheAccessSpeed")
			{
				if(arguments.size() == 1)
				{
					if(numChips == 1)
					{
						//this is single L2 speed
						cacheAccessSpeeds[0] = getCacheAccessSpeed(arguments[0]);
					}
					else
					{
						//this is L3 speed
						l3CacheAccessSpeed = getCacheAccessSpeed(arguments[0]);
					}					
				}
				else if(arguments.size() == 2)
				{
					ulong cacheAccessSpeed = getCacheAccessSpeed(arguments[1]);
					ulong chip = strtoul(arguments[0].c_str(), NULL, 0);
					cacheAccessSpeeds[chip] = cacheAccessSpeed;
				}
				else
				{
					throw invalid_argument(cacheAccessSpeedError);
				}
			}
				
			else if(command == "replacementSpeed")
			{
				replacementSpeed = parseSpeed(arguments, command);
			}
			else if(command == "broadcastSpeed")
			{
				broadcastSpeed = parseSpeed(arguments, command);
			}
			else if(command == "memoryAccessSpeed")
			{
				memoryAccessSpeed = parseSpeed(arguments, command);
			}			
			command = getUntilDelim(configFile, '(');
		}
		
		//TODO: check to make sure inits are non-zero
		if(cacheLineSize == 0 || replacementSpeed == 0 || broadcastSpeed == 0 || memoryAccessSpeed == 0)
			throw invalid_argument("Error: Detected missing or zero initialized argument");
		checkArray(numCores, numChips);
		checkArray(cacheSizes, numChips);
		checkArray(cacheAccessSpeeds, numChips);
		
		//create the caches
		Cache l3Cache = Cache(l3CacheSize / cacheLineSize);
		vector<Cache> l2Caches;
		
		for(ulong i = 0; i < numChips; i++)
		{
			l2Caches.push_back(Cache(cacheSizes[i] / cacheLineSize));
		}

		//configuration is loaded, switch back to grabbing input from cin.
		command = getUntilDelim(cin, '(');

		while(cin.good())
		{
			cout << command << "(";
			if(command != "read" && command != "write")
				throw invalid_argument("Error: Expected read/write, got " + command + ".");
				
			arguments = splitUntilDelim(cin, ',', ')');
			vector<string> sizeInfo;
			ulong addr, core, chip, size;
			
			if(arguments.size() == 3)
			{
				chip = 0;
				core = strtoul(arguments[0].c_str(), NULL, 0);
				addr = parseAddress(arguments[1]);
				sizeInfo = parseSize(arguments[2]);
			}			
			else if(arguments.size() == 4)
			{
				chip = strtoul(arguments[0].c_str(), NULL, 0);
				core = strtoul(arguments[1].c_str(), NULL, 0);
				addr = parseAddress(arguments[2]);
				sizeInfo = parseSize(arguments[3]);
				cout << chip << ", ";
			}
			else
			{
				throw invalid_argument("Error: " + command + " takes 3 or 4 arguments");
			}
			
			size = strtoul(sizeInfo[0].c_str(), NULL, 0);
			switch(convertSize(sizeInfo[1]))
			{
				case MB:
					size <<= 10;
				case KB:
					size <<= 10;
				case B:
					break;
				default:
					throw invalid_argument("Error: Size must be in <B|KB|MB>");
			}

			cout << core << ", 0x" << hex << addr << dec << ", " << sizeInfo[0] << sizeInfo[1] << ") ";
			
			ulong startingAddr = (addr / cacheLineSize) * cacheLineSize;
			ulong endingAddr = addr + size - cacheLineSize;
			endingAddr = (1 + ((endingAddr - 1) / cacheLineSize)) * cacheLineSize;
			stringstream L2idx;
			stringstream state;
			ulong L2WriteCount = 0;
			ulong L2ReadCount = 0;
			ulong L2MissCount = 0;
			ulong L2HitCount = 0;
			ulong L2WriteBack = 0;
			ulong replaceCount = 0;
			ulong broadcast = 0;
			ulong mem_read = 0;
			ulong total = 0;
			
			vector<ulong> L3idx;
			vector<cacheState> L3state;
			ulong L3WriteCount = 0;
			ulong L3ReadCount = 0;
			ulong L3MissCount = 0;
			ulong L3HitCount = 0;
			ulong L3WriteBack = 0;
			
			L2idx << "L2idx=";
			state << "state=";
			
			for(ulong i = startingAddr; i <= endingAddr; i+= cacheLineSize)
			{
				bool hit, wasReplaced;
				cacheState replacedState;
				ulong replaced;
				replaced = l2Caches[chip].access(i, hit, wasReplaced, replacedState);
				page* thisPage = l2Caches[chip].pageTable[i];
				hit = hit && (thisPage->state != I);
				
				L2idx << ((i != startingAddr) ? "&" : "") << thisPage->index;								
				
				if(command == "write")
					L2WriteCount++;
				else
					L2ReadCount++;
					
				if(!hit)
				{
					//not in L2
					L2MissCount++;
					thisPage->ownerCore = core;
					thisPage->ownerChip = chip;
					if(wasReplaced)
					{
						replaceCount++;
						if(replacedState == M)
						{
							//the replaced page needs to be written to memory
							L2WriteBack++;
						}
						if(replacedState == S)
						{
							//broadcast to L1 that this shared entry is invalid MAYBE
							broadcast++;
						}
					}
					
					if(command == "read")
					{
						thisPage->state = E;
						if(numChips == 1) //no L3
							mem_read++;
					}
					else
					{
						thisPage->state = M;
					}
					if(numChips > 1) //check L3
					{
						replaced = l3Cache.access(i, hit, wasReplaced, replacedState);
						thisPage = l3Cache.pageTable[i];
						L3idx.push_back(thisPage->index);						
						hit = hit && (thisPage->state != I);
						if(!hit)
						{
							//not in L3
							L3MissCount++;
							thisPage->ownerChip = chip;
							if(wasReplaced)
							{
								replaceCount++;
								if(replacedState == M)
								{
									L3WriteBack++;
								}
								if(replacedState == S)
								{
									broadcast++;
									//invalidate L2 that references this address MAYBE
									for(ulong j = 0; j < numChips; j++)
									{
										if(l2Caches[j].pageTable.find(replaced) != l2Caches[j].pageTable.end())
										{
											l2Caches[j].pageTable[replaced]->state = I;
										}
									}
								}
							}
							if(command == "read")
							{
								thisPage->state = E;
								mem_read++;
								L3ReadCount++;
							}
							else
							{
								L3WriteCount++;
								thisPage->state = M;
							}						
						}						
						else
						{
							//L3 hit
							L3HitCount++;
							if(command == "read")
							{
								L3ReadCount++;
								if(thisPage->ownerChip != chip && thisPage->state != S)
								{
									//another chip owns this non-shared page
									if(thisPage->state == M)
									{
										L3WriteBack++;
										//L2 for the owner should be E after the writeback
										l2Caches[thisPage->ownerChip].pageTable[thisPage->addr]->state = E;
									}
									thisPage->state = S;
								}
							}
							else
							{
								L3WriteCount++;
								if(!(thisPage->ownerChip == chip && (thisPage->state == M || thisPage->state == E)))
								{
									broadcast++;
									//invalidate L2s that hold this reference
									for(ulong j = 0; j < numChips; j++)
									{
										if(l2Caches[j].pageTable.find(thisPage->addr) != l2Caches[j].pageTable.end())
										{
											l2Caches[j].pageTable[thisPage->addr]->state = I;
										}
									}									
								}
								thisPage->state = M;
							}
						}
						
						L3state.push_back(thisPage->state);
					}
				}
				else
				{
					//L2 hit
					L2HitCount++;
					if(command == "read")
					{
						if((thisPage->ownerCore != core || thisPage->ownerChip != chip) && thisPage->state != S)
						{
							//another chip+core owns this non-shared page
							if(thisPage->state == M)
							{
								//other core owns this in a modified state, must writeback before setting to shared
								L2WriteBack++;
								if(l3Cache.pageTable.find(thisPage->addr) != l3Cache.pageTable.end())
								{
									//update L3 to be exclusive if it isn't.
									if(l3Cache.pageTable[thisPage->addr]->state == M)
									{
										l3Cache.pageTable[thisPage->addr]->state = E;
									}
								}
							}
							thisPage->state = S;
						}
					}
					else
					{
						//if(!(thisPage->ownerCore == core && thisPage->ownerChip == chip && (thisPage->state == M || thisPage->state == E)))
						if(thisPage->state == S) //criteria given in professor's email
						{
							//this is either shared or not our page, need to broadcast that it has been changed
							broadcast++;
						}
						thisPage->state = M;
						
						//must write to L3 as well...
						if(numChips > 1)
						{
							L3WriteCount++;
							replaced = l3Cache.access(i, hit, wasReplaced, replacedState);
							thisPage = l3Cache.pageTable[i];
							L3idx.push_back(thisPage->index);							
							hit = hit && (thisPage->state != I);
							
							if(hit)
							{
								L3HitCount++;
								//in L3 already, just need to update state to M and invalidate as necessary
								if(!(thisPage->ownerChip == chip && (thisPage->state == M || thisPage->state == E)))
								{
									broadcast++;
									//invalidate L2s that hold this reference
									for(ulong j = 0; j < numChips; j++)
									{
										if(l2Caches[j].pageTable.find(thisPage->addr) != l2Caches[j].pageTable.end())
										{
											l2Caches[j].pageTable[thisPage->addr]->state = I;
										}
									}									
								}
							}
							else
							{
								L3MissCount++;
								//not in L3...need to make space
								thisPage->ownerChip = chip;
								if(wasReplaced)
								{
									replaceCount++;
									if(replacedState == M)
									{
										L3WriteBack++;
									}
									if(replacedState == S)
									{
										broadcast++;
										//invalidate L2 that references this address MAYBE
										for(ulong j = 0; j < numChips; j++)
										{
											if(l2Caches[j].pageTable.find(replaced) != l2Caches[j].pageTable.end())
											{
												l2Caches[j].pageTable[replaced]->state = I;
											}
										}
									}
								}
							}
							thisPage->state = M;
							L3state.push_back(thisPage->state);
						}
					}
				}

				state << ((i != startingAddr) ? "&" : "") << stateString(thisPage->state);
			}
			
			total = cacheAccessSpeeds[chip] * (L2MissCount + L2HitCount + L2WriteCount + L2ReadCount) + broadcast * broadcastSpeed +
					replaceCount * replacementSpeed + memoryAccessSpeed * (L2WriteBack + mem_read + L3WriteBack)
					+ (L3MissCount + L3HitCount + L3WriteCount + L3ReadCount) * l3CacheAccessSpeed;
			
			cout << L2idx.str() << ", ";
			
			if(L3idx.size() > 0)
			{
				cout << "L3idx=";
				for(size_t i = 0; i < L3idx.size(); i++)
				{
					if(i!=0)
						cout << "&";
					cout << L3idx[i];
				}
				
				cout << ", ";
			}
			
			cout << "time(";
			
			if(L2MissCount) 
			{
				cout << "L2miss=" << cacheAccessSpeeds[chip] << "ns";
				if(L2MissCount != 1) cout << "*" << L2MissCount;
				cout << ", ";
			}
			if(L3MissCount) 
			{
				cout << "L3miss=" << l3CacheAccessSpeed << "ns";
				if(L3MissCount != 1) cout << "*" << L3MissCount;
				cout << ", ";
			}
			
			if(L2HitCount)
			{
				cout << "L2hit=" << cacheAccessSpeeds[chip] << "ns"; 
				if(L2HitCount != 1) cout << "*" << L2HitCount;
				cout << ", ";
			}
			
			if(L3HitCount)
			{
				cout << "L3hit=" << l3CacheAccessSpeed << "ns"; 
				if(L3HitCount != 1) cout << "*" << L3HitCount;
				cout << ", ";
			}
			
			if(broadcast)
			{
				cout << "broadcast=" << broadcastSpeed << "ns"; 
				if(broadcast != 1) cout << "*" << broadcast;
				cout << ", ";
			}
			
			if(L2WriteCount)
			{
				cout << "L2write=" << cacheAccessSpeeds[chip] << "ns"; 
				if(L2WriteCount != 1) cout << "*" << L2WriteCount;
				cout << ", ";
			}
			if(L3WriteCount)
			{
				cout << "L3write=" << l3CacheAccessSpeed << "ns"; 
				if(L3WriteCount != 1) cout << "*" << L3WriteCount;
				cout << ", ";
			}
			if(replaceCount)
			{
				cout << "replace=" << replacementSpeed << "ns"; 
				if(replaceCount != 1) cout << "*" << replaceCount;
				cout << ", ";
			}
			if(L2WriteBack)
			{
				cout << "L2writeback=" << memoryAccessSpeed << "ns"; 
				if(L2WriteBack != 1) cout << "*" << L2WriteBack;
				cout << ", ";
			}
			if(L3WriteBack)
			{
				cout << "L3writeback=" << memoryAccessSpeed << "ns"; 
				if(L3WriteBack != 1) cout << "*" << L3WriteBack;
				cout << ", ";
			}

			if(mem_read) 
			{
				cout << "mem_read=" << memoryAccessSpeed << "ns";
				if(mem_read != 1) cout << "*" << mem_read;
				cout << ", ";
			}
			
			if(L2ReadCount)
			{
				cout << "L2read=" << cacheAccessSpeeds[chip] << "ns"; 
				if(L2ReadCount != 1) cout << "*" << L2ReadCount;
				cout << ", ";
			}		
			if(L3ReadCount)
			{
				cout << "L3read=" << l3CacheAccessSpeed << "ns"; 
				if(L3ReadCount != 1) cout << "*" << L3ReadCount;
				cout << ", ";
			}		
			
			cout << "total=" << total << "ns), " << state.str();
			
			if(L3state.size() > 0)
			{
				cout << ", L3state=";
				for(size_t i = 0; i < L3state.size(); i++)
				{
					if(i!=0)
						cout << "&";
					cout << stateString(L3state[i]);
				}
				
			}
			
			cout << endl;
			
			command = getUntilDelim(cin, '(');
		}
	}
	catch(exception &e)
	{
		cerr << e.what() << endl;
	}
	return 0;
}