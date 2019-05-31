#pragma once
#pragma warning(disable : 4996)
//基于文件实现的一个vector

#include <string>
#include <iostream>
#include <stdio.h>
#include "tool.h"

using std::fstream;

template<class T>
class dataFile
{
	FILE        *_file;
	long        num;
	const int   Tsize;
public:
	// 构造函数：参数为文件名，如果不存在就创建
	dataFile(const std::string &name) : Tsize(sizeof(T))
	{
		_file = fopen(name.c_str(), "rb+");
		if (!_file) _file = fopen(name.c_str(), "wb+"), num = 0;
		else fread(&num, sizeof(long), 1, _file);
	}
	dataFile() = delete;
	~dataFile()
	{
		fseek(_file, 0, SEEK_SET);
		fwrite(&num, sizeof(long), 1, _file);
		fclose(_file);
	}


	// get: return the i th record, 
	// 1-base and the following functions are the same
	T get(int i) const;

	// replace the i th record with 'now'
	void replace(const T& now, int i);

	// push in a new record
	void push(const T& ele);

	void pop();

	int size() const { return num; }

	void loadAll(T *ptr);

	void load(int l, int r, T *ptr);

	void upload(int __size, T *ptr);

	void clear()
	{
		//fclose(_file);
		//_file = fopen(_file, "wb+");
		num = 0;
	}
};

template<class T>
T dataFile<T>::get(int i) const
{
	fseek(_file, (i - 1) * Tsize + sizeof(long), SEEK_SET);
	T it;
	fread(&it, Tsize, 1, _file);
	return it;
}

template<class T>
void dataFile<T>::replace(const T& now, int i)
{
	fseek(_file, (i - 1) * Tsize + sizeof(long), SEEK_SET);
	fwrite(&now, Tsize, 1, _file);
}

template<class T>
void dataFile<T>::push(const T& ele)
{
	fseek(_file, num * Tsize + sizeof(long), SEEK_SET);
	fwrite(&ele, Tsize, 1, _file);
	++num;
}

template<class T>
void dataFile<T>::loadAll(T* ptr)
{
	fseek(_file, sizeof(long), SEEK_SET);
	fread(ptr, Tsize, num * Tsize, _file);
}

template<class T>
void dataFile<T>::load(int l, int r, T *ptr) {
	fseek(_file, sizeof(long) + (l - 1) * Tsize, SEEK_SET);
	fread(ptr, Tsize, r - l + 1, _file);
}

template<class T>
void dataFile<T>::upload(int __size, T* ptr)
{
	num = __size;
	rewind(_file);
	fwrite(&num, sizeof(long), 1, _file);
	fwrite(ptr, Tsize, num, _file);
}

template<class T>
void dataFile<T>::pop()
{
	--num;
}