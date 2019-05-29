#pragma once

//#define DEBUGMODE
#include <string>
#include <iostream>
#include "vector.hpp"
#include "exceptions.h"
#include <sstream>
#include <algorithm>
#include <iomanip>

#define RMB_SYMBOL -17
#define RMB_LEGNTH 3

struct Time;
struct date;
using sjtu::vector;
using std::endl;
using std::min;
using std::setw;
using std::setfill;
class String;

enum TYPE{STRING,_DATE,TIME,_INT,_DOUBLE};

typedef std::pair<TYPE, String> token;
class shortString;

class String {
	friend class shortString;
	static const int SIZE = 40;
	friend std::ostream &operator << (std::ostream &os, const String &str);
	char s[SIZE+1];
	char l;

	int cmp(const String &a) const {
		if (l != a.l) return l < a.l ? -1 : 1;
		return strcmp(s, a.s);
	}
public:
	String() :l(0) {}
	String(std::string str) {
		l = str.length();
		if (l > SIZE) throw std::string("Name is too long.");
		for (int i = 0; i < l; i++) s[i] = str[i];
		s[l] = '\0';
	}
	String(const char *c) {
		l = strlen(c);
		for (int i = 0; i < l; i++) s[i] = c[i];
		s[l] = '\0';
	}
	String(const String &other) {
		l = other.l;
		for (int i = 0; i <= l; i++) s[i] = other.s[i];
	}
	String &operator = (const String &str) {
		l = str.l;
		for (int i = 0; i < l; i++) s[i] = str.s[i];
		s[l] = '\0';
		return *this;
	}

	bool operator < (const String &a) const { return cmp(a) < 0; }
	bool operator == (const String &a) const { return cmp(a) == 0; }
	bool operator != (const String &a) const { return cmp(a) != 0; }
	bool operator > (const String &a) const { return cmp(a) > 0; }
	const char &operator [] (const int &idx) const {
		return s[idx];
	}
	char &operator [] (const int &idx){
		return s[idx];
	}
	int length() const { return l; }
	int asint() const ;
	float asfloat() const;
	date asdate() const;
	Time asTime() const;
	char aschar()const;
	bool contain(const char &c)const;
};

class shortString {
	static const int SIZE = 20;
	friend std::ostream &operator << (std::ostream &os, const shortString &str);
	char s[SIZE + 1];
	char l;

	int cmp(const shortString &a) const {
		if (l != a.l) return l < a.l ? -1 : 1;
		return strcmp(s, a.s);
	}

public:
	shortString() = default;
	shortString(const String &str):l(str.l){
		for (int i = 0; i < l; i++) s[i] = str.s[i];
		s[l] = '\0';
	}

	shortString(const char *c) {
		l = strlen(c);
		for (int i = 0; i < l; i++) s[i] = c[i];
		s[l] = '\0';
	}

	operator String() const {
		return String(s);
	}
	bool operator < (const shortString &a) const { return cmp(a) < 0; }
	bool operator == (const shortString &a) const { return cmp(a) == 0; }
	bool operator != (const shortString &a) const { return cmp(a) != 0; }
	bool operator > (const shortString &a) const { return cmp(a) > 0; }
};

struct Time {
	friend std::ostream &operator << (std::ostream &os, const String &Time);
	short t;
	Time(int h = 0, int m = 0) :t(h*60+m){}
	bool operator < (const Time &rhs) const{
		return t < rhs.t;
	}
	bool operator > (const Time &rhs) const {
		return t > rhs.t;
	}
	bool operator == (const Time &rhs) const {
		return t == rhs.t;
	}

	Time operator - (const Time &rhs) const {
		Time ret;
		ret.t = t - rhs.t;
		return ret;
	}
};

struct date {
	friend std::ostream &operator << (std::ostream &os, const String &date);
	short day;

	date() = default;
	date(int y, int m, int d) :day(d) {
		if (y != 2019 || m != 6) throw wrong_parameter();
	}
	date(int d) :day(d) {}


	bool operator < (const date &d) const {
		return day < d.day;
	}
	bool operator == (const date &d) const {
		return day == d.day;
	}
	bool operator != (const date &d) const {
		return !this->operator==(d);
	}

	int asint() {return day-1;}

	date tomorrow() const { return date(day + 1);}
};

std::ostream &operator << (std::ostream &os, const Time &t);

std::ostream &operator << (std::ostream &os, const date &d);

std::ostream &operator << (std::ostream &os, const String &str);

std::ostream &operator << (std::ostream &os, const shortString &str);
