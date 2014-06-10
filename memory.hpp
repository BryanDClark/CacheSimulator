#ifndef MEMORY
#define MEMORY

#include "cache.hpp"

class Memory : public Cache
{
public:
	void read(ulong )
	{
		//std::cout << "memory read" << std::endl;
		readCount++;
	}
	void write(ulong )
	{
		//std::cout << "memory write" << std::endl;
		writeCount++;
	}
};

#endif