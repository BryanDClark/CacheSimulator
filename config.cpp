#include "config.hpp"
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

string cacheSizeError = "Error: Usage is: L*CacheSize([<chip_idx>,] <size> <KB|MB|GB>)";
string blockSizeError = "Error: Usage is: L*BlockSize(<size> <B|KB>)";
string cacheAccessSpeedError = "Error: Usage is: cacheAccessSpeed([<chip_idx>,] <size> <us|ns>)";

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

ulong getBlockSize(string size)
{
	vector<string> blockSizeStr = parseSize(size);
	ulong blockSize = strtoul(blockSizeStr[0].c_str(), NULL, 0);

	if(blockSizeStr[1] == "KB")
		blockSize <<= 10;
	else if(blockSizeStr[1] != "B")
		throw invalid_argument(blockSizeError);
	return blockSize;
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

bool Config::isInit(string command)
{
	vector<string> commands {"L1CacheSize", "L1BlockSize", "L2CacheSize", "L2BlockSize", "L3CacheSize", "L3BlockSize", "blockSize", "cacheLineSize", "cacheSize", "cacheAccessSpeed", "replacementSpeed", "broadcastSpeed", "memoryAccessSpeed"};
	for(size_t i = 0; i < commands.size(); i++)
	{
		if(command == commands[i])
			return true;
	}
	return false;
}

void Config::initialize(int argc,char *argv[], bool skipPreamble) 
{
	string configFilePath = "config.txt";
	//assume this is the file config
	if (argc == 2)
		configFilePath = argv[1];
	
	ifstream configFile(configFilePath);
	if (!configFile.is_open()) 
		throw invalid_argument("Error: Configuration file could not be opened.");

	string command = getUntilDelim(configFile, '(');
	vector<string> arguments;

	if (!skipPreamble)
	{
		string argument = getUntilDelim(configFile, ')');
		arguments = parseSize(argument);

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
		
		numCores = new ulong[numChips];
		cacheSizes = new ulong[numChips];
		cacheAccessSpeeds = new ulong[numChips];
		
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

		else if(command == "L1CacheSize")
			L1CacheSize = getCacheSize(arguments[0]);
		else if(command == "L1BlockSize")
			L1BlockSize = getBlockSize(arguments[0]);
		else if(command == "L2CacheSize")
			L2CacheSize = getCacheSize(arguments[0]);
		else if(command == "L2BlockSize")
			L2BlockSize = getBlockSize(arguments[0]);
		else if(command == "L3CacheSize")
			L3CacheSize = getCacheSize(arguments[0]);
		else if(command == "L3BlockSize")
			L3BlockSize = getBlockSize(arguments[0]);
		else if(command == "blockSize")
			blockSize = getBlockSize(arguments[0]);
		
		command = getUntilDelim(configFile, '(');
	}
	
	if (!skipPreamble)
	{
		//TODO: check to make sure inits are non-zero
		if(replacementSpeed == 0 || broadcastSpeed == 0 || memoryAccessSpeed == 0)
			throw invalid_argument("Error: Detected missing or zero initialized argument");
		checkArray(numCores, numChips);
		checkArray(cacheSizes, numChips);
		checkArray(cacheAccessSpeeds, numChips);
	}
}