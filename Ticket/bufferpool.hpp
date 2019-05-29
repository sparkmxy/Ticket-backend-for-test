//in this version, free list has no use, since the only element to free is mutex.
//in this version, old_list has no use, only a simple LRU is done.(all elements are moved to HIR)
//in the next version, a queue is needed in order to change LRU to LIRS
#ifndef SJTU_BUFFER_POOL_HPP
#define SJTU_BUFFER_POOL_HPP


#include <iostream>
#include <stdio.h>
#include "utility.hpp"
#include "hash.hpp"

using lathe = size_t;

//block of a buffer
struct buf_block_t{
    using byte  = char;
    using point = long;
    byte                        *frame;
    point                       offset;

    //state = 0 when it is free, 1 when it is in HIR, 2 for in LIR and 3, 4 for it is dirty in such a list
    int                         state;
    //all info about hash
    int                         hash_value;
    buf_block_t                 *hash_next;
    //its location at three list
    ut_list_node<buf_block_t>   free, LRU, flush;
    long                        IRR, recency;
    lathe                       RW_lathe;

    buf_block_t(byte *f = nullptr):frame(f), offset(0), RW_lathe(),
                                state(0), IRR(0), recency(),
                                hash_value(0), hash_next(nullptr),
                                free(), LRU(), flush(){}
    ~buf_block_t(){}
    
};

struct to_block_t{
    buf_block_t *it;
    to_block_t(buf_block_t *other = nullptr):it(other){
        if (it != nullptr) ++it->RW_lathe;
    }
    to_block_t(const to_block_t &other):it(other.it){
        if (it != nullptr) ++it->RW_lathe;
    }
    ~to_block_t(){
        if (it != nullptr) --it->RW_lathe;
    }
    to_block_t operator=(const to_block_t &other){
        if (it != nullptr) --it->RW_lathe;
        it = other.it;
        ++it->RW_lathe;
        return *this;
    }
    to_block_t operator=(to_block_t &&other){
        if (it != nullptr) --it->RW_lathe;
        it = other.it;
        other.it = nullptr;
        return *this;
    }
};
#ifndef BUF_POOL_TOTAL_NUM
    #define BUF_POOL_TOTAL_NUM 1100
#endif
#ifndef BUF_COLD_PERCENTAGE
    #define BUF_COLD_PERCENTAGE 12
#endif
template<size_t BUFFER_SIZE = 4096>
class buf_pool_t{
    using byte  = char;
    using point = long;
    byte                                *mem_head;
    ut_list_head<buf_block_t>           free, LRU, flush;
    long                                clock;
    hash_table_t<buf_block_t*>          hash_table;
    buf_block_t                         *HIR_head;
    FILE                                *f;
    
    /*
        * a basic function in order to know whether the needed key exists.
        * even the page is found, its clock is not changed
    */
    buf_block_t *_find(const point &pos) const{
        buf_block_t *temp = hash_table.find(pos);
        while(temp != nullptr && temp->offset != pos) temp = temp->hash_next;
        return temp;
    }
    /*
        * get the oldest avaliable page in LRU_list
    */
    buf_block_t *_LRU_oldest(){
        buf_block_t *tmp = LRU.end;
        while(tmp != nullptr){
            if (tmp->state < 3 && !(tmp->RW_lathe)) break;
            tmp = (tmp->LRU).prev;
        }
        if (tmp == nullptr){
            if (flush.count){
                flush_all();
                return _LRU_oldest();
            }
            else{printf("error: buffer_pool is too small\n");throw runtime_error();}
        }
        return tmp;
    }
    //the following functions are movements of LRU-list: insert or remove
    inline void _pick_out_hash(buf_block_t *it){
        buf_block_t *pre = hash_table.find(it->offset);
        if (pre == it){
            hash_table.insert(it->offset, it->hash_next);
        }
        else{
            while(pre->hash_next != it) pre = pre->hash_next;
            pre->hash_next = it->hash_next;
        }
        it->hash_next = nullptr;
    }
    /*
        * pick out a block from the LRU-list.(simply change the pointer to it)
        * HIR_head is considered
    */
    inline void _pick_out_LRU(buf_block_t *it){
        if (it == HIR_head) HIR_head = it->LRU.next;
        if (it->LRU.next != nullptr) (it->LRU.next)->LRU.prev = it->LRU.prev;
        else {
            LRU.end = it->LRU.prev;
        }
        if (it->LRU.prev != nullptr) (it->LRU.prev)->LRU.next = it->LRU.next;
        else {
            LRU.start = it->LRU.next;
        }
        it->LRU.prev = it->LRU.next = nullptr;
        //shall we really write this?
    }
    /*
        * can only be used while *it is a clean buf_block not in LRU. 
        * get info from the storage, and move it to LRU-old-head.
        * DO NOT CHANGE LRU.COUNT BUT it->STATE = 1.
    */
    inline void _to_HIR(buf_block_t *it){
        if (it->state < 3) it->state = 1;
        else if (it->state == 4) it->state = 3;
        if (HIR_head != nullptr) {
            (it->LRU).prev = (HIR_head->LRU).prev;
            (it->LRU).next = HIR_head;
            HIR_head->LRU.prev = it;
            if (HIR_head = LRU.start) LRU.start = it;
            HIR_head = it;
            return;
        }
        else{
            //all nodes are young or no node exists
            if (LRU.end != nullptr){
                (it->LRU).prev = LRU.end;
                HIR_head = LRU.end = it;
            }
            else{
                LRU.start = LRU.end = HIR_head = it;
            }
        }
    }
        /*
            * certainly p must be in the lru list before this.(young or old)
            * make it the first of lru-list as well as a LIR one. 
            * re-consider-needed: pointer lru-old; oldest in LIR set 
        */
        //done; re-consider needed
        // void _to_LIR(buf_block_t &p){
        //     _pick_out_LRU(p);
        //     if ((p.state == 1) || (p.state == 3)) ++p.state;     
        //     p.LRU.prev = nullptr, p.LRU.next = LRU.start;
        //     LRU.start = &p;
        //     p.recency = ++clock;
        //     //to check if the oldest in LIR set should be removed to the cold list(HIR)?
        //     //or instead of cold list, flush it?
        // }
    /*
        * make it the last of LRU-list.
    */
    inline void _to_LRU_HIR_end(buf_block_t *it){
        _pick_out_LRU(it);
        if (it->state == 2 || it->state == 4) --(it->state);
        it->LRU.next = nullptr, it->LRU.prev = LRU.end;
        if (HIR_head == nullptr) HIR_head = it;
        if (LRU.start == nullptr) LRU.start = it;
        LRU.end = it;
    }
        // void _to_LRU_LIR_end(buf_block_t &p){
        //     _pick_out_LRU(p);
        //     if (p.state == 1 || p.state == 3) ++p.state;
        //     if (HIR_head == nullptr) {
        //         p.LRU.prev = LRU.end;
        //         LRU.end = &p;
        //         if (LRU.start == nullptr) LRU.start = &p;
        //     }
        //     else {
        //         p.LRU.prev = HIR_head->LRU.prev;
        //         if (LRU.start == HIR_head) LRU.start = &p;
        //     }
        //     p.LRU.next = HIR_head;
        // }
    //the following are those involving i/o
    /*
        * read info and make the page the HIR_head
    */
    inline void _read_inf(buf_block_t *it, const point &pos, FILE *f){
        fseek(f, pos, SEEK_SET);
        fread(it->frame, 1, BUFFER_SIZE, f);
        it->offset = pos;
        it->hash_value = hash_table.get_value(pos);
        it->hash_next = hash_table.find(pos);
        hash_table.insert(pos, it);
        if (it->state) _pick_out_LRU(it);
        else {
            ++LRU.count;it->state = 1;
        }
        _to_HIR(it);
    }
    /*
        * fulfill a free page and move it to the LRU-list
    */
    inline buf_block_t *free_use(const point &pos, FILE *f){
        buf_block_t *tmp = free.start;
        free.start = (tmp->free).next;
        if (free.start == nullptr) free.end = nullptr;
        else free.start->free.prev = nullptr; // do we really need this sentence?
        (tmp->free).next = nullptr;
        --free.count;
        _read_inf(tmp, pos, f);
        return tmp;
    }
    /*
        * rewrite a page in LRU_page and make it the HIR_head
    */
    inline buf_block_t *LRU_use(const point &pos, FILE *f){
        buf_block_t *tmp = _LRU_oldest();
        _pick_out_hash(tmp);
        _read_inf(tmp, pos, f);
        return tmp;
    }
    /*
        * it must be in flush queue
        * flush-back to the storage
        * remove it from the flush queue
    */
    inline void flush_back(buf_block_t *it, FILE *f){
        if (it->flush.next != nullptr) (it->flush.next)->flush.prev = it->flush.prev;
        else flush.end = it->flush.prev;
        if (it->flush.prev != nullptr) (it->flush.prev)->flush.next = it->flush.next;
        else flush.start = it->flush.next;
        if (flush.start == nullptr) flush.end = nullptr;
        else if (flush.end == nullptr) flush.start = nullptr;
        fseek(f, it->offset, SEEK_SET);
        fwrite(it->frame, 1, BUFFER_SIZE, f);
            (it->state) -= 2;
            --flush.count;
    }
    //mode = 0 for free_list, 1 for lru list and 2 for flush list
    inline void flush_all(){
        for (auto temp = flush.start;temp != nullptr;temp = (temp->flush).next){
            fseek(f, temp->offset, SEEK_SET);
            fwrite(temp->frame, 1, BUFFER_SIZE, f);
            if ((temp->flush).prev != nullptr){
                (((temp->flush).prev)->flush).next = nullptr;
                (temp->flush).prev = nullptr;
            }
            (temp->state) -= 2;
        }
        flush.start = flush.end = nullptr;
        flush.count = 0;
    }
    inline void clean_connection_list(){
        int n = 0;
        for (auto temp = free.start;temp != nullptr;temp = (temp->free).next){
            if ((temp->free).prev != nullptr){
                (((temp->free).prev)->free).next = nullptr;
                delete (temp->free).prev;
                (temp->free).prev = nullptr;
            }
            n++;
        }
        n = 0;
        for (auto temp = LRU.start;temp != nullptr;temp = (temp->LRU).next){
            if ((temp->LRU).prev != nullptr){
                (((temp->LRU).prev)->LRU).next = nullptr;
                delete (temp->LRU).prev;
                (temp->LRU).prev = nullptr;
            }
            n++;
        }
        n = 0;
    }
public:
    buf_pool_t():   f(),
                    free(), LRU(), flush(), HIR_head(nullptr),
                    clock(0),
                    hash_table(nullptr){
        mem_head = new byte[BUFFER_SIZE * (BUF_POOL_TOTAL_NUM + 1)];
        buf_block_t *temp;
        free.count = BUF_POOL_TOTAL_NUM;
        free.start = free.end = new buf_block_t(mem_head);
        for (size_t i = 1;i < BUF_POOL_TOTAL_NUM;i++){
            (free.end)->free.next = temp = new buf_block_t(mem_head + BUFFER_SIZE * i);
            (temp->free).prev = free.end;
            free.end = temp;
        }
    }
    void init(FILE *fil){
        f = fil;
        flush_all();
        clean_connection_list();
        buf_block_t *temp;
        free.count = BUF_POOL_TOTAL_NUM;
        free.start = free.end = new buf_block_t(mem_head);
        for (size_t i = 1;i < BUF_POOL_TOTAL_NUM;i++){
            (free.end)->free.next = temp = new buf_block_t(mem_head + BUFFER_SIZE * i);
            (temp->free).prev = free.end;
            free.end = temp;
        }
    }

    ~buf_pool_t(){
        flush_all();
        clean_connection_list();
        delete[] mem_head;
    }
    void file_change(FILE *fil){
        f = fil;
    }
    void re_init(){
        for (auto temp = flush.start;temp != nullptr;temp = (temp->flush).next){
            if ((temp->flush).prev != nullptr){
                (((temp->flush).prev)->flush).next = nullptr;
            }
            (temp->flush).prev = nullptr;
            (temp->state) -= 2;
        }
        flush.start = flush.end = nullptr;
        flush.count = 0;
        for (auto temp = LRU.start;temp != nullptr;temp = (temp->LRU).next){
            if ((temp->LRU).prev != nullptr){
                (((temp->LRU).prev)->free.next) = temp;
                (((temp->LRU).prev)->LRU.next) = nullptr;
            }
            temp->free.prev = temp->LRU.prev;
            temp->LRU.prev = nullptr;
            temp->state = 0;
        }
        if (free.end != nullptr)
            free.end->free.next = LRU.start;
        else free.start = LRU.start;
        if (LRU.start != nullptr)
            LRU.start->free.prev = free.end;
        free.end = LRU.end;
        free.count = BUF_POOL_TOTAL_NUM;
        LRU.count = 0, LRU.start = LRU.end = nullptr;
    }
    to_block_t load_it(const long offset) {
        //in a further mode, if it is used(a x-lathe added), spin and wait.
        buf_block_t *it = _find(offset);
        if (it != nullptr) {
            _pick_out_LRU(it);
            _to_HIR(it);
            return to_block_t(it);
        }
        if (free.count) return to_block_t(free_use(offset, f));
        if (LRU.count > flush.count) return to_block_t(LRU_use(offset, f));
        flush_all();
        return to_block_t(LRU_use(offset, f));
    }
    void dirty(to_block_t &to_it){
        buf_block_t *it = to_it.it;
        if (it->state > 2) return;
        //if (flush.start != it){//do we need this? maybe not
            it->flush.next = flush.start;
            if (flush.start != nullptr) flush.start->flush.prev = it;
            else flush.end = it;
            flush.start = it;
        //}
        (it->state) += 2;
        ++flush.count;
    }

    void check_hash(){
        for (int i = 0;i < 512;i++){
            buf_block_t *now = hash_table.find(i * 4096);
            check_hash_point(now, i);
        }
    }
    void check_hash_point(buf_block_t *it, int k){
        if (it == nullptr) return;
        if (hash_table.get_value(it->offset) != k){
            printf("wrong_hash_checking: wrong offset ");
            throw runtime_error();
        }
        int tmp = it->state;
        if (tmp != -1) it->state = -1;
        else {
            printf("wrong_hash_checking: a circle ");
            throw runtime_error();
        }
        check_hash_point(it->hash_next, k);
        it->state = tmp;
    }
    void check_flush(){
        check_flush_point(flush.start);
    }
    void check_flush_point(buf_block_t *it){
        if (it == nullptr) return;
        if (it->state < 0){
            printf("wrong_flush_checking: circle ");
            throw runtime_error();
        }
        int tmp = it->state;
        it->state = -1;
        check_flush_point((it->flush).next);
        it->state = tmp;
    }
    void check_LRU(){
		if (HIR_head == nullptr) return;
        if ((HIR_head->LRU).prev != nullptr) {
            printf("wrong_LRU_checking: HIR_head ");
            throw runtime_error();
        }
        check_LRU_point(LRU.start);
    }
    void check_LRU_point(buf_block_t *it, size_t num = 0){
        if (it == nullptr) return;
        if (it->state <= 0){
            printf("wrong_LRU_checking: circle ");
            throw runtime_error();
        }
        if (num > LRU.count){
            printf("wrong_LRU_checking: oversize ");
            throw runtime_error();
        }
        int tmp = it->state;
        it->state = -1;
        check_LRU_point((it->LRU).next, num + 1);
        it->state = tmp;
    }
    void check_useage(){
        check_useage(LRU.start);
    }
    void check_useage(buf_block_t *it){
        if (it == nullptr) return;
        if (it->RW_lathe){
            printf("wrong_useage_checking: still in use now ");
            throw(runtime_error());
        }
        check_useage(it->LRU.next);
    }
};

#endif
