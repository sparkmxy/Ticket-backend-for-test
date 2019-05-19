#include "train.h"


std::ostream &operator << (std::ostream &os, const train &t) {
	os << t.ID << ' ' << t.name << ' ' << t.catalog << ' '
		<< t.n << ' ' << t.classN << ' ';
	for (int i = 0; i < t.classN; i++) os << t.seatClass[i] << ' ';
	os << '\n';
	for (int i = 0; i < t.n; i++) {
		os << t.s[i].name << ' ' << t.s[i].arrive << ' ' << t.s[i].leave << ' ' << t.s[i].stop << ' ';
		for (int j = 0; j < t.classN; j++)
			os << "￥" << t.s[i].price[j] << ' ';
		if (i + 1 != t.n)os << '\n';
	}
	return os;
}

int train::getStationID(const String &target)const {
	int i = 0;
	while (i < n && s[i].name != target) i++;
	return i == n ? -1 : i;
}

int train::getClassID(const String &cls) const {
	int i = 0;
	while (i < classN && seatClass[i] != cls) i++;
	return i == classN ? -1 : i;
}

int train::getDay2(const int &from) const {
	int ret = 0;
	if (from > 0 && s[1].arrive < s[0].leave) ret = 1;
	for (int i = 2; i <= from; i++)
		if (s[i].arrive < s[i - 1].arrive) ret++;
	//std::cout << "from = " << from << " extra Day = " << ret << endl;
	return ret;
}

int train::getDay(const int &from) const {
	int ret = 0;
	for (int i = 1; i <= from; i++)
		if (s[i].leave < s[i - 1].leave) ret++;
	//std::cout << "from = " << from << " extra Day = " << ret << endl;
	return ret;
}

bool train::ok(const String &from, const String &to)const {
	return getStationID(from) < getStationID(to);
}


bool trainSystem::add(const String &id, const String &name, const String &catalog,
	const vector<String> &classes, const vector<station> &V) {
	//std::cout << "add: " << id << endl;
	return B.insert(id,train(id,name,catalog,classes,V));
}

bool trainSystem::sale(const String &id) {
	auto result = B.find(id);
	if (!result.first) return false;
	if (result.second.onsale) return false;
	result.second.onsale = true;
	B.set(id, result.second);
	return true;
}

std::pair<bool, train> trainSystem::query(const String &id) {
	return B.find(id);
}

bool trainSystem::remove(const String &id) {
	return B.remove(id);
}

bool trainSystem::modify(const String &id, const String &name, const String &catalog,
	const vector<String> &classes, const vector<station> &V) {
	auto result = B.find(id);
	if (!result.first || result.second.onsale) return false;
	B.set(id,train(id,name,catalog,classes,V));
	return true;
}

bool trainSystem::modifyTicket(purchaseLog *log, const vector<token> &V,int f) {
	auto result0 = B.find(V[2].second);
	if (!result0.first) return false;
	train &t = result0.second;
	if (!t.onsale) return false;
	int c = t.getClassID(V[6].second);
	if (c == -1) return false;
	int user = V[0].second.asint();
	int st = t.getStationID(V[3].second), ed = t.getStationID(V[4].second);
	date Date = V[5].second.asdate();
	//Date.day -= t.getDay(st);
	//std::cout<<"buy : "<<Date<<endl;
	int d = Date.asint();
#ifdef DEBUGMODE
	std::cout << "-------------------------------DEBUG-----------------------\n";
	std::cout << (f == -1 ? "buy: " : "refund: ");
	std::cout << "userID = " << user << "class: " << c << " train: " << t.ID << endl;
	std::cout << "-----------------------------------------------------------\n";
#endif

	int delta = V[1].second.asint();
	if (f == 1) {
		auto result = log->findOrder(keyInfo(user, Date, t.catalog,
			t.ID, V[3].second, V[4].second));
		if (!result.first) return false;
		if (result.second.num[c] < delta) return false;
	}
	else {
		for (int i = st+1; i <= ed; i++)
			if (t.s[i].num[d][c] + station::INITIAL_QUANTITY < delta) {
				//std::cout << "ticket is not enough.\n";
				return false;
			}
	}

	for (int i = st + 1; i <= ed; i++) {
		t.s[i].num[d][c] += f * delta;
		//std::cout << "after: " << t.s[i].num[d][c] << endl;
	}
	B.set(t.ID, t);
	if (f == -1)
		log->buy(keyInfo(user, Date, t.catalog, t.ID, V[3].second, V[4].second), t, V[6].second, delta);
	else
		log->refund(keyInfo(user, Date, t.catalog, t.ID, V[3].second, V[4].second), t, V[6].second, delta);
	return true;
}
