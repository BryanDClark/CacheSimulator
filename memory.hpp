#ifndef MEMORY
#define MEMORY

#include "cache.hpp"

class Memory : public Cache
{
public:
	void read(ulong )
	{
		readCount++;
	}
	void write(ulong )
	{
		writeCount++;
	}
};

#endif