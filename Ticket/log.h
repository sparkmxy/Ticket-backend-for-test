#pragma once

#include "bplustree.hpp"
#include "tool.h"
#include "trainBase.h"
#include "exceptions.h"
#include "index.h"

struct keyInfo {
	int userID;
	char catalog;
	shortString trainID, from, to;
	date Date;

	keyInfo() = default;
	keyInfo(const int &u, const date &d, const char &c,
		const String &t, const String &_from, const String &_to)
		:userID(u), Date(d), catalog(c), trainID(t), from(_from), to(_to) {}
	
	keyInfo(const int &u, const date &d, const char &c)
		:userID(u), Date(d), catalog(c) {}

	bool operator < (const keyInfo &k) const {
		if (userID != k.userID) return userID < k.userID;
		if (Date!= k.Date) return Date < k.Date;
		if (catalog != k.catalog) return catalog < k.catalog;
		if (trainID != k.trainID) return trainID < k.trainID;
		if (from != k.from) return from < k.from;
		return to < k.to;
	}
};

struct Detail{
	static const int maxClassN = 5;
	Time leave, arrive;
	shortString seatClass[maxClassN];
	short classN,num[maxClassN];
	float price[maxClassN];

	Detail() = default;
	Detail(const train &T, const shortString &from,const shortString &to, const String &seatCls, const int &quantity) {
		classN = T.classN;
		for (int i = 0; i < classN; i++) 
			seatClass[i] = T.seatClass[i],num[i] = 0,price[i] = 0;
		num[T.getClassID(seatCls)] = quantity;
		int st = T.getStationID(from), ed = T.getStationID(to);
		leave = T.s[st].leave;
		arrive = T.s[ed].arrive;
		for (int i = st + 1; i <= ed; i++)
			for (int j = 0; j < classN; j++) price[j] += T.s[i].price[j];
	}
	void modify(const int &cls, const int &delta) {
		num[cls] += delta;
	}

	bool isZero() {
		for (int i = 0; i < classN; i++)
			if (num[i]) return false;
		return true;
	}
};

typedef std::pair<keyInfo, Detail> record;

class purchaseLog {
	Index<keyInfo, Detail> B;

	void merge(vector<record> &V, const vector<record> &U)const  {
		for (int i = 0; i < U.size(); i++) V.push_back(U[i]);
	}
public:
	purchaseLog() :B("log"){}

	void buy(const keyInfo &info, const train &T, const String &seatClass, const int &n);
	void refund(const keyInfo &info, const train &T, const String &seatClass, const int &n);
	std::pair<bool, Detail> findOrder(const keyInfo &key)const ;
	vector<record> query(const vector< std::pair<TYPE, String> > &V) const;
	void clear() {
		B.clear();
	}
};

std::ostream &operator << (std::ostream &os, const record &t);