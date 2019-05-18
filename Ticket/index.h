#pragma once
#include "tool.h"
#include "bplustree.hpp"
#include "dataFile.h"

template<class key_type,class value_type>
class Index {
	bplustree<key_type,int,4096> B;
	dataFile<value_type> data;
	typedef std::pair<bool,value_type> result_type;
	typedef std::pair<key_type, value_type> record_type;
public:
	Index(std::string name) :data(name+"data"){
		B.init((name+"_index").c_str(),(name+"_alloc").c_str());
	}

	result_type find(const key_type &key) const {
		auto t = B.find(key);
		if (!t.first) return  std::make_pair(false, value_type());
		return std::make_pair(true,data.get(t.second));
	}
	
	bool remove(const key_type &key){
		return B.remove(key);
	}

	bool insert(const key_type &key, const value_type &val) {
		data.push(val);
		return B.insert(key, data.size());
	}

	vector<record_type> listof(const key_type &key, 
		bool(*cmp)(const key_type &a, const key_type &b)) const {
		auto V = B.listof(key, cmp);
		vector<record_type> ret;
		for (int i = 0; i < V.size(); i++) 
			ret.push_back(std::make_pair(V[i].first,data.get(V[i].second)));
		return ret;
	}

	bool set(const key_type &key, const value_type &val){
		auto t = B.find(key);
		if (!t.first) return false;
		data.replace(val, t.second);
	}
	
	int size() const { return data.size(); }

	void clear() {
		B.clear();
		data.clear();
	}
};
