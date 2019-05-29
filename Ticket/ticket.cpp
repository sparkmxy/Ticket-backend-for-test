#include "ticket.h"

std::ostream &operator << (std::ostream &os, const Seat &s) {
	os << s.type << ' ' << s.num + station::INITIAL_QUANTITY << ' ' << s.price;
	return os;
}

std::ostream &operator << (std::ostream &os, const ticket &t) {
	os << t.tID << ' ' << t.from << ' ' << t.Date << ' ' << t.leave << ' '
		<< t.to << ' ';
	os<<t.Date2<< ' ' << t.arrive << ' ';
	for (int i = 0; i < t.seat.size(); i++)
		os << t.seat[i] << ' ';
	return os;
}

bool cmpByFirstDim(const std::pair<shortString,shortString> &lhs, const std::pair<shortString,shortString>& rhs) {
	return lhs.first < rhs.first;
}

std::pair<bool,String> checkTransfer //返回是否构成中转方案和中转站
(const train &T1, const train &T2, const String &from, const String &to) {
	int x = T1.getStationID(from), y = T2.getStationID(to);
	for (int i = x + 1; i < T1.n; i++) {
		// 以T1.s[i]为中转站
	//	std::cout << "transfer check: " <<T1.s[i].name<< endl;
		int j = 0;
		while (j < y && (T2.s[j] != T1.s[i]||T1.s[i].arrive > T2.s[j].leave)) {
		//	std::cout << T2.s[j].name << endl;
			j++;
		}
		if (j < y) return std::make_pair(true,T1.s[i].name);
	}
	return std::make_pair(false,String(""));
}

void myMin(ticketPair &p1,const ticketPair &p2) {
	if (!p1.first.valid() ||
		p2.second.arrive - p2.first.leave < p1.second.arrive - p1.first.leave) p1 = p2;
}

void ticketSystem::add(const String &st, const String &id) {
#ifdef  DEBUGMODE
	std::cout << "add: Train #" << id << " will pass station " << st << endl;
#endif
	B.insert(std::make_pair(st,id),id);
}


vector<ticket> ticketSystem::query(const String &from, const String &to,
	const date &d,const String &catalog) {
	auto  V = B.listof(std::make_pair(from,String()), cmpByFirstDim);
	auto  U = B.listof(std::make_pair(to, String()), cmpByFirstDim);
#ifdef  DEBUGMODE
	std::cout << "------------------DEBUG-----------------------------------" << endl;
	std::cout << "Trains at " << from << endl;
	for (int i = 0; i < V.size(); i++) std::cout << V[i].second << endl;
	std::cout << "Trains at " << to << endl;
	for (int i = 0; i < V.size(); i++) std::cout << U[i].second<< endl;
	std::cout << "----------------------------------------------------------" << endl;
#endif //  DEBUG_MODE
	vector<String> C;
	int i = 0, j = 0;
	while (i < V.size()) {
		while (j < U.size() && U[j].second < V[i].second) j++;
		if (j == U.size()) break;
		if (V[i].second == U[j].second) 
			C.push_back(V[i].second);
		i++;
	}
	vector<ticket> ret;

	for (i = 0; i < C.size(); i++) {
		train t = TS->query(C[i]).second;
		if (catalog.contain(t.catalog) && t.ok(from, to))ret.push_back(ticket(t,from,to,d));
	}

	return ret;
}

ticketPair ticketSystem::transfer(const String &from, const String &to,
	const date &d,const String &catalog) {
	auto  V = B.listof(std::make_pair(from, String()), cmpByFirstDim);
	auto  U = B.listof(std::make_pair(to, String()), cmpByFirstDim);
#ifdef  DEBUGMODE
	std::cout << "------------------DEBUG-----------------------------------" << endl;
	std::cout << "Trains at " << from << endl;
	for (int i = 0; i < V.size(); i++) std::cout << V[i].second << endl;
	std::cout << "Trains at " << to << endl;
	for (int i = 0; i < U.size(); i++) std::cout << U[i].second << endl;
	std::cout << "----------------------------------------------------------" << endl;
#endif //  DEBUG_MODE
	ticketPair ret;
	for (int i = 0; i < V.size(); i++) {
		train T1 = TS->query(V[i].second).second;
		if (!catalog.contain(T1.catalog)) continue;
		for (int j = 0; j < U.size(); j++) {
			train T2 = TS->query(U[j].second).second;
			if (!catalog.contain(T2.catalog)) continue;
			//std::cout << "check: " << T1.ID << " " << T2.ID << endl;
				auto check = checkTransfer(T1, T2, from, to);
			if (check.first)
				myMin(ret,
					ticketPair(ticket(T1, from, check.second, d), ticket(T2, check.second, to, d))
				);
		}
	}
	return ret;
}