#ifndef TOKENSCANNER_HPP
#define TOKENSCANNER_HPP

#include <string>
#include "tool.h"
using std::string;

class TokenScanner
{
private:
	string str;
	int ptr = 0;

public:
	void setInput(const string &strr)
	{
		str = strr;
	}

	void ignoreWhiteSpace()
	{
		for (; ptr < str.length() && (str[ptr] == ' ' || str[ptr] == '\n'); ++ptr);
	}

	bool hasMoreTokens()
	{
		ignoreWhiteSpace();
		return (ptr < str.length());
	}
	
	/*int is_INT(string str)//???
	{
		int num = 0;
		if (str.length() == 0) return -1;
		for (int i = 0; i < (int)str.length(); ++i)
		{
			if (str[i] < '0' || str[i] > '9') return -1;
			num = num*10 + str[i] - '0';
		}
		return num;
	}*/

	string nextToken()
	{
		string ret = "";
		ignoreWhiteSpace();

		for (; ptr < str.length() && !(str[ptr] == ' ' || str[ptr] == '\n'); ++ ptr) ret += str[ptr];
		return ret;
	}
	
};

#endif //! TOKENSCANNER_HPP