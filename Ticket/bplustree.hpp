#pragma warning(disable : 4996)

#ifndef SJTU_BPLUSTREE_HPP
#define SJTU_BPLUSTREE_HPP
//#define OUTPUT_INIT

#include <cstddef>
#include <functional>
#include <stdio.h>
#include <cstring>
#include "exceptions.h"
#include "alloc.hpp"
#include "vector.hpp"
#include "tool.h"

const point invalid_p = 0xdeadbeef;
template<class key_t,
         class value_type, 
         size_t node_size = 4096, 
         class Compare = std::less<key_t>
>   class bplustree{
    using point     =   long;
    using byte      =   char;
    using list_type =   std::pair<key_t, value_type>;
    using find_type =   std::pair<bool, value_type>;
    struct node{
        key_t       key;
        point       prior, next;
        point       pos;
        size_t      size;                    //the size of its brothers
        bool        type;                    //0 for a leaf and 1 otherwise
        node(key_t k = key_t(),
        point p = invalid_p, size_t s = 1, bool ty = 0, 
        point pre = invalid_p, point nex = invalid_p)
        :key(k),pos(p),prior(pre),next(nex),size(s),type(ty){}
    };
    node                    root;
    Compare                 com;
    size_t                  num;
    FILE                    *datafile;
	const size_t            part_size_l, part_size_n;
    ALLOC<node_size>        alloc;
    point                   root_pos;
    char                    *index_name, *data_name;

    inline bool equal(const key_t& k1,const key_t& k2) const{
        return !(com(k1, k2) || com(k2, k1));
    }
    /*
        * basic i/o functions for leaf and nonleaf node
    */
    inline void load_cache_n(byte *start,const node& p) const{
        fseek(datafile, p.pos + sizeof(node), SEEK_SET);
        fread(start, 1, (sizeof(key_t) + sizeof(point)) * p.size, datafile);
		#ifdef DEBUG_MODE
		printf("load_cache_seek: %d\n", p.pos + sizeof(node));
		printf("which are:\n");
        for (int i = 0;i < p.size;i++)
        #ifdef TEST_INT_MODE
            printf("key: %d, point: %d; ", nth_key_n(start, i)->a, *nth_point(start, i));
        #endif
        #ifdef TEST_STRING_MODE
            printf("key: %s, point: %d; ", nth_key_n(start, i)->inf, nth_point(start, i)->inf);
        #endif
        printf("\n");
		#endif
    }
    inline void load_cache_l(byte *start,const node& p) const{
        fseek(datafile, p.pos + sizeof(node), SEEK_SET);
        fread(start, 1, (sizeof(key_t) + sizeof(value_type)) * p.size, datafile);
		#ifdef DEBUG_MODE
		printf("load_cache_seek: %d\n", p.pos + sizeof(node));
		printf("which are:\n");
        for (int i = 0;i < p.size;i++)
        #ifdef TEST_INT_MODE
            printf("key: %d, value: %d; ", nth_key_l(start, i)->a, *nth_value(start, i));
        #endif
        #ifdef TEST_STRING_MODE
            printf("key: %s, value: %s; ", nth_key_l(start, i)->inf, nth_value(start, i)->inf);
        #endif
        printf("\n");
        #endif
    }
    inline void save_cache_n(byte *start,const node &p) const{
        fseek(datafile, p.pos + sizeof(node), SEEK_SET);
        fwrite(start, 1, (sizeof(key_t) + sizeof(point)) * p.size, datafile);
		#ifdef DEBUG_MODE
		printf("save_cache_seek: %d\n", p.pos + sizeof(node));
        printf("which are:\n");
        for (int i = 0;i < p.size;i++)
        #ifdef TEST_INT_MODE
            printf("key: %d, point: %d; ", nth_key_n(start, i)->a, *nth_point(start, i));
        #endif
        #ifdef TEST_STRING_MODE
            printf("key: %s, value: %d; ", nth_key_n(start, i)->inf, nth_point(start, i)->inf);
        #endif
        printf("\n");
		#endif
    }
    inline void save_cache_l(byte *start,const node &p) const{
        fseek(datafile, p.pos + sizeof(node), SEEK_SET);
        fwrite(start, 1, (sizeof(key_t) + sizeof(value_type)) * p.size, datafile);
		#ifdef DEBUG_MODE
		printf("save_cache_seek: %d\n", p.pos + sizeof(node));
        printf("which are:\n");
        for (int i = 0;i < p.size;i++)
        #ifdef TEST_INT_MODE
            printf("key: %d, value: %d; ", nth_key_l(start, i)->a, *nth_value(start, i));
        #endif
        #ifdef TEST_STRING_MODE
            printf("key: %s, value: %s; ", nth_key_l(start, i)->inf, nth_value(start, i)->inf);
        #endif
        printf("\n");
		#endif
    }
    inline node load_node(point l)const{
        fseek(datafile, l, SEEK_SET);
        node tmp;
        fread(&tmp, sizeof(node), 1, datafile);
		#ifdef DEBUG_MODE
		printf("load_node_seek: %d\n", l);
        #ifdef TEST_INT_MODE
        printf("which is: pos:%d size:%d key:%d\n", tmp.pos, tmp.size, tmp.key.a);
		#endif
        #ifdef TEST_STRING_MODE
        printf("which is: pos:%d size:%d key:%s\n", tmp.pos, tmp.size, tmp.key.inf);
        #endif
        #endif
        return tmp;
    }
    inline bool save_node(const node &p)const{
        if (p.pos == invalid_p) return false;
        fseek(datafile, p.pos, SEEK_SET);
        fwrite(&p, sizeof(node), 1, datafile);
		#ifdef DEBUG_MODE
		printf("save_node_seek: %d\n", p.pos);
        #ifdef TEST_INT_MODE
        printf("which is: pos:%d size:%d key:%d\n", p.pos, p.size, p.key.a);
		#endif
        #ifdef TEST_STRING_MODE
        printf("which is: pos:%d size:%d key:%s\n", p.pos, p.size, p.key.inf);
        #endif
		#endif
		return true;
    }
    /*
        * basic function in order to get particular info of a node
    */
    inline key_t* nth_key_n(byte *start, size_t n = 0)const{
        return (key_t *)(start + (sizeof(key_t) + sizeof(point)) * n);
    }
    inline key_t* nth_key_l(byte *start, size_t n = 0)const{
        return (key_t *)(start + (sizeof(key_t) + sizeof(value_type)) * n);
    }
    inline point* nth_point(byte *start, size_t n = 0)const{
        return (point *)(start + sizeof(key_t) * (n + 1) + sizeof(point) * n);
    }
    inline value_type* nth_value(byte *start, size_t n = 0)const{
        return (value_type *)(start + sizeof(key_t) * (n + 1) + sizeof(value_type) * n);
    }
    inline point nth_value_loc(const node &now, size_t n = 0)const{
        return now.pos + sizeof(node) + (sizeof(key_t) + sizeof(value_type)) * n + sizeof(key_t);
    }
    inline value_type get_value(point loc)const{
        value_type v;
        fseek(datafile, loc, SEEK_SET);
        fread(&v, sizeof(value_type), 1, datafile);
        return v;
    }
    /*
        * binary search function to help find a particular child
    */
    inline size_t b_search_n(byte *start,const key_t& k, size_t n)const{
        size_t l = 0, r = n, mid;
        while (l < r){
            mid = (l + r) >> 1;
            if ( com(*nth_key_n(start, mid), k) ) l = mid + 1;
            else r = mid;
        }
        if (l >= n) return n - 1;
        else if (equal(*nth_key_n(start, l), k)) return l;
        else return l - 1;
    }
    inline size_t b_search_l(byte *start,const key_t& k, size_t n)const{
        size_t l = 0, r = n, mid;
        while (l < r){
            mid = (l + r) >> 1;
            if ( com(*nth_key_l(start, mid), k) ) l = mid + 1;
            else r = mid;
        }
        if (l >= n) return n - 1;
        else if (equal(*nth_key_l(start, l), k)) return l;
        else return l - 1;
    }
    /*
        * find a particular key and return its pointer
    */
    point _find(const node &p,const key_t& k)const{
        size_t ord;
        point tmp;
        if (!p.type){
            byte cache[node_size];
            load_cache_l(cache, p);
            ord = b_search_l(cache, k, p.size);
            tmp = nth_value_loc(p, ord);
            if (equal(*nth_key_l(cache, ord), k)) return tmp;
            else return invalid_p;
        }
        else{
            byte cache[node_size];
            load_cache_n(cache, p);
            ord = b_search_n(cache, k, p.size);
            tmp = *nth_point(cache, ord);
            return _find(load_node(tmp), k);
        }
    }
    /*
        * return a continuous segment of key-value sets
    */
    vector<list_type> _listof(const node &p, const key_t &k, 
                            bool (*comp)(const key_t &a, const key_t &b)) const{
        size_t ord;
        point tmp;
        if (!p.type){
            byte cache[node_size];
            load_cache_l(cache, p);
            if (com(k, p.key)) ord = 0;
            else ord = b_search_l(cache, k, p.size);
            tmp = nth_value_loc(p, ord);
            vector<list_type> ret;
            node now = p;
            while ( !comp(k, *nth_key_l(cache, ord)) ){
                if (!comp(*nth_key_l(cache, ord), k) )
                ret.push_back(
                    list_type(*nth_key_l(cache, ord), 
                        *nth_value(cache, ord)
                    )
                );
                if (ord + 1 < now.size) ++ord;
                else{
                    if (now.next == invalid_p) break;
                    now = load_node(now.next);
                    load_cache_l(cache, now);
                    ord = 0;
                }
            }
            return ret;
        }
        else {
            byte cache[node_size];
            load_cache_n(cache, p);
            if (com(k, p.key)) ord = 0;
            else ord = b_search_n(cache, k, p.size);
            tmp = *nth_point(cache, ord);
            return _listof(load_node(tmp), k, comp);
        }
    }
    /*
        * Make the first of p become the last of l,
        * while the size of p equals to part_size and that of l is less. 
        * Save them both
    */
    inline void Lbalance_n(node &p, node &l, byte *cache_p){
        byte cache_l[node_size];
        load_cache_n(cache_l, l);
        size_t mov = p.size - ((p.size + l.size) >> 1);
        for (int i = 0;i < mov;i++){
            *nth_key_n(cache_l, l.size + i) = *nth_key_n(cache_p, i);
            *nth_point(cache_l, l.size + i) = *nth_point(cache_p, i);
        }
        l.size += mov, p.size -= mov;
        p.key = *nth_key_n(cache_p, mov);
        save_cache_n(cache_l, l);
        save_cache_n((byte *)nth_key_n(cache_p, mov), p);
        save_node(l), save_node(p);
    }
    inline void Lbalance_l(node &p, node &l, byte *cache_p){
        byte cache_l[node_size];
        load_cache_l(cache_l, l);
        size_t mov = p.size - ((p.size + l.size) >> 1);
        for (int i = 0;i < mov;i++){
            *nth_key_l(cache_l, l.size + i) = *nth_key_l(cache_p, i);
            *nth_value(cache_l, l.size + i) = *nth_value(cache_p, i);
        }
        l.size += mov, p.size -= mov;
        p.key = *nth_key_l(cache_p, mov);
        save_cache_l(cache_l, l);
        save_cache_l((byte *)nth_key_l(cache_p, mov), p);
        save_node(l), save_node(p);
    }
    /*
        * Make the last of p become the first of r,
        * while the size of p equals to part_size and that of r is less. 
        * Save them both.
    */
    inline void Rbalance_n(node &p, node &r, byte *cache_p){
        byte cache_r[node_size];
        load_cache_n(cache_r, r);
        size_t mov = p.size - ((p.size + r.size) >> 1);
        for (size_t i = r.size + mov - 1;i > mov - 1;i--){
            *nth_key_n(cache_r, i)  = *nth_key_n(cache_r, i - mov);
            *nth_point(cache_r, i)  = *nth_point(cache_r, i - mov);
        }
        r.size += mov, p.size -= mov;
        for (size_t i = 0;i < mov;i++){
            *nth_key_n(cache_r, i)  = *nth_key_n(cache_p, p.size + i);
            *nth_point(cache_r, i)  = *nth_point(cache_p, p.size + i);
        }
        r.key = *nth_key_n(cache_r, 0);
        save_cache_n(cache_r, r);
        save_cache_n(cache_p, p);
        save_node(r), save_node(p);
    }
    inline void Rbalance_l(node &p, node &r, byte *cache_p){
        byte cache_r[node_size];
        load_cache_l(cache_r, r);
        size_t mov = p.size - ((p.size + r.size) >> 1);
        for (size_t i = r.size + mov - 1;i > mov - 1;i--){
            *nth_key_l(cache_r, i)  = *nth_key_l(cache_r, i - mov);
            *nth_value(cache_r, i)  = *nth_value(cache_r, i - mov);
        }
        r.size += mov, p.size -= mov;
        for (size_t i = 0;i < mov;i++){
            *nth_key_l(cache_r, i)  = *nth_key_l(cache_p, p.size + i);
            *nth_value(cache_r, i)  = *nth_value(cache_p, p.size + i);
        }
        r.key = *nth_key_l(cache_r, 0);
        save_cache_l(cache_r, r);
        save_cache_l(cache_p, p);
        save_node(r), save_node(p);
    }
    /*
        * Receive a node from the left part if avaliable
        * Save them both
    */
    inline void Lreceive_n(node &p, node &l, byte *cache_p){
        byte cache_l[node_size];
        load_cache_n(cache_l, l);
        for (size_t i = p.size;i > 0;i--){
            *nth_key_n(cache_p, i)  = *nth_key_n(cache_p, i - 1);
            *nth_point(cache_p, i)  = *nth_point(cache_p, i - 1);
        }
        ++p.size;--l.size;
        p.key = *nth_key_n(cache_p, 0)  = *nth_key_n(cache_l, l.size);
        *nth_point(cache_p)             = *nth_point(cache_l, l.size);
        save_cache_n(cache_l, l);
        save_cache_n(cache_p, p);
        save_node(l), save_node(p);
    }
    inline void Lreceive_l(node &p, node &l, byte *cache_p){
        byte cache_l[node_size];
        load_cache_l(cache_l, l);
        for (size_t i = p.size;i > 0;i--){
            *nth_key_l(cache_p, i) = *nth_key_l(cache_p, i - 1);
            *nth_value(cache_p, i) = *nth_value(cache_p, i - 1);
        }
        ++p.size;--l.size;
        p.key = *nth_key_l(cache_p, 0)  = *nth_key_l(cache_l, l.size);
        *nth_value(cache_p)             = *nth_value(cache_l, l.size);
        save_cache_l(cache_l, l);
        save_cache_l(cache_p, p);
        save_node(l), save_node(p);
    }
    /*
        * Receive a node from the right part if avaliable. 
        * Save them both
    */
    inline void Rreceive_n(node &p, node &r, byte *cache_p){
        byte cache_r[node_size];
        load_cache_n(cache_r, r);
        *nth_key_n(cache_p, p.size) = *nth_key_n(cache_r);
        *nth_point(cache_p, p.size) = *nth_point(cache_r);
        ++p.size, --r.size;
        r.key = *nth_key_n(cache_r, 1);
        save_cache_n((byte *)nth_key_n(cache_r, 1), r);
        save_cache_n(cache_p, p);
        save_node(r), save_node(p);
    }
    inline void Rreceive_l(node &p, node &r, byte *cache_p){
        byte cache_r[node_size];
        load_cache_l(cache_r, r);
        *nth_key_l(cache_p, p.size) = *nth_key_l(cache_r, 0);
        *nth_value(cache_p, p.size) = *nth_value(cache_r);
        ++p.size, --r.size;
        r.key = *nth_key_l(cache_r, 1);
        save_cache_l((byte *)nth_key_l(cache_r, 1), r);
        save_cache_l(cache_p, p);
        save_node(r), save_node(p);
    }
    /*
        * Find the quickest way to solve the problem of extreme size.
        * save now and cache and left/right and its cache if necessary, 
        * but do not save the cache of the parent
    */
    inline void deal_surplus_n(node &now, node &par, byte *cache, byte *cache_par, size_t ord){
        node tmp;
        if (now.pos != *nth_point(cache_par)){
            tmp = load_node(now.prior);
            if (tmp.size < part_size_n * 3 / 4) {
                Lbalance_n(now, tmp, cache);
                *nth_key_n(cache_par, ord) = now.key;
                return;
            }
        }
        if (now.pos != *nth_point(cache_par, par.size - 1)){
            tmp = load_node(now.next);
            if (tmp.size < part_size_n * 3 / 4){
                Rbalance_n(now, tmp, cache);
                *nth_key_n(cache_par, ord + 1) = tmp.key;
                return;
            }
            else split_n(now, cache, par, cache_par, ord);
        }
        else split_n(now, cache, par, cache_par, ord);
    }
    inline void deal_surplus_l(node &now, node &par, byte *cache, byte *cache_par, size_t ord){
        node tmp;
        if (now.pos != *nth_point(cache_par)){
            tmp = load_node(now.prior);
            if (tmp.size < part_size_l * 3 / 4) {
                Lbalance_l(now, tmp, cache);
                *nth_key_n(cache_par, ord) = now.key;
                return;
            }
        }
        if (now.pos != *nth_point(cache_par, par.size - 1)){
            tmp = load_node(now.next);
            if (tmp.size < part_size_l * 3 / 4){
                Rbalance_l(now, tmp, cache);
                *nth_key_n(cache_par, ord + 1) = tmp.key;
                return;
            }
            else split_l(now, cache, par, cache_par, ord);
        }
        else split_l(now, cache, par, cache_par, ord);
    }
    inline void deal_deficit_n(node &now, node &par, byte *cache,byte *cache_par, size_t ord){
        node tmp;
        if (now.pos != *nth_point(cache_par, par.size - 1)){
            tmp = load_node(now.next);
            if (tmp.size > (part_size_n >> 1)){
                Rreceive_n(now, tmp, cache);
                *nth_key_n(cache_par, ord + 1) = tmp.key;
                return;
            }
            else merge_n(now, cache, par, cache_par, tmp, ord);
            return;
        }
        if (now.pos != *nth_point(cache_par)){
            tmp = load_node(now.prior);
            if (tmp.size > (part_size_n >> 1)){
                Lreceive_n(now, tmp, cache);
                *nth_key_n(cache_par, ord) = now.key;
                return;
            }
            else{
                tmp.next = now.next;
                if (tmp.next != invalid_p){
                    node temp = load_node(tmp.next);
                    temp.prior = tmp.pos;
                    save_node(temp);
                }
                fseek(datafile, tmp.pos + sizeof(node) + (sizeof(key_t) + sizeof(point)) * tmp.size, SEEK_SET);
                fwrite(cache, 1, (sizeof(key_t) + sizeof(point)) * now.size, datafile);
                tmp.size += now.size;
                alloc.free(now.pos, node_size);
                --par.size;
                for (size_t i = ord;i < par.size;i++){
                    *nth_key_n(cache_par, i)    = *nth_key_n(cache_par, i + 1);
                    *nth_point(cache_par, i)    = *nth_point(cache_par, i + 1);
                }
                save_node(tmp);
            }
        }
    }
    inline void deal_deficit_l(node &now, node &par, byte *cache,byte *cache_par, size_t ord){
        node tmp;
        if (now.pos != *nth_point(cache_par, par.size - 1)){
            tmp = load_node(now.next);
            if (tmp.size > (part_size_l >> 1)){
                Rreceive_l(now, tmp, cache);
                *nth_key_n(cache_par, ord + 1) = tmp.key;
                return;
            }
            else merge_l(now, cache, par, cache_par, tmp, ord);
            return;
        }
        if (now.pos != *nth_point(cache_par)){
            tmp = load_node(now.prior);
            if (tmp.size > (part_size_l >> 1)){
                Lreceive_l(now, tmp, cache);
                *nth_key_n(cache_par, ord) = now.key;
                return;
            }
            else{
                tmp.next = now.next;
                if (tmp.next != invalid_p){
                    node temp = load_node(tmp.next);
                    temp.prior = tmp.pos;
                    save_node(temp);
                }
                fseek(datafile, tmp.pos + sizeof(node) + (sizeof(key_t) + sizeof(value_type)) * tmp.size, SEEK_SET);
                fwrite(cache, 1, (sizeof(key_t) + sizeof(value_type)) * now.size, datafile);
                tmp.size += now.size;
                alloc.free(now.pos, node_size);
                --par.size;
                for (size_t i = ord;i < par.size;i++){
                    *nth_key_n(cache_par, i)    = *nth_key_n(cache_par, i + 1);
                    *nth_value(cache_par, i)    = *nth_value(cache_par, i + 1);
                }
                save_node(tmp);
            }
            return;
        }
    }
    /*
        * Split the cache belonging to node now. 
        * save now and its cache but do not save any info of parent into storage,
        * though we have changed it in memory
    */
    void split_n(node &now, byte *cache, node &par, byte *cache_par, size_t order){
        size_t s = now.size >> 1;
        now.size -= s;
        size_t ns = now.size;
        point pos = alloc.alloc(node_size);
        // printf("ask_for_bpt: %d where %d\n", non_leaf_size, pos);
        node tmp = node(*nth_key_n(cache, ns), pos, s, now.type, now.pos, now.next);
        if (now.next != invalid_p){
            node temp = load_node(now.next);
            temp.prior = tmp.pos;
            save_node(temp);
        }
        now.next = tmp.pos;
        byte *cache_tmp = (byte *)nth_key_n(cache, ns);
        save_cache_n(cache_tmp, tmp);
        save_cache_n(cache, now);
        ns = order;
        for (size_t i = par.size;i > ns + 1;i--){
            *nth_key_n(cache_par, i)    = *nth_key_n(cache_par, i - 1);
            *nth_point(cache_par, i)    = *nth_point(cache_par, i - 1);
        }
        *nth_key_n(cache_par, ns + 1) = tmp.key;
        *nth_point(cache_par, ns + 1) = tmp.pos;
        ++par.size;
        save_node(now);save_node(tmp);
    }
    void split_l(node &now, byte *cache, node &par, byte *cache_par, size_t order){
        size_t s = now.size >> 1;
        now.size -= s;
        size_t ns = now.size;
        point pos = alloc.alloc(node_size);
        // printf("ask_for_bpt: %d where %d\n", leaf_size, pos);
        node tmp = node(*nth_key_l(cache, ns), pos, s, now.type, now.pos, now.next);
        if (now.next != invalid_p){
            node temp = load_node(now.next);
            temp.prior = tmp.pos;
            save_node(temp);
        }
        now.next = tmp.pos;
        byte *cache_tmp = (byte *)nth_key_l(cache, ns);
        save_cache_l(cache_tmp, tmp);
        save_cache_l(cache, now);
        ns = order;
        for (size_t i = par.size;i > ns + 1;i--){
            *nth_key_n(cache_par, i)    = *nth_key_n(cache_par, i - 1);
            *nth_point(cache_par, i)  = *nth_point(cache_par, i - 1);
        }
        *nth_key_n(cache_par, ns + 1) = tmp.key;
        *nth_point(cache_par, ns + 1) = tmp.pos;
        ++par.size;
        save_node(now);save_node(tmp);
    }
    /*
        * Merge now and the next of it,
        * if the result is too big, use split automatically.
        * save now and cache, but not par or cache_par
    */
    void merge_n(node &now, byte *cache, node &par, byte *cache_par, node &tmp, size_t s){
        now.next = tmp.next;
        if (tmp.next != invalid_p){
            node temp = load_node(tmp.next);
            temp.prior = now.pos;
            save_node(temp);
        }
        load_cache_n((byte *)nth_key_n(cache, now.size), tmp);
        now.size += tmp.size;
        alloc.free(tmp.pos, node_size);
        --par.size;
        for (size_t i = s + 1;i < par.size;i++){
            *nth_key_n(cache_par, i)    = *nth_key_n(cache_par, i + 1);
            *nth_point(cache_par, i)  = *nth_point(cache_par, i + 1);
        }
        if (now.size >= part_size_n) split_n(now, cache, par, cache_par, s);
        save_cache_n(cache, now);
        save_node(now);
    }
    void merge_l(node &now, byte *cache, node &par, byte *cache_par, node &tmp, size_t s){
        now.next = tmp.next;
        if (tmp.next != invalid_p){
            node temp = load_node(tmp.next);
            temp.prior = now.pos;
            save_node(temp);
        }
        load_cache_l((byte *)nth_key_l(cache, now.size), tmp);
        now.size += tmp.size;
        alloc.free(tmp.pos, node_size);
        --par.size;
        for (size_t i = s + 1;i < par.size;i++){
            *nth_key_n(cache_par, i)    = *nth_key_n(cache_par, i + 1);
            *nth_point(cache_par, i)  = *nth_point(cache_par, i + 1);
        }
        if (now.size >= part_size_l) split_l(now, cache, par, cache_par, s);
        save_cache_l(cache, now);
        save_node(now);
    }
    /*
        * Insert (k,v) and return true if it is an insertion and false for a change, 
        * whether split or not is considered in its parent,
        * as we prove that all parts are smaller than part_size, the memory is safe.
        * But mention to judge the size of the root.(actually, it does need to be specially treated)
    */
    bool _insert_n(node &now, const key_t &k, const value_type &v, byte *cache){
        size_t ord  = b_search_n(cache, k, now.size);
        point loc = *nth_point(cache, ord);
        node node_child = load_node(loc);
        bool ret;
        if (node_child.type){
            byte cache_child[node_size];
            load_cache_n(cache_child, node_child);
            ret = _insert_n(node_child, k, v, cache_child);
            if (!ret) return false;
            if (node_child.size >= part_size_n) {
                deal_surplus_n(node_child, now, cache_child, cache, ord);
            }
            else {
                save_node(node_child);
                save_cache_n(cache_child, node_child);
            }
        }
        else{
            byte cache_child[node_size];
            load_cache_l(cache_child, node_child);
            ret = _insert_l(node_child, k, v, cache_child);
            if (!ret) return false;
            if (node_child.size >= part_size_l) {
                deal_surplus_l(node_child, now, cache_child, cache, ord);
            }
            else {
                save_node(node_child);
                save_cache_l(cache_child, node_child);
            }
        }
        return ret;
    }
    bool _insert_l(node &now, const key_t &k, const value_type &v, byte *cache){
        size_t ord  = b_search_l(cache, k, now.size);
        point loc = nth_value_loc(now, ord);
        if (equal(*nth_key_l(cache, ord), k)) return false;
        ++num;
        for (size_t i = now.size;i > ord + 1;i--){
            *nth_key_l(cache, i)    = *nth_key_l(cache, i - 1);
            *nth_value(cache, i)    = *nth_value(cache, i - 1);
        }
        ++now.size;
        *nth_key_l(cache, ord + 1) = k;
        *nth_value(cache, ord + 1) = v;
        // p.key = *nth_key(p, 0); shall we add this?
        return true;
    }
    /*
        * Insert (k,v) at the head of the tree and return true, 
        * whether split or not is considered in its parent,
        * as we prove that all parts are smaller than part_size, the memory is safe.
        * But mention to judge the size of the root.
    */
    void _insert_head_n(node &now, const key_t &k, const value_type &v, byte *cache){
        node node_child = load_node(*nth_point(cache));
        if (node_child.type){
            byte cache_child[node_size];
            load_cache_n(cache_child, node_child);
            _insert_head_n(node_child, k, v, cache_child);
            *nth_key_n(cache, 0) = now.key = k;
            if (node_child.size >= part_size_n){
                deal_surplus_n(node_child, now, cache_child, cache, 0);
            }
            else{
                save_node(node_child);
                save_cache_n(cache_child, node_child);
            }
        }
        else{
            byte cache_child[node_size];
            load_cache_l(cache_child, node_child);
            _insert_head_l(node_child, k, v, cache_child);
            *nth_key_l(cache, 0) = now.key = k;
            if (node_child.size >= part_size_l){
                deal_surplus_l(node_child, now, cache_child, cache, 0);
            }
            else {
                save_node(node_child);
                save_cache_l(cache_child, node_child);
            }
        }
    }
    void _insert_head_l(node &now, const key_t &k, const value_type &v, byte *cache){
        ++num;
        for (size_t i = now.size;i > 0;i--){
            *nth_key_l(cache, i)    = *nth_key_l(cache, i - 1);
            *nth_value(cache, i)    = *nth_value(cache, i - 1);
        }
        ++now.size;
        *nth_key_l(cache, 0) = now.key = k;
        *nth_value(cache, 0) = v;
    }
    /*
        * remove (k,v) and return true if it does remove k and false when it cannot find k, 
        * whether merge or not is considered in its parent,
        * thus a merge is save unless the part_size is greater than 3.
        * But mention to judge the size of the root.
    */
    bool _remove_n(node &p, const key_t &k, byte *cache){
        size_t ord = b_search_n(cache, k, p.size);
        point loc = *nth_point(cache, ord);
        node node_child = load_node(loc);
        bool ret;
        if (node_child.type){
            byte cache_child[node_size];
            load_cache_n(cache_child, node_child);
            ret = _remove_n(node_child, k, cache_child);
            if (!ret) return false;
            *nth_key_n(cache, ord) = node_child.key;
            if (node_child.size < (part_size_n >> 1)){
                deal_deficit_n(node_child, p, cache_child, cache, ord);
            }
            else{
                save_node(node_child);
                save_cache_n(cache_child, node_child);
            }
        }
        else{
            byte cache_child[node_size];
            load_cache_l(cache_child, node_child);
            ret = _remove_l(node_child, k, cache_child);
            if (!ret) return false;
            *nth_key_n(cache, ord) = node_child.key;
            if (node_child.size < (part_size_l >> 1)){
                deal_deficit_l(node_child, p, cache_child, cache, ord);
            }
            else{
                save_node(node_child);
                save_cache_l(cache_child, node_child);
            }
        }
        p.key = *nth_key_n(cache, 0);
        return ret;
    }
    bool _remove_l(node &p, const key_t k, byte *cache){
        size_t ord = b_search_l(cache, k, p.size);
        if (equal(*nth_key_l(cache, ord), k)){
            --num;
            --p.size;
            for (size_t i = ord;i < p.size;i++){
                *nth_key_l(cache, i)    = *nth_key_l(cache, i + 1);
                *nth_value(cache, i)    = *nth_value(cache, i + 1);
            }
            p.key = *nth_key_l(cache, 0);
            return true;
        }
        else return false;
    }
public:
    bplustree():part_size_n((node_size - sizeof(node)) / (sizeof(key_t) + sizeof(point))),
	part_size_l((node_size - sizeof(node)) / (sizeof(key_t) + sizeof(value_type))), 
    root(), num(0), root_pos(0){}
    void init(const char *datafile_name, const char *alloc_name){
        index_name = new char[strlen(alloc_name) + 1];
        strcpy(index_name, alloc_name);
        data_name = new char[strlen(datafile_name) + 1];
        strcpy(data_name, datafile_name);
        alloc.init(index_name);
        datafile = fopen(data_name, "rb+");
        if (!datafile) datafile = fopen(data_name, "wb+");
		#ifdef OUTPUT_INIT
        printf("size_of_node: %d; leaf_part: %d; non_leaf_part: %d\n", sizeof(node), part_size_l, part_size_n);
        printf("ini_root_point_seek: %d\n", 0);
		#endif
        if (!fread(&root_pos, sizeof(point), 1, datafile)){
            alloc.alloc(node_size);
            root_pos = alloc.alloc(node_size);
            fseek(datafile, root_pos, SEEK_SET);
            // printf("ini_root_seek: %d\n", root_pos);
            fwrite(&root, sizeof(node), 1, datafile);
            // printf("which is: pos:%d size:%d key:%d\n", root.pos, root.size, root.key);
        }
        else{
            fread(&num, sizeof(size_t), 1, datafile);
            root = load_node(root_pos);
        }
        // printf("\n%d\n", root_pos);
    }
    void clear(){
        if (datafile) fclose(datafile);
        datafile = fopen(data_name, "wb+");
        alloc.refill(index_name);
        num = 0;
        alloc.alloc(node_size);
        root_pos = alloc.alloc(node_size);
        root = node();
		#ifdef DEBUG_MODE
        printf("clear_root_point_seek: %d\n", 0);
		#endif
        fseek(datafile, root_pos, SEEK_SET);
		fwrite(&root, sizeof(node), 1, datafile);
		#ifdef DEBUG_MODE
		printf("clear_root_seek: %d\n", 0);
        #ifdef TEST_INT_MODE
        printf("which is: pos:%d size:%d key:%s\n", root.pos, root.size, root.key.a);
		#endif
        #ifdef TEST_STRING_MODE
        printf("which is: pos:%d size:%d key:%s\n", root.pos, root.size, root.key.inf);
        #endif
		#endif
    }
    bool empty()const{
        return !num;
    }
    ~bplustree(){
        alloc.save(index_name);
        save_node(root);
        fseek(datafile, 0, SEEK_SET);
        if (root.pos != invalid_p) root_pos = root.pos;
		#ifdef DEBUG_MODE
        printf("decode_root_point_seek: %d\nwhich is: %d\n", 0, root_pos);printf("which is: %d\n", root_pos);
		#endif
        fwrite(&root_pos, sizeof(point), 1, datafile);
        fwrite(&num, sizeof(size_t), 1, datafile);
        if (datafile) fclose(datafile);
		if (index_name != nullptr)        
			delete []index_name;
		if (data_name != nullptr)
			delete []data_name;
    }
    bool count(const key_t &k) const{
        if (empty()) return 0;
        if (com(k, root.key)) return 0;
        point p = _find(root, k);
        if (p == invalid_p) return 0;
        return 1;
    }
    find_type find(const key_t &k)const{
        if (empty()) return find_type(false, value_type());
        if (com(k, root.key)) return find_type(false, value_type());
        point p = _find(root, k);
        if (p == invalid_p) return find_type(false, value_type());
        fseek(datafile, p, SEEK_SET);
        value_type v;
        fread(&v, sizeof(value_type), 1, datafile);
		#ifdef DEBUG_MODE
        printf("find_seek_in_database: %d\nwhich is: %d\n", p + sizeof(key_t),v);
		#endif
        return find_type(true, v);
    }
    bool set(const key_t &k, const value_type &v){
        if (empty()) throw(container_is_empty());
        if (com(k, root.key)) return false;
        point p = _find(root, k);
        if (p == invalid_p) return false;
        fseek(datafile, p, SEEK_SET);
        fwrite(&v, sizeof(value_type), 1, datafile);
        return 1;
    }
    bool insert(const key_t &k,const value_type &v){
        if (root.pos == invalid_p){
            if (!datafile) throw(runtime_error());
            root.pos = root_pos;
            root.size = num = 1;root.key = k;root.type = 0;
            fseek(datafile, root.pos + sizeof(node), SEEK_SET);
            // printf("insert_no_root_seek\n");
            fwrite(&k, sizeof(key_t), 1, datafile);fwrite(&v, sizeof(value_type), 1, datafile);
            // printf("which is: key: %d value: %d\n", k, v);
            return true;
        }
        bool ret = true;
        if (root.type){
            byte cache[node_size];
            load_cache_n(cache, root);
            if (com(root.key, k)) ret = _insert_n(root, k, v, cache);
            else _insert_head_n(root, k, v, cache);
            save_cache_n(cache, root);
            if (root.size >= part_size_n){
                point pos = alloc.alloc(node_size);
                node now_root(root.key, pos, 2, 1);
                size_t s = root.size >> 1;
                root.size -= s;
                root.next = pos = alloc.alloc(node_size);
                node temp(*nth_key_n(cache, root.size), pos, s, root.type, root.pos);
                save_cache_n((byte *)nth_key_n(cache, root.size), temp);
                save_node(temp);
                *nth_key_n(cache, 0) = root.key, *nth_point(cache, 0) = root.pos;
                *nth_key_n(cache, 1) = temp.key, *nth_point(cache, 1) = temp.pos;
                save_cache_n(cache, now_root);
                save_node(root);
                root = now_root;root_pos = root.pos;
            }
        }
        else{
            byte cache[node_size];
            load_cache_l(cache, root);
            if (com(root.key, k)) ret = _insert_l(root, k, v, cache);
            else _insert_head_l(root, k, v, cache);
            save_cache_l(cache, root);
            if (root.size >= part_size_l){
                point pos = alloc.alloc(node_size);
                node now_root(root.key, pos, 2, 1);
                size_t s = root.size >> 1;
                root.size -= s;
                root.next = pos = alloc.alloc(node_size);
                node temp(*nth_key_l(cache, root.size), pos, s, root.type, root.pos);
                save_cache_l((byte *)nth_key_l(cache, root.size), temp);
                save_node(temp);
                *nth_key_n(cache, 0) = root.key, *nth_point(cache, 0) = root.pos;
                *nth_key_n(cache, 1) = temp.key, *nth_point(cache, 1) = temp.pos;
                save_cache_n(cache, now_root);
                save_node(root);
                root = now_root;root_pos = root.pos;
            }
        }
        return ret;
    }
    bool remove(const key_t &k){
        if (empty()) return false;
        if (com(k, root.key)) return 0;
        bool ret = true;
        byte cache[node_size];
        if (root.type) {
            load_cache_n(cache, root);
            ret = _remove_n(root, k, cache);
            if (root.size < 2){
                alloc.free(root.pos, node_size);
                root = load_node(*nth_point(cache));
                root_pos = root.pos;
            }
            else{
                save_cache_n(cache, root);
                save_node(root);
            }
        }
        else {
            load_cache_l(cache, root);
            ret = _remove_l(root, k, cache);
                save_cache_l(cache, root);
                save_node(root);
        }
        return ret;
    }
    vector<list_type> listof(const key_t &k, bool (*comp)(const key_t &a, const key_t &b)) const{
		if (empty()) return vector < list_type >();
        return _listof(root, k, comp);
    }
    int size() const{
        return num;
    }
};
#endif
