#pragma warning(disable : 4996)
#ifndef SJTU_BPLUSTREE_HPP
#define SJTU_BPLUSTREE_HPP
#include <cstddef>
#include <functional>
#include <cstring>
#include "exceptions.h"
#include "alloc.hpp"
#include "vector.hpp"
#include "bufferpool.hpp"
#include "tool.h"
const point invalid_p = 0xdeadbeef;

template<class key_t,
         class value_type, 
         size_t node_size = 4096, 
         class Compare = std::less<key_t>
>   class bplustree
{
    using point         =   long;
    using byte          =   char;
    using list_t        =   pair<key_t, value_type>;
    using find_t        =   pair<bool, value_type>;
    using sub_find_t    =   pair<size_t, buf_block_t*>;
    struct node
    {
        key_t       key;
        point       prior, next;
        point       pos;
        size_t      size;                    //the size of its brothers
        bool        type;                    //0 for a leaf and 1 otherwise
        node(key_t k = key_t(),
        point p = invalid_p, size_t s = 1, bool ty = 0, 
        point pre = invalid_p, point nex = invalid_p)
        :key(k),pos(p),prior(pre),next(nex),size(s),type(ty){}
        node operator=(const node &other){
            key = other.key, prior = other.prior, next = other.next;
            size = other.size, type = other.type, pos = other.pos;
            return *this;
        }
    };
    buf_pool_t<node_size>   *buf;
    Compare                 com;
    size_t                  num;
    FILE                    *datafile;
	const size_t            part_size_l, part_size_n;
    ALLOC<node_size>        alloc;
    point                   root_pos;
    char                    *index_name, *data_name;

    inline bool equal(const key_t& k1,const key_t& k2) const
    {
        return !(com(k1, k2) || com(k2, k1));
    }
    /*
        * basic function in order to get particular info of a node
    */
    inline key_t* nth_key_n(byte *start, size_t n = 0) const
    {
        return (key_t *)(start + (sizeof(key_t) + sizeof(point)) * n);
    }
    inline key_t* nth_key_l(byte *start, size_t n = 0) const
    {
        return (key_t *)(start + (sizeof(key_t) + sizeof(value_type)) * n);
    }
    inline point* nth_point(byte *start, size_t n = 0) const
    {
        return (point *)(start + sizeof(key_t) * (n + 1) + sizeof(point) * n);
    }
    inline value_type* nth_value(byte *start, size_t n = 0) const
    {
        return (value_type *)(start + sizeof(key_t) * (n + 1) + sizeof(value_type) * n);
    }
    /*
        * binary search function to help find a particular child
    */
    inline size_t b_search_n(byte *start,const key_t& k, size_t n) const
    {
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
    inline size_t b_search_l(byte *start,const key_t& k, size_t n) const
    {
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
    sub_find_t _find(to_block_t &to_it,const key_t& k) const
    {
        size_t ord;
        byte *cache = to_it.it->frame + sizeof(node);
        node *p     = (node *)(cache - sizeof(node));
        if (!p->type){
            ord = b_search_l(cache, k, p->size);
            if (equal(*nth_key_l(cache, ord), k)) {
                return sub_find_t(ord, to_it.it);
            }
            else return sub_find_t(false, nullptr);
        }
        else{
            ord = b_search_n(cache, k, p->size);
            to_block_t tmp = buf->load_it(*nth_point(cache, ord));
            return _find(tmp, k);
        }
    }
    /*
        * return a continuous segment of key-value sets
    */
    vector<list_t> _listof(to_block_t &to_it, const key_t &k, 
                            bool (*comp)(const key_t &a, const key_t &b)) const
    {
        size_t ord;
        to_block_t tmp;
        byte *cache = to_it.it->frame + sizeof(node);
        node *p     = (node *)(cache - sizeof(node));
        if (!p->type){
            if (com(k, p->key)) ord = 0;
            else ord = b_search_l(cache, k, p->size);
            vector<list_t> ret;
            node *now = p;
            while ( !comp(k, *nth_key_l(cache, ord)) ){
                if (!comp(*nth_key_l(cache, ord), k) )
                ret.push_back(
                    list_t(*nth_key_l(cache, ord), 
                        *nth_value(cache, ord)
                    )
                );
                if (ord + 1 < now->size) ++ord;
                else{
                    if (now->next == invalid_p) break;
                    tmp = buf->load_it(now->next);
                    now = (node *)(tmp.it->frame);
					cache = tmp.it->frame + sizeof(node);
                    ord = 0;
                }
            }
            return ret;
        }
        else {
            if (com(k, p->key)) ord = 0;
            else ord = b_search_n(cache, k, p->size);
            tmp = buf->load_it(*nth_point(cache, ord));
            return _listof(tmp, k, comp);
        }
    }
    /*
        * Make the first of p become the last of l,
        * while the size of p equals to part_size and that of l is less. 
        * Save them both
    */
    inline void Lbalance_n(to_block_t &to_it, to_block_t &to_left)
    {
        byte *cache_l = to_left.it->frame + sizeof(node),
             *cache_p = to_it.it->frame + sizeof(node);
        node *p = (node *)(cache_p - sizeof(node)),
             *l = (node *)(cache_l - sizeof(node));
        size_t ls = l->size;
        size_t mov = p->size - ((p->size + ls) >> 1);
        for (int i = 0;i < mov;i++){
            *nth_key_n(cache_l, ls + i) = *nth_key_n(cache_p, i);
            *nth_point(cache_l, ls + i) = *nth_point(cache_p, i);
        }
        l->size += mov, p->size -= mov;
        for (int i = 0;i < p->size;i++){
            *nth_key_n(cache_p, i) = *nth_key_n(cache_p, i + mov);
            *nth_point(cache_p, i) = *nth_point(cache_p, i + mov);
        }
        p->key = *nth_key_n(cache_p);
        buf->dirty(to_left), buf->dirty(to_it);
    }
    inline void Lbalance_l(to_block_t &to_it, to_block_t &to_left)
    {
        byte *cache_l = to_left.it->frame + sizeof(node),
             *cache_p = to_it.it->frame + sizeof(node);
        node *p = (node *)(cache_p - sizeof(node)),
             *l = (node *)(cache_l - sizeof(node));
        size_t ls = l->size;
        size_t mov = p->size - ((p->size + ls) >> 1);
        for (int i = 0;i < mov;i++){
            *nth_key_l(cache_l, ls + i) = *nth_key_l(cache_p, i);
            *nth_value(cache_l, ls + i) = *nth_value(cache_p, i);
        }
        l->size += mov, p->size -= mov;
        for (int i = 0;i < p->size;i++){
            *nth_key_l(cache_p, i) = *nth_key_l(cache_p, i + mov);
            *nth_value(cache_p, i) = *nth_value(cache_p, i + mov);
        }
        p->key = *nth_key_l(cache_p);
        buf->dirty(to_left), buf->dirty(to_it);
        //print_node_l(to_it.it);print_node_l(to_left.it);
    }
    /*
        * Make the last of p become the first of r,
        * while the size of p equals to part_size and that of r is less. 
        * Save them both.
    */
    inline void Rbalance_n(to_block_t &to_it, to_block_t &to_right)
    {
        byte *cache_r = to_right.it->frame + sizeof(node),
             *cache_p = to_it.it->frame + sizeof(node);
        node *p = (node *)(cache_p - sizeof(node)), 
             *r = (node *)(cache_r - sizeof(node));
        size_t mov = p->size - ((p->size + r->size) >> 1);
        for (size_t i = r->size + mov - 1;i > mov - 1;i--){
            *nth_key_n(cache_r, i)  = *nth_key_n(cache_r, i - mov);
            *nth_point(cache_r, i)  = *nth_point(cache_r, i - mov);
        }
        r->size += mov, p->size -= mov;
        size_t ps = p->size;
        for (size_t i = 0;i < mov;i++){
            *nth_key_n(cache_r, i)  = *nth_key_n(cache_p, ps + i);
            *nth_point(cache_r, i)  = *nth_point(cache_p, ps + i);
        }
        r->key = *nth_key_n(cache_r, 0);
        buf->dirty(to_it), buf->dirty(to_right);
    }
    inline void Rbalance_l(to_block_t &to_it, to_block_t &to_right)
    {
        byte *cache_r = to_right.it->frame + sizeof(node),
             *cache_p = to_it.it->frame + sizeof(node);
        node *p = (node *)(cache_p - sizeof(node)), 
             *r = (node *)(cache_r - sizeof(node));
        size_t mov = p->size - ((p->size + r->size) >> 1);
        for (size_t i = r->size + mov - 1;i > mov - 1;i--){
            *nth_key_l(cache_r, i)  = *nth_key_l(cache_r, i - mov);
            *nth_value(cache_r, i)  = *nth_value(cache_r, i - mov);
        }
        r->size += mov, p->size -= mov;
        size_t ps = p->size;
        for (size_t i = 0;i < mov;i++){
            *nth_key_l(cache_r, i)  = *nth_key_l(cache_p, ps + i);
            *nth_value(cache_r, i)  = *nth_value(cache_p, ps + i);
        }
        r->key = *nth_key_l(cache_r, 0);
        buf->dirty(to_it), buf->dirty(to_right);
    }
    /*
        * Receive a node from the left part if avaliable
        * Save them both
    */
    inline void Lreceive_n(to_block_t &to_it, to_block_t &to_left)
    {
        byte *cache_l = to_left.it->frame + sizeof(node),
             *cache_p = to_it.it->frame + sizeof(node);
        node *p = (node *)(cache_p - sizeof(node)),
             *l = (node *)(cache_l - sizeof(node));
        for (size_t i = p->size;i > 0;i--){
            *nth_key_n(cache_p, i)  = *nth_key_n(cache_p, i - 1);
            *nth_point(cache_p, i)  = *nth_point(cache_p, i - 1);
        }
        ++(p->size);--(l->size);
        p->key = *nth_key_n(cache_p, 0) = *nth_key_n(cache_l, l->size);
        *nth_point(cache_p)             = *nth_point(cache_l, l->size);
        buf->dirty(to_it), buf->dirty(to_left);
    }
    inline void Lreceive_l(to_block_t &to_it, to_block_t &to_left)
    {
        byte *cache_l = to_left.it->frame + sizeof(node),
             *cache_p = to_it.it->frame + sizeof(node);
        node *p = (node *)(cache_p - sizeof(node)),
             *l = (node *)(cache_l - sizeof(node));
        for (size_t i = p->size;i > 0;i--){
            *nth_key_l(cache_p, i) = *nth_key_l(cache_p, i - 1);
            *nth_value(cache_p, i) = *nth_value(cache_p, i - 1);
        }
        ++(p->size);--(l->size);
        p->key = *nth_key_l(cache_p, 0) = *nth_key_l(cache_l, l->size);
        *nth_value(cache_p)             = *nth_value(cache_l, l->size);
        buf->dirty(to_it), buf->dirty(to_left);
    }
    /*
        * Receive a node from the right part if avaliable. 
        * Save them both
    */
    inline void Rreceive_n(to_block_t &to_it, to_block_t &to_right)
    {
        byte *cache_r = to_right.it->frame + sizeof(node), 
             *cache_p = to_it.it->frame + sizeof(node);
        node *p = (node *)(cache_p - sizeof(node)), 
             *r = (node *)(cache_r - sizeof(node));
        *nth_key_n(cache_p, p->size) = *nth_key_n(cache_r);
        *nth_point(cache_p, p->size) = *nth_point(cache_r);
        ++(p->size), --(r->size);
        r->key = *nth_key_n(cache_r, 1);
        for (size_t i = 0;i < r->size;i++){
            *nth_key_n(cache_r, i) = *nth_key_n(cache_r, i + 1);
            *nth_point(cache_r, i) = *nth_point(cache_r, i + 1);
        }
        buf->dirty(to_it), buf->dirty(to_right);
    }
    inline void Rreceive_l(to_block_t &to_it, to_block_t &to_right)
    {
        byte *cache_r = to_right.it->frame + sizeof(node), 
             *cache_p = to_it.it->frame + sizeof(node);
        node *p = (node *)(cache_p - sizeof(node)), 
             *r = (node *)(cache_r - sizeof(node));
        *nth_key_l(cache_p, p->size) = *nth_key_l(cache_r, 0);
        *nth_value(cache_p, p->size) = *nth_value(cache_r);
        ++(p->size), --(r->size);
        r->key = *nth_key_l(cache_r, 1);
        for (size_t i = 0;i < r->size;i++){
            *nth_key_l(cache_r, i) = *nth_key_l(cache_r, i + 1);
            *nth_value(cache_r, i) = *nth_value(cache_r, i + 1);
        }
        buf->dirty(to_it), buf->dirty(to_right);
    }
    /*
        * Find the quickest way to solve the problem of extreme size.
        * save now and cache and left/right and its cache if necessary, 
        * but do not save the cache of the parent
    */
    inline void deal_surplus_n(to_block_t &to_now_b, to_block_t &to_par_b, size_t ord)
    {
        byte *cache     = to_now_b.it->frame + sizeof(node), 
             *cache_par = to_par_b.it->frame + sizeof(node);
        node *now = (node *)(cache     - sizeof(node)), 
             *par = (node *)(cache_par - sizeof(node));
        node *tmp;
        to_block_t to_tmp_b;
        if (now->pos != *nth_point(cache_par)){
            to_tmp_b = buf->load_it(now->prior);
            tmp = (node *)(to_tmp_b.it->frame);
            if (tmp->size < part_size_n * 3 / 4) {
                Lbalance_n(to_now_b, to_tmp_b);
                *nth_key_n(cache_par, ord) = now->key;
                buf->dirty(to_par_b);
                return;
            }
        }
        if (now->pos != *nth_point(cache_par, par->size - 1)){
            to_tmp_b = buf->load_it(now->next);
            tmp = (node *)(to_tmp_b.it->frame);
            if (tmp->size < part_size_n * 3 / 4){
                Rbalance_n(to_now_b, to_tmp_b);
                *nth_key_n(cache_par, ord + 1) = tmp->key;
                buf->dirty(to_par_b);
                return;
            }
            else split_n(to_now_b, to_par_b, ord);
        }
        else split_n(to_now_b, to_par_b, ord);
    }
    inline void deal_surplus_l(to_block_t &to_now_b, to_block_t &to_par_b, size_t ord)
    {
        byte *cache     = to_now_b.it->frame + sizeof(node), 
             *cache_par = to_par_b.it->frame + sizeof(node);
        node *now = (node *)(cache     - sizeof(node)), 
             *par = (node *)(cache_par - sizeof(node));
        node *tmp;
        to_block_t to_tmp_b;
        if (now->pos != *nth_point(cache_par)){
            to_tmp_b = buf->load_it(now->prior);
            tmp = (node *)(to_tmp_b.it->frame);
            if (tmp->size < part_size_l * 3 / 4) {
                Lbalance_l(to_now_b, to_tmp_b);
                if (!equal(*nth_key_n(cache_par, ord), now->key)){
                    *nth_key_n(cache_par, ord) = now->key;
                    buf->dirty(to_par_b);
                }
                return;
            }
        }
        if (now->pos != *nth_point(cache_par, par->size - 1)){
            to_tmp_b = buf->load_it(now->next);
            tmp = (node *)(to_tmp_b.it->frame);
            if (tmp->size < part_size_l * 3 / 4){
                Rbalance_l(to_now_b, to_tmp_b);
                if (!equal(*nth_key_n(cache_par, ord + 1), tmp->key)){
                    *nth_key_n(cache_par, ord + 1) = tmp->key;
                    buf->dirty(to_par_b);
                }
                return;
            }
            else split_l(to_now_b, to_par_b, ord);
        }
        else split_l(to_now_b, to_par_b, ord);
    }
    inline void deal_deficit_n(to_block_t &to_now_b, to_block_t &to_par_b, size_t ord)
    {
        byte *cache     = to_now_b.it->frame + sizeof(node),
             *cache_par = to_par_b.it->frame + sizeof(node);
        node *now = (node *)(cache     - sizeof(node)),
             *par = (node *)(cache_par - sizeof(node));
        node *tmp;
        to_block_t to_tmp_b;
        if (now->pos != *nth_point(cache_par, par->size - 1)){
            to_tmp_b = buf->load_it(now->next);
            tmp = (node *)(to_tmp_b.it->frame);
            if (tmp->size > (part_size_n >> 1)){
                Rreceive_n(to_now_b, to_tmp_b);
                if (!equal(*nth_key_n(cache_par, ord + 1), tmp->key)){
                    *nth_key_n(cache_par, ord + 1) = tmp->key;
                    buf->dirty(to_par_b);
                }
                return;
            }
            else merge_n(to_now_b, to_par_b, to_tmp_b, ord);
            return;
        }
        if (now->pos != *nth_point(cache_par)){
            to_tmp_b = buf->load_it(now->prior);
            tmp = (node *)(to_tmp_b.it->frame);
            if ((part_size_n >> 1) < tmp->size){
                Lreceive_n(to_now_b, to_tmp_b);
                if (!equal(*nth_key_n(cache_par, ord), now->key)){
                    *nth_key_n(cache_par, ord) = now->key;
                    buf->dirty(to_par_b);
                }
                return;
            }
            else merge_n(to_tmp_b, to_par_b, to_now_b, ord - 1);
        }
    }
    inline void deal_deficit_l(to_block_t &to_now_b, to_block_t &to_par_b, size_t ord)
    {
        byte *cache     = to_now_b.it->frame + sizeof(node),
             *cache_par = to_par_b.it->frame + sizeof(node);
        node *now = (node *)(cache - sizeof(node)),
             *par = (node *)(cache_par - sizeof(node));
        node *tmp;
        to_block_t to_tmp_b;
        if (now->pos != *nth_point(cache_par, par->size - 1)){
            to_tmp_b = buf->load_it(now->next);
            tmp = (node *)(to_tmp_b.it->frame);
            if (tmp->size > (part_size_l >> 1)){
                Rreceive_l(to_now_b, to_tmp_b);
                if (!equal(*nth_key_n(cache_par, ord + 1), tmp->key)){
                    *nth_key_n(cache_par, ord + 1) = tmp->key;
                    buf->dirty(to_par_b);
                }
                return;
            }
            else merge_l(to_now_b, to_par_b, to_tmp_b, ord);
            return;
        }
        if (now->pos != *nth_point(cache_par)){
            to_tmp_b = buf->load_it(now->prior);
            tmp = (node *)(to_tmp_b.it->frame);
            if (tmp->size > (part_size_l >> 1)){
                Lreceive_l(to_now_b, to_tmp_b);
                if (!equal(*nth_key_n(cache_par, ord), now->key)){
                    *nth_key_n(cache_par, ord) = now->key;
                    buf->dirty(to_par_b);
                }
                return;
            }
            else merge_l(to_tmp_b, to_par_b, to_now_b, ord - 1);
        }
    }
    /*
        * Split the cache belonging to node now. 
        * save now and its cache but do not save any info of parent into storage,
        * though we have changed it in memory
    */
    void split_n(to_block_t &to_now_b, to_block_t &to_par_b, size_t order)
    {
        byte *cache     = to_now_b.it->frame + sizeof(node),
             *cache_par = to_par_b.it->frame + sizeof(node);
        node *now = (node *)(cache - sizeof(node)), 
             *par = (node *)(cache_par - sizeof(node));
        size_t s = now->size >> 1;
        now->size -= s;
        size_t ns = now->size;

        point pos = alloc.alloc(node_size);
        to_block_t to_tmp_b = buf->load_it(pos);

        byte *cache_tmp = to_tmp_b.it->frame + sizeof(node);
        node *tmp       = (node *)(cache_tmp - sizeof(node));
        tmp->key    = *nth_key_n(cache, ns), tmp->type = now->type, tmp->pos = pos, tmp->size = s;
        tmp->prior  = now->pos, tmp->next = now->next;

        if (now->next != invalid_p){
            to_block_t temp = buf->load_it(now->next);
            ((node *)(temp.it->frame))->prior = tmp->pos;
            buf->dirty(temp);
        }
        now->next = tmp->pos;

        for (size_t i = 0;i < s;i++){
            *nth_key_n(cache_tmp, i) = *nth_key_n(cache, ns + i);
            *nth_point(cache_tmp, i) = *nth_point(cache, ns + i);
        }
        buf->dirty(to_tmp_b), buf->dirty(to_now_b);
        ns = order;
        for (size_t i = par->size;i > ns + 1;i--){
            *nth_key_n(cache_par, i)    = *nth_key_n(cache_par, i - 1);
            *nth_point(cache_par, i)    = *nth_point(cache_par, i - 1);
        }
        *nth_key_n(cache_par, ns + 1) = tmp->key;
        *nth_point(cache_par, ns + 1) = tmp->pos;
        ++(par->size);
        buf->dirty(to_par_b);
    }
    void split_l(to_block_t &to_now_b, to_block_t &to_par_b, size_t order){
        byte *cache     = to_now_b.it->frame + sizeof(node),
             *cache_par = to_par_b.it->frame + sizeof(node);
        node *now = (node *)(cache - sizeof(node)), 
             *par = (node *)(cache_par - sizeof(node));
        size_t s = now->size >> 1;
        now->size -= s;
        size_t ns = now->size;

        point pos           = alloc.alloc(node_size);
        to_block_t to_tmp_b = buf->load_it(pos);
        byte *cache_tmp     = to_tmp_b.it->frame + sizeof(node);
        node *tmp           = (node *)(cache_tmp - sizeof(node));
        tmp->key    = *nth_key_l(cache, ns), tmp->type = now->type, tmp->pos = pos, tmp->size = s;
        tmp->prior  = now->pos, tmp->next = now->next;

        if (now->next != invalid_p){
            to_block_t temp = buf->load_it(now->next);
            ((node *)(temp.it->frame))->prior = tmp->pos;
            buf->dirty(temp);
        }
        now->next = tmp->pos;

        for (size_t i = 0;i < s;i++){
            *nth_key_l(cache_tmp, i) = *nth_key_l(cache, ns + i);
            *nth_value(cache_tmp, i) = *nth_value(cache, ns + i);
        }
        buf->dirty(to_tmp_b), buf->dirty(to_now_b);
        ns = order;
        for (size_t i = par->size;i > ns + 1;i--){
            *nth_key_n(cache_par, i)    = *nth_key_n(cache_par, i - 1);
            *nth_point(cache_par, i)    = *nth_point(cache_par, i - 1);
        }
        *nth_key_n(cache_par, ns + 1) = tmp->key;
        *nth_point(cache_par, ns + 1) = tmp->pos;
        ++(par->size);
        buf->dirty(to_par_b);
    }
    /*
        * Merge now and the next of it,
        * if the result is too big, use split automatically.
        * save now and cache, but not par or cache_par
    */
    void merge_n(to_block_t &to_now_b, to_block_t &to_par_b, to_block_t &to_tmp_b, size_t s)
    {
        byte *cache     = to_now_b.it->frame + sizeof(node),
             *cache_par = to_par_b.it->frame + sizeof(node),
             *cache_tmp = to_tmp_b.it->frame + sizeof(node); 
        node *now = (node *)(cache - sizeof(node)), 
             *par = (node *)(cache_par - sizeof(node)),
             *tmp = (node *)(cache_tmp - sizeof(node));
        now->next = tmp->next;
        if (tmp->next != invalid_p){
            to_block_t temp = buf->load_it(tmp->next);
            ((node *)(temp.it->frame))->prior = now->pos;
            buf->dirty(temp);
        }
        size_t ns = now->size;
        for (size_t i = 0;i < tmp->size;i++){
            *nth_key_n(cache, i + ns) = *nth_key_n(cache_tmp, i);
            *nth_point(cache, i + ns) = *nth_point(cache_tmp, i);
        }
        now->size += tmp->size;
        alloc.free(tmp->pos, node_size);
        --(par->size);
        for (size_t i = s + 1;i < par->size;i++){
            *nth_key_n(cache_par, i) = *nth_key_n(cache_par, i + 1);
            *nth_point(cache_par, i) = *nth_point(cache_par, i + 1);
        }
        if (now->size >= part_size_n)
            split_n(to_now_b, to_par_b, s);
        buf->dirty(to_now_b), buf->dirty(to_par_b);
    }
    void merge_l(to_block_t &to_now_b, to_block_t &to_par_b, to_block_t &to_tmp_b, size_t s)
    {
        byte *cache     = to_now_b.it->frame + sizeof(node),
             *cache_par = to_par_b.it->frame + sizeof(node),
             *cache_tmp = to_tmp_b.it->frame + sizeof(node); 
        node *now = (node *)(cache - sizeof(node)), 
             *par = (node *)(cache_par - sizeof(node)),
             *tmp = (node *)(cache_tmp - sizeof(node));
        now->next = tmp->next;
        if (tmp->next != invalid_p){
            to_block_t temp = buf->load_it(tmp->next);
            ((node *)(temp.it->frame))->prior = now->pos;
            buf->dirty(temp);
        }
        size_t ns = now->size;
        for (size_t i = 0;i < tmp->size;i++){
            *nth_key_l(cache, i + ns) = *nth_key_l(cache_tmp, i);
            *nth_value(cache, i + ns) = *nth_value(cache_tmp, i);
        }
        now->size += tmp->size;
        alloc.free(tmp->pos, node_size);
        --(par->size);
        for (size_t i = s + 1;i < par->size;i++){
            *nth_key_n(cache_par, i)  = *nth_key_n(cache_par, i + 1);
            *nth_point(cache_par, i)  = *nth_point(cache_par, i + 1);
        }
        if (now->size >= part_size_l) split_n(to_now_b, to_par_b, s);
        buf->dirty(to_now_b), buf->dirty(to_par_b);
    }
    /*
        * Insert (k,v) and return true if it is an insertion and false for a change, 
        * whether split or not is considered in its parent,
        * as we prove that all parts are smaller than part_size, the memory is safe.
        * But mention to judge the size of the root.(actually, it does need to be specially treated)
    */
    int _insert_n(to_block_t &to_it, const key_t &k, const value_type &v)
    {
        byte *cache = to_it.it->frame + sizeof(node);
        node *now   = (node *)(cache - sizeof(node));
        size_t ord  = b_search_n(cache, k, now->size);
        point loc   = *nth_point(cache, ord);
        to_block_t buf_child    = buf->load_it(loc);    
        byte *cache_child       = buf_child.it->frame + sizeof(node);
        node *node_child        = (node *)(cache_child - sizeof(node));
        int ret;
        if (node_child->type){
            ret = _insert_n(buf_child, k, v);
            if (!ret) return 0;
            if (ret == 2){
                deal_surplus_n(buf_child, to_it, ord);
                if (now->size >= part_size_n) ret = 2;
                else ret = 1;
            }
            else buf->dirty(buf_child);
        }
        else{
            ret = _insert_l(buf_child, k, v);
            if (!ret) return 0;
            if (ret == 2){
                deal_surplus_l(buf_child, to_it, ord);
                if (now->size >= part_size_n) ret = 2;
                else ret = 1;
            }
            else buf->dirty(buf_child);
        }
        return ret;
    }
    int _insert_l(to_block_t &to_it, const key_t &k, const value_type &v)
    {
        byte *cache = to_it.it->frame + sizeof(node);
        node *now   = (node *)(cache - sizeof(node));
        size_t ord  = b_search_l(cache, k, now->size);
        if (equal(*nth_key_l(cache, ord), k)) return false;
        ++num;
        for (size_t i = now->size;i > ord + 1;i--){
            *nth_key_l(cache, i)    = *nth_key_l(cache, i - 1);
            *nth_value(cache, i)    = *nth_value(cache, i - 1);
        }
        ++(now->size);
        *nth_key_l(cache, ord + 1) = k;
        *nth_value(cache, ord + 1) = v;
        if (now->size >= part_size_l) return 2;
        return 1;
    }
    /*
        * Insert (k,v) at the head of the tree and return true, 
        * whether split or not is considered in its parent,
        * as we prove that all parts are smaller than part_size, the memory is safe.
        * But mention to judge the size of the root.
    */
    void _insert_head_n(to_block_t &to_it, const key_t &k, const value_type &v)
    {
        byte *cache = to_it.it->frame + sizeof(node);
        node *now   = (node *)(cache - sizeof(node));
        to_block_t buf_child    = buf->load_it(*nth_point(cache));    
        byte *cache_child       = buf_child.it->frame + sizeof(node);
        node *node_child        = (node *)(cache_child - sizeof(node));
        if (node_child->type){
            _insert_head_n(buf_child, k, v);
            *nth_key_n(cache, 0) = now->key = k;
            if (node_child->size >= part_size_n){
                deal_surplus_n(buf_child, to_it, 0);
            }
            else buf->dirty(buf_child);
        }
        else{
            _insert_head_l(buf_child, k, v);
            *nth_key_l(cache, 0) = now->key = k;
            if (node_child->size >= part_size_l){
                deal_surplus_l(buf_child, to_it, 0);
            }
            else buf->dirty(buf_child);
        }
    }
    void _insert_head_l(to_block_t &to_it, const key_t &k, const value_type &v)
    {
        byte *cache = to_it.it->frame + sizeof(node);
        node *now   = (node *)(cache - sizeof(node));
        ++num;
        for (size_t i = now->size;i > 0;i--){
            *nth_key_l(cache, i)    = *nth_key_l(cache, i - 1);
            *nth_value(cache, i)    = *nth_value(cache, i - 1);
        }
        ++(now->size);
        *nth_key_l(cache, 0) = now->key = k;
        *nth_value(cache, 0) = v;
    }
    /*
        * remove (k,v) and return true if it does remove k and false when it cannot find k, 
        * whether merge or not is considered in its parent,
        * thus a merge is save unless the part_size is greater than 3.
        * But mention to judge the size of the root.
    */
    bool _remove_n(to_block_t &to_it, const key_t &k){
        byte *cache = to_it.it->frame + sizeof(node);
        node *now = (node *)(cache - sizeof(node));
        size_t ord = b_search_n(cache, k, now->size);
        point loc = *nth_point(cache, ord);
        to_block_t buf_child = buf->load_it(loc);    
        byte *cache_child = buf_child.it->frame + sizeof(node);
        node *node_child = (node *)(cache_child - sizeof(node));
        bool ret;
        if (node_child->type){
            ret = _remove_n(buf_child, k);
            if (!ret) return false;
            *nth_key_n(cache, ord) = node_child->key;
            if (node_child->size < (part_size_n >> 1)){
                deal_deficit_n(buf_child, to_it, ord);
            }
            else buf->dirty(buf_child);
        }
        else{
            ret = _remove_l(buf_child, k);
            if (!ret) return false;
            *nth_key_n(cache, ord) = node_child->key;
            if (node_child->size < (part_size_l >> 1)){
                deal_deficit_l(buf_child, to_it, ord);
            }
            else buf->dirty(buf_child);
        }
        now->key = *nth_key_n(cache, 0);
        return ret;
    }
    bool _remove_l(to_block_t &to_it, const key_t k){
        byte *cache = to_it.it->frame + sizeof(node);
        node *now = (node *)(cache - sizeof(node));
        size_t ord = b_search_l(cache, k, now->size);
        if (equal(*nth_key_l(cache, ord), k)){
            --num;
            --(now->size);
            for (size_t i = ord;i < now->size;i++){
                *nth_key_l(cache, i)    = *nth_key_l(cache, i + 1);
                *nth_value(cache, i)    = *nth_value(cache, i + 1);
            }
            now->key = *nth_key_l(cache, 0);
            return true;
        }
        else return false;
    }
public:
    bplustree():part_size_n((node_size - sizeof(node)) / (sizeof(key_t) + sizeof(point))),
	            part_size_l((node_size - sizeof(node)) / (sizeof(key_t) + sizeof(value_type))), 
                num(0), root_pos(0), buf(nullptr){}
    void init(const char *datafile_name, const char *alloc_name)
    {
        buf = new buf_pool_t<node_size>;
        index_name = new char[strlen(alloc_name) + 1];
        strcpy(index_name, alloc_name);
        data_name = new char[strlen(datafile_name) + 1];
        strcpy(data_name, datafile_name);
        alloc.init(index_name);
        datafile = fopen(data_name, "rb+");
        if (!datafile) datafile = fopen(data_name, "wb+");
        buf->init(datafile);
		#ifdef OUTPUT_INIT
        printf("size_of_node: %d; leaf_part: %d; non_leaf_part: %d\n", sizeof(node), part_size_l, part_size_n);
        printf("ini_root_point_seek: %d\n", 0);
		#endif
        if (!fread(&root_pos, sizeof(point), 1, datafile)){
            alloc.alloc(node_size);
            root_pos = alloc.alloc(node_size);
            node tmp;
            fseek(datafile, root_pos, SEEK_SET);
            fwrite(&tmp, sizeof(node), 1, datafile);
        }
        else{
            fread(&num, sizeof(size_t), 1, datafile);
        }
    }
    void clear()
    {
        if (datafile) fclose(datafile);
        datafile = fopen(data_name, "wb+");
        alloc.refill(index_name);
        num = 0;
        alloc.alloc(node_size);
        root_pos = alloc.alloc(node_size);
        node tmp;
        fseek(datafile, root_pos, SEEK_SET);
        fwrite(&tmp, sizeof(node), 1, datafile);
		#ifdef DEBUG_MODE
        printf("clear_root_point_seek: %d\n", 0);
		#endif
        buf->file_change(datafile);
        buf->re_init();
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
    bool empty() const
    {
        return !num;
    }
    ~bplustree()
    {
        alloc.save(index_name);
        fseek(datafile, 0, SEEK_SET);
		#ifdef DEBUG_MODE
        printf("decode_root_point_seek: %d\nwhich is: %d\n", 0, root_pos);printf("which is: %d\n", root_pos);
		#endif
        fwrite(&root_pos, sizeof(point), 1, datafile);
        fwrite(&num, sizeof(size_t), 1, datafile);
		if (index_name != nullptr)        
			delete []index_name;
		if (data_name != nullptr)
			delete []data_name;
        delete buf;
		if (datafile) fclose(datafile);
    }
    bool count(const key_t &k) const
    {
        if (empty()) return 0;
        to_block_t root = buf->load_it(root_pos);
        if (com(k, ((node *)(root.it->frame))->key)) return 0;
        sub_find_t p = _find(root, k);
        if (p.second == nullptr) return 0;
        return 1;
    }
    find_t find(const key_t &k) const
    {
		if (empty()) return find_t(false,value_type());
        to_block_t root = buf->load_it(root_pos);
        if (com(k, ((node *)(root.it->frame))->key)) return find_t(false, value_type());
        sub_find_t p = _find(root, k);
        if (p.second == nullptr) return find_t(false, value_type());
		#ifdef DEBUG_MODE
        printf("find_seek_in_database: %d\nwhich is: %d\n", p + sizeof(key_t),v);
		#endif
        return find_t(true, *nth_value(p.second->frame + sizeof(node), p.first));
    }
    bool set(const key_t &k, const value_type &v) const
    {
        if (empty()) throw(container_is_empty());
        to_block_t root = buf->load_it(root_pos);
        if (com(k, ((node *)(root.it->frame))->key)) return false;
        sub_find_t p = _find(root, k);
        if (p.second == nullptr) return false;
        *nth_value(p.second->frame + sizeof(node), p.first) = v;
        buf->dirty(p.second);
        return 1;
    }
    bool insert(const key_t &k, const value_type &v)
    {
        to_block_t root = buf->load_it(root_pos);
        node *root_n = (node *)(root.it->frame);
        if (root_n->pos == invalid_p){
            if (!datafile) throw(runtime_error());
            root_n->pos     = root_pos, root_n->size    = num = 1;
            root_n->key     = k,        root_n->type    = 0;
            root_n->prior   = root_n->next = invalid_p;
            *nth_key_l(root.it->frame + sizeof(node)) = k;
            *nth_value(root.it->frame + sizeof(node)) = v;
            buf->dirty(root);
            return true;
        }
        int ret = 1;
        byte *cache = root.it->frame + sizeof(node);
        if (root_n->type){
            if (com(root_n->key, k))
                ret = _insert_n(root, k, v);
            else{
                ret = 2;
                _insert_head_n(root, k, v);
            }
            if (ret == 2){
                buf->dirty(root);
                if (root_n->size >= part_size_n){
                    point pos = alloc.alloc(node_size);
                    to_block_t new_root_b = buf->load_it(pos);
                    byte *cache_new_root = new_root_b.it->frame + sizeof(node);
                    node *new_root = (node *)(cache_new_root - sizeof(node));
                    new_root->key = root_n->key,    new_root->size = 1;
                    new_root->pos = pos,            new_root->type = 1;
                    new_root->prior = new_root->next = invalid_p;
                    *nth_key_n(cache_new_root) = root_n->key;
                    *nth_point(cache_new_root) = root_n->pos;
                    split_n(root, new_root_b, 0); 
                    root_pos = new_root->pos;
                }
            }
        }
        else{
            if (com(root_n->key, k))
                ret = _insert_l(root, k, v);
            else {
                ret = 2;
                _insert_head_l(root, k, v);
            }
            if (ret == 2){
                buf->dirty(root);
                if (root_n->size >= part_size_l){
                    point pos = alloc.alloc(node_size);
                    to_block_t new_root_b = buf->load_it(pos);
                    byte *cache_new_root = new_root_b.it->frame + sizeof(node);
                    node *new_root = (node *)(cache_new_root - sizeof(node));
                    new_root->key = root_n->key, new_root->size = 1;
                    new_root->pos = pos, new_root->type = 1;
                    new_root->prior = new_root->next = invalid_p;
                    *nth_key_n(cache_new_root) = root_n->key;
                    *nth_point(cache_new_root) = root_n->pos;
                    split_l(root, new_root_b, 0);
                    root_pos = new_root->pos;
                }
            }
        }
        return ret;
    }
    bool remove(const key_t &k)
    {
        if (empty()) return false;
        to_block_t root_b = buf->load_it(root_pos);
        byte *cache = root_b.it->frame + sizeof(node);
        node *root  = (node *)(cache - sizeof(node));
        if (com(k, root->key)) return 0;
        bool ret = true;
        if (root->type) {
            ret = _remove_n(root_b, k);
            if (!ret) return 0;
            if (root->size < 2){
                alloc.free(root->pos, node_size);
                root_b      = buf->load_it(*nth_point(cache));
                root_pos    = ((node *)(root_b.it->frame))->pos;
            }
            else buf->dirty(root_b);
        }
        else {
            ret = _remove_l(root_b, k);
            if (!ret) return 0; 
            buf->dirty(root_b);
        }
        return ret;
    }
    vector<list_t> listof(key_t k, bool (*comp)(const key_t &a, const key_t &b)) const
    {
        if (empty()) return vector<list_t>();
        to_block_t root_b = buf->load_it(root_pos);
        return _listof(root_b, k, comp);
    }
    int size() const
    {
        return num;
    }
    void print_node_l(buf_block_t *it)
    {
        node *tmp = (node *)(it->frame);
        byte *cache = it->frame + sizeof(node);
        printf("values are:\n");
        for (size_t i = 0;i < tmp->size;i++){
            nth_key_l(cache, i);
            *nth_value(cache, i);
        }
        printf("\n");
    }
    void double_check()
    {
        buf->check_hash();
        buf->check_flush();
        buf->check_LRU();
        buf->check_useage();
    }
};

#endif
