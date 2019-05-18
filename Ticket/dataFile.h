#pragma once
//基于文件实现的一个vector

#include <string>
#include <fstream>
#include <iostream>
#include "tool.h"

using std::fstream;

template<class T>
class dataFile {
	std::string fileName;
	std::ifstream in;
	std::fstream out;
	std::ifstream *ptr;
	int _size;
	const int sizeofT;
public:
	//构造函数：参数为文件名，如果不存在就创建
	dataFile(std::string name) :sizeofT(sizeof(T)),fileName(name) {
		in.open(fileName,fstream::binary);
		if (!in) {
			out.open(fileName,fstream::out| fstream::binary);
			_size = 0;
			out.seekp(0);
			out.write(reinterpret_cast<const char *>(&_size), sizeof(int));
			out.flush();
			in.open(fileName, fstream::binary);
		}
		else {
			out.open(fileName,fstream::in|fstream::out| fstream::binary);
			in.seekg(0);
			in.read(reinterpret_cast<char *>(&_size), sizeof(int));
			//std ::cout << "File "<<fileName<<"exist,size = " << _size << '\n';
		}
		ptr = &in;
	}
	dataFile() = delete;

	// 析构函数：关闭文件
	~dataFile() {
		out.seekp(0);
		out.write(reinterpret_cast<const char *>(&_size), sizeof(int));
		in.close();
		out.close();
	}
	// get: return the i th record
	T get(int i) const ; 

	// replace the i th record with 'now'
	void replace(const T& now, int i);

	// push in a new record
	void push(const T& ele);

	void pop();

	int size() const { return _size; }

	void loadAll(T *ptr);

	void load(int l, int r, T *ptr);

	void upload(int __size, T *ptr);

	void clear() { _size = 0; }
};

template<class T>
T dataFile<T>::get(int i) const {
	ptr->seekg((i - 1)*sizeofT + sizeof(int));
	T ret;
	ptr->read(reinterpret_cast<char *>(&ret), sizeofT);
	return ret;
}

template<class T>
void dataFile<T>::replace(const T& now, int i) {
	out.seekp((i - 1)*sizeofT + sizeof(int));
	out.write(reinterpret_cast<const char *>(&now), sizeofT);
	out.flush();
}

template<class T>
void dataFile<T>::push(const T& ele) {
	_size++;
	out.seekp((_size - 1)*sizeofT + sizeof(int));
	out.write(reinterpret_cast<const char *>(&ele), sizeofT);
	out.flush();
}

template<class T>
void dataFile<T>::loadAll(T* ptr) {
	in.seekg(sizeof(int));
	in.read(reinterpret_cast<char *>(ptr), _size*sizeofT);
}

template<class T>
void dataFile<T>::load(int l, int r, T *ptr) {
	in.seekg(sizeof(int) + (l - 1)*sizeofT);
	in.read(reinterpret_cast<char *>(ptr), (r - l + 1) * sizeof(T));
}

template<class T>
void dataFile<T>::upload(int __size,T* ptr) {
	out.seekp(0);
	_size = __size;
	out.write(reinterpret_cast<const char *>(&_size), sizeof(int));
	out.seekp(sizeof(int));
	out.write(reinterpret_cast<const char *>(ptr), _size*sizeofT);
	out.flush();
}

template<class T>
void dataFile<T>::pop() {
	_size--;
}