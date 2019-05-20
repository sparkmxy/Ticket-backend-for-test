#include "tool.h"

std::ostream &operator << (std::ostream &os,const Time &time){
	if (time.t < 0) os << "xx:xx";
	else os << setw(2) << setfill('0') << time.t / 60
		<< ':' << setw(2) << setfill('0') << time.t % 60;
	return os;
}

std::ostream &operator << (std::ostream &os, const date &d) {
	if (d.day > 30)
		os << "2019-07-" << setw(2) << setfill('0') << d.day - 30;
	else os << "2019-06-" << setw(2) << setfill('0') << d.day;
	return os;
}

std::ostream &operator << (std::ostream &os, const String &str) {
	os<<str.s;
	return os;
}

std::ostream &operator << (std::ostream &os, const shortString &str) {
	os << str.s;
	return os;
}

int String::asint()const {
	int x = s[0]-'0';
	for (int i = 1; i < l; i++) x = x * 10 + s[i] - '0';
	return x;
}

float String::asfloat()const {
	int i = (int)s[0] == RMB_SYMBOL ? RMB_LEGNTH : 0;
	float x = 0, t = 0.1;
	for (; i < l && s[i] != '.'; i++) x = x * 10 + s[i] - '0';
	for (i++; i < l; i++, t *= 0.1) x += t * (s[i] - '0');
	return x;
}

date String::asdate() const {
	if (l != 10) throw wrong_parameter();
	return date(2019, (s[5] - '0') * 10 + s[6] - '0', (s[8] - '0') * 10 + s[9] - '0');
}

Time String::asTime() const {
	if (l != 5) throw wrong_parameter();
	if (s[0] == 'x') return Time(0, -1);
	return Time((s[0] - '0') * 10 + s[1] - '0', (s[3] - '0') * 10 + s[4] - '0');
}

char String::aschar()const {
	if(l != 1) throw wrong_parameter();
	return s[0];
}

bool String::contain(const char &c)const {
	int i = 0;
	while (i < l && s[i] != c) i++;
	return i != l;
}