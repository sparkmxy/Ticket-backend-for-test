#pragma once

#include "dataFile.h"
#include "tool.h"
#include "index.h"

struct user {
	enum userType {COMMON=1,ADMIN};

	int id;
	userType type;
	shortString passwd, email, phone,name;

public:
	user() = default;
	user(const int &i, const String &n, const String &p,
		const String &e, const String &ph, const userType &t = COMMON)
		:id(i), name(n), passwd(p), email(e), phone(ph), type(t){}

	bool match(String _pswd) { return _pswd == passwd; }
	void reset(const String &n, const String &p, const String &e, const String &ph) {
		name = n; 
		passwd = p;
		email = e;
		phone = ph;
	}
};

class userSystem {
	dataFile<user> B;
	int currentID;
	static const int INITIAL_ID = 2019;
public:
	userSystem() :B("user"){}

	int add(const vector<token> &V);

	bool login(const int &id, const String &pswd) const;

	std::pair<bool, user> query(const int &id) const;

	bool modify(const vector<token> &V);

	bool modifyPrivilege(const int &master, const int &id, int p);

	void clear() {
		B.clear();
	}
};

std::ostream &operator << (std::ostream &os, const user &u);