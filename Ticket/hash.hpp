#ifndef SJTU_HASH_TABLE
#define SJTU_HASH_TABLE
#include "exceptions.h"
template<class V>
class hash_table_t{
    using point = long;
    V list[512];
public:
    hash_table_t(V ini = V()){
        for (long i = 0;i < 512;i++) list[i] = ini;
    }
    ~hash_table_t(){}
    int get_value(const point code) const{
        return (code / 4096) % 512;
    }
    V find(const point code) const{
   // if (code > 3000000) throw runtime_error();
        return list[(code / 4096) % 512];
    }
    //even if U already exists, force to insert it.
    void insert(const point code, const V &v){
        list[(code / 4096) % 512] = v;
    }
};
#endif
