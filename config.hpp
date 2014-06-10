#ifndef CONFIG_HPP
#define CONFIG_HPP

#include <string>
#include "shared.hpp"

class Config
{	
public:
	ulong memSize, numChips;
	ulong replacementSpeed;
	ulong broadcastSpeed;
	ulong memoryAccessSpeed;
	ulong l3CacheSize;
	ulong l3CacheAccessSpeed;
	ulong cacheLineSize;

	ulong* numCores;
	ulong* cacheSizes;
	ulong* cacheAccessSpeeds;

	ulong L1CacheSize;
	ulong L1BlockSize;
	ulong L2CacheSize;
	ulong L2BlockSize;
	ulong L3CacheSize;
	ulong L3BlockSize;

	void initialize(int argc,char *argv[]);

private:
	bool isInit(std::string command);
};

#endif