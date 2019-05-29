#include "user.h"

std::ostream &operator << (std::ostream &os, const user &u) {
	os << u.name << ' ' << u.email << ' ' << u.phone << ' ' << static_cast<int>(u.type);
	return os;
}

int userSystem::add(const vector<token> &V) {
	if (V.size() != 4) return -1;
	for (int i = 0; i < 4; i++) if (V[i].first != STRING) return -1;
	/*
	此处应当加入更多判断，如邮箱是否重复等等
	*/
	if(B.size() == 0)
		B.push(user(currentID, V[0].second, V[1].second, V[2].second, V[3].second,user::ADMIN));
	else 
		B.push(user(currentID, V[0].second, V[1].second, V[2].second,V[3].second));
	return B.size() + INITIAL_ID - 1;
}

bool userSystem::login(const int &id, const String &pswd) const {
	//std::cout << "login: " << id << " " << pswd << endl;
	if (id - INITIAL_ID>= B.size()) return false;
	return B.get(id - INITIAL_ID + 1).match(pswd);
}

std::pair<bool, user> userSystem::query(const int &id) const {
	if (id - INITIAL_ID >= B.size()) return std::make_pair(false,user());
	return std::make_pair(true,B.get(id-INITIAL_ID+1));
}

bool userSystem::modify(const vector<token> &V) {
	if (V.size() != 5) return false;
	if (V[0].first != _INT) return false;
	for (int i = 1; i < 5; i++) if (V[i].first != STRING) return false;
	int id = V[0].second.asint();
	if (id - INITIAL_ID >= B.size()) return false;
	auto cur = B.get(id - INITIAL_ID + 1);
	cur.reset(V[1].second,V[2].second,V[3].second,V[4].second);
	//std::cout << cur << endl;
	B.replace(cur,id-INITIAL_ID+1);
	return true;
}


bool userSystem::modifyPrivilege(const int &admin, const int &id, int p) {
//	if (!B.count(admin) || !B.count(id)) return false;
	if (admin - INITIAL_ID >= B.size() || id - INITIAL_ID >= B.size()) return false;
	user _admin =B.get(admin-INITIAL_ID+1);
	if (_admin.type != user::ADMIN) return false;
	user u = B.get(id - INITIAL_ID + 1);
	if (u.type == user::ADMIN) return p == 2;
	if (u.type != p) {
		u.type = user::userType(p);
		B.replace(u, id - INITIAL_ID + 1);
	}
	return true;
}
