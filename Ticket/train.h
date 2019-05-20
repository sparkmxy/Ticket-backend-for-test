#pragma once
/*
车次相关的功能

*/
#include "bplustree.hpp"
#include "trainBase.h"
#include "tool.h"
#include "log.h"
#include "index.h"

class trainSystem {
	Index<shortString,train> B;
public:
	trainSystem() :B("train"){}

	bool add(const String &id, const String &name, const String &catalog,
		const vector<String> &classes, const vector<station> &V);

	bool sale(const String &id);

	std::pair<bool,train> query(const String &id);

	bool remove(const String &id);

	bool modify(const String &id, const String &name, const String &catalog,
		const vector<String> &classes, const vector<station> &V);

	bool modifyTicket(purchaseLog *log, const vector<token> &V,int f);

	void clear() {
		B.clear();
	}
};

std::ostream &operator << (std::ostream &os, const train &t);