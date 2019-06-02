#include "user.h"

std::ostream &operator << (std::ostream &os, const user &u) {
	os << u.name << ' ' << u.email << ' ' << u.phone << ' ' << static_cast<int>(u.type);
	return os;
}

int userSystem::add(const vector<token> &V) {
	if(B.size() == 0)
		B.insert(currentID,user(currentID, V[0].second, V[1].second, V[2].second, V[3].second,user::ADMIN));
	else 
		B.insert(currentID,user(currentID, V[0].second, V[1].second, V[2].second,V[3].second));
	return currentID++;
}

bool userSystem::login(const int &id, const String &pswd) const {
	//std::cout << "login: " << id << " " << pswd << endl;
	if (id >= currentID) return false;
	return B.find(id - INITIAL_ID + 1).second.match(pswd);
}

std::pair<bool, user> userSystem::query(const int &id) const {
	if (id >= currentID) return std::make_pair(false,user());
	return B.find(id-INITIAL_ID+1);
}

bool userSystem::modify(const vector<token> &V) {
	int id = V[0].second.asint();
	if (id >= currentID) return false;
	auto cur = B.find(id - INITIAL_ID + 1).second;
	cur.reset(V[1].second,V[2].second,V[3].second,V[4].second);
	//std::cout << cur << endl;
	B.set(id-INITIAL_ID+1,cur);
	return true;
}


bool userSystem::modifyPrivilege(const int &admin, const int &id, int p) {
//	if (!B.count(admin) || !B.count(id)) return false;
	if (admin >=currentID || id >= currentID) return false;
	user _admin =B.find(admin-INITIAL_ID+1).second;
	if (_admin.type != user::ADMIN) return false;
	user u = B.find(id - INITIAL_ID + 1).second;
	if (u.type == user::ADMIN) return p == 2;
	if (u.type != p) {
		u.type = user::userType(p);
		B.set(id - INITIAL_ID + 1,u);
	}
	return true;
}
