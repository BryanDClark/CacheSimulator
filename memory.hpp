#include "cache.hpp"

class Memory : public Cache
{
	void read(ulong )
	{
		readCount++;
	}
	void write(ulong )
	{
		writeCount++;
	}
}