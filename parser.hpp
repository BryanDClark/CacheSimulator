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

template<class T>
char goToChar(T &stream, char end)
{
	char c = stream.get();
	while(c != end && !stream.eof())
	{
		c = stream.get();
	}
	
	return c;
}

template<class T>
char trimWhitespace(T &stream)
{
	char c = stream.get();
	while((isspace(c) || c == '#') && !stream.eof())
	{
		if(c == '#')
		{
			c =	goToChar(stream, '\n');
		}
		else
		{
			c = stream.get();
		}
	}	
	return c;
}

template<class T>
std::vector<std::string> splitUntilDelim(T &stream, char split, char delim)
{
	char c = trimWhitespace(stream);
	std::vector<std::string> arguments;
	std::string arg = "";
	
	while(c != delim && !stream.eof())
	{
		if(c == '#')
		{
			goToChar(stream, '\n');
			c = trimWhitespace(stream);
		}
		else if(c == split)
		{
			arguments.push_back(arg);
			arg = "";
			c = trimWhitespace(stream);
		}
		else
		{
			arg+=c;
			c = stream.get();
		}
	}
	if(arg != "")
		arguments.push_back(arg);
	return arguments;
}

inline std::vector<std::string> splitDirectory(std::string &str, bool &relativePath)
{
	std::stringstream stream(str);
	char c = trimWhitespace(stream);
	std::vector<std::string> directorySeq;
	std::string directoryName = "";
	relativePath = true;
	
	if(c == '/')
	{
		relativePath = false;
		c = stream.get();
	}	
	
	while(stream.good())
	{
		if(c == '/')
		{
			directorySeq.push_back(directoryName);
			directoryName = "";
		}
		else
		{
			directoryName += c;
		}
		c = stream.get();
	}
	
	directorySeq.push_back(directoryName);
	
	return directorySeq;
}

template<class T>
std::string getUntilDelim(T &stream, char delim)
{
	char c = trimWhitespace(stream);
	std::string word = "";
	while(c != delim && !stream.eof())
	{
		if(c == '#')
		{
			goToChar(stream, '\n');
			c = trimWhitespace(stream);
		}
		else
		{
			if(!isspace(c))
				word+=c;
			c = stream.get();
		}
	}	
	return word;
}

inline std::vector<std::string> parseSize(const std::string &str)
{
	std::stringstream stream(str);
	char c = trimWhitespace(stream);
	std::vector<std::string> arguments;
	std::string arg = "";
	
	while(isdigit(c) || c == ',' || isspace(c))
	{
		if(c != ',')
			arg += c;
		c = stream.get();
	}
	arguments.push_back(arg);
	
	arg = "";
	while(c != ' ' && stream.good())
	{
		arg += c;
		c = stream.get();
	}
	arguments.push_back(arg);
	
	return arguments;
}
