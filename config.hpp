#ifndef CONFIG_HPP
#define CONFIG_HPP

#include <string>
#include "shared.hpp"

class Config
{	
public:
	ulong memSize, numChips;
	ulong replacementSpeed = 0;
	ulong broadcastSpeed = 0;
	ulong memoryAccessSpeed = 0;
	ulong l3CacheSize = 0;
	ulong l3CacheAccessSpeed = 0;
	ulong cacheLineSize = 0;

	ulong* numCores;
	ulong* cacheSizes;
	ulong* cacheAccessSpeeds;

	ulong L1CacheSize = 0;
	ulong L1BlockSize = 0;
	ulong L2CacheSize = 0;
	ulong L2BlockSize = 0;
	ulong L3CacheSize = 0;
	ulong L3BlockSize = 0;

	void initialize(int argc,char *argv[]);

private:
	bool isInit(std::string command);
};

#endif