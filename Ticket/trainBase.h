#pragma once

#include "tool.h"

static const int maxDate = 31;
static const int maxClassN = 5;

struct station {
	static const short INITIAL_QUANTITY = 2000;
	shortString name;
	Time arrive, leave, stop;
	short num[maxDate][maxClassN]; // 2018-06-01 到2018-06-30每一天的余票数
	float price[maxClassN];

	station() = default;

	station(const String &str, const Time &_arrive, const Time &Leave, const Time &_stop,
		const vector<float> &V) :name(str), arrive(_arrive), leave(Leave), stop(_stop) {
		for (int i = 0; i < V.size(); i++) price[i] = V[i];
	}

	void init() {
		memset(num, 0, sizeof(num));
	}

	bool operator == (const station &s) const { return name == s.name; }
	bool operator != (const station &s) const { return !this->operator==(s); }
};

struct train {
	static const int maxN = 50;
	short n; //经过站的数量
	short classN; //座位类别的数量
	String name;
	char catalog;
	shortString ID,seatClass[maxClassN];
	station s[maxN];
	bool onsale;

	train() = default;
	train(const String &id, const String &_name, const String &cata,
		const vector<String> &C, const vector<station> &V)
		:ID(id), name(_name), catalog(cata.aschar()), onsale(false) {
		classN = C.size();
		n = V.size();
		for (int i = 0; i < classN; i++) seatClass[i] = C[i];
		for (int i = 0; i < n; i++) {
			s[i] = V[i];
			s[i].init();
		}
	}

	int getDay(const int &from) const;
	int getDay2(const int &from) const;
	int getStationID(const String &target) const;
	int getClassID(const String &cls) const;
	bool ok(const String &from, const String &to)const;
};
