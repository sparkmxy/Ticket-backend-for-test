#ifndef TOKENSCANNER_HPP
#define TOKENSCANNER_HPP

#include <string>
#include "tool.h"
using std::string;

/*
UPDATE 0516
把内部的变成了char *str;
nextToken返回值改为String（需要String中增加+=运算符和addend）
新增firstToken和原来的nextToken一样
*/
class TokenScanner
{
private:
	char *str;
	int ptr = 0;
	int len = 0;

public:
	void setInput(string strr)
	{
		len = strr.length();
		str = new char[len];
		for (int i = 0; i < len; ++i) str[i] = strr[i];
	}

	void ignoreWhiteSpace()
	{
		for (; ptr < len && (str[ptr] == ' ' || str[ptr] == '\n'); ++ptr);
	}

	bool hasMoreTokens()
	{
		ignoreWhiteSpace();
		return (ptr < len);
	}

	/*int isINT(string str)//???
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
	string firstToken()
	{
		string ret;
		ignoreWhiteSpace();

		for (; ptr < len && !(str[ptr] == ' ' || str[ptr] == '\n'); ++ptr) ret += str[ptr];
		return ret;
	}

	String nextToken()
	{
		String ret;
		ignoreWhiteSpace();

		for (; ptr < len && !(str[ptr] == ' ' || str[ptr] == '\n'); ++ptr) ret += str[ptr];
		ret.addend();
		return ret;
	}

};

#endif //! TOKENSCANNER_HPP