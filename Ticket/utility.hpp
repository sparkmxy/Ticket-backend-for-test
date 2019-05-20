#ifndef SJTU_UTILITY_HPP
#define SJTU_UTILITY_HPP

#include <utility>

template<class T1, class T2>
class pair {
public:
	T1 first;
	T2 second;
	constexpr pair() : first(), second() {}
	pair(const pair &other) = default;
	pair(pair &&other) = default;
	pair(const T1 &x, const T2 &y) : first(x), second(y) {}
	template<class U1, class U2>
	pair(U1 &&x, U2 &&y) : first(x), second(y) {}
	template<class U1, class U2>
	pair(const pair<U1, U2> &other) : first(other.first), second(other.second) {}
	template<class U1, class U2>
	pair(pair<U1, U2> &&other) : first(other.first), second(other.second) {}
	pair operator=(const pair &other){
		first = other.first;
		second = other.second;
		return *this;
	}
};
//node of a list
template<typename TYPE>
struct ut_list_node{
	TYPE* prev;
	TYPE* next;
    ut_list_node(TYPE* p = nullptr, TYPE* n = nullptr):prev(p), next(n){}
};
//head of a list
template<typename TYPE>
struct ut_list_head{
	int count;
	TYPE* start;
	TYPE* end;
    ut_list_head(TYPE* p = nullptr, TYPE* n = nullptr, int c = 0):
        start(p), end(n), count(c){}
};
#endif
