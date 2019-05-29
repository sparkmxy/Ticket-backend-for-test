#define _CRT_SECURE_NO_WARNINGS
//#define LOCAL_TEST

#include <iostream>
#include "parser.hpp"
#include "bookingSystem.h"

ticketBookingSystem S;

string getToken(const string &str, int t) {
	int l = str.length(), cnt = 0, st = 0;
	for(int i=0;i<l;i++)
		if (str[i] == ' ') {
			cnt++;
			if (cnt == t) return str.substr(st,i-st);
			else if (cnt == t - 1) st = i + 1;
		}
	return str;
}

int main() {
#ifdef LOCAL_TEST
	freopen("2s.in", "r", stdin);
	freopen("myans.txt", "w", stdout);
	S.process("clean", vector<token>());
#endif // LOCAL_TEST
	std::ios_base::sync_with_stdio(0);
	//std::cout.precision(10);	
	std::string str,s;
	//std::cout << sizeof(keyInfo) << " " << sizeof(Detail) << endl;
	while (true) {
		std::getline(std::cin,str);
		s = getToken(str, 1);
		if (s == "add_train" || s == "modify_train") {
			s = getToken(str,5);
			int n = 0;
			for (int i = 0; i < s.length(); i++) n = n * 10 + s[i] - '0';
			while (n--) {
				std::getline(std::cin, s);
				str = str + '\n' + s;
			}
		}
		EXECUTOR(S,str);
	}
	return 0;
}
