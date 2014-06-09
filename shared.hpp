#ifndef SHARED_HPP
#define SHARED_HPP

#include <string>

typedef unsigned long ulong;
enum sizeUnit {B, KB, MB, GB, INVAL};

inline sizeUnit convertSize(std::string size)
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

#endif