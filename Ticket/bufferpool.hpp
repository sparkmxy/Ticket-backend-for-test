//in this version, free list has no use, since the only element to free is mutex.
//some re-consider needed
#ifndef SJTU_BUFFER_POOL_HPP
#define SJTU_BUFFER_POOL_HPP
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
	size_t count;
	TYPE* start;
	TYPE* end;
    ut_list_head(TYPE* p = nullptr, TYPE* n = nullptr, size_t c):
        prev(p), next(n), count(c){}
};
//block of a buffer
struct buf_block_t{
    using byte = char;
    byte                        *frame;
    size_t                      offset;

    //state = 0 when it is free, 1 when it is in lru list and 2 when it is dirty
    size_t                      state;
    //all info about hash
    int                         hash_value;
    buf_block_t                 *hash_next;
    //its location at three list
    ut_list_node<buf_block_t>   free, LRU, flush;
    size_t                      LRU_pos;

    buf_block_t(byte *f = nullptr):frame(f), offset(0), 
                                state(0), LRU_pos(0),
                                hash_value(0), hash_next(nullptr),
                                free(), LRU(), flush(){}
};
template<size_t BUFFER_SIZE = 4096, size_t TOTAL_NUM = 256>
class buf_pool_t{
    using byte = char;
    byte                                *mem_head;
    ut_list_head<buf_block_t>           free, LRU, flush;
    size_t                              clock;
    hash_table_t<int, buf_block_t*>     hash_table;
    buf_block_t                         old_LRU;
    
    //can only be used while p is a clean buf_block
    //done
    void _read_inf(buf_block_t &p, const size_t &pos, FILE *f){
        fseek(f, pos, SEEK_SET);
        fread(p.frame, 1, BUFFER_SIZE, f);
        p.LRU_pos = ++clock, p.state = 1, p.offset = pos;
        p.hash_value = hash_table.get_value(pos);
        p.hash_next = nullptr;
        buf_block_t *temp = hash_table.find(pos);
        if (temp == nullptr) p.hash_next = temp;
        hash_table.insert(pos, &p);
    }
    //certainly p must be in the lru list
    //done; re-consider
    void young(buf_block_t &p){
        if (p.LRU.next != nullptr) (p.LRU.next)->LRU.prev = p.LRU.prev;
        else LRU.end = p.LRU.prev;
        if (p.LRU.prev != nullptr) (p.LRU.prev)->LRU.next = p.LRU.next;
        else LRU.start = p.LRU.next;
        if (LRU.start == nullptr) LRU.end = nullptr;
        if (LRU.end == nullptr) LRU.start = nullptr;
        p.LRU.prev = nullptr, p.LRU.next = LRU.start;
        LRU.start = &p;
        p.LRU_pos = ++clock;
        //to check if the oldest in hot set should be removed to the cold list?
        //or instead of cold list, flush it?
    }
    //done
    buf_block_t *has_it(const size_t &pos){
        buf_block_t *temp = hash_table.find(pos);
        while(temp != nullptr && temp->offset != pos) temp = temp->hash_next;
        return temp;
    }
    //it must be in flush queue
    //done
    void flush_back(buf_block_t &it, FILE *f){
        if (it.flush.next != nullptr) (it.flush.next)->flush.prev = it.flush.prev;
        else flush.end = it.flush.prev;
        if (it.flush.prev != nullptr) (it.flush.prev)->flush.next = it.flush.next;
        else flush.start = it.flush.next;
        if (flush.start == nullptr) flush.end = nullptr;
        if (flush.end == nullptr) flush.start = nullptr;
        it.state = 1;
        fseek(f, it.offset, SEEK_SET);
        fwrite(it.frame, 1, BUFFER_SIZE, f);
    }
public:
    buf_pool_t():free(), LRU(), flush(),old_LRU(nullptr)
                clock(0),
                hash_table(){
        mem_head = new byte[BUFFER_SIZE * (TOTAL_NUM + 1)];
        free.count = TOTAL_NUM;
        free.start = free.end = new buf_block_t(mem_head);
        buf_block_t *temp;
        for (size_t i = 1;i < TOTAL_NUM;i++){
            (free.end)->free.next = temp = new buf_block_t(mem_head + BUFFER_SIZE * i);
            free.end = temp;
        }
    }


    buf_block_t *load_it(const size_t &pos, FILE *f){
        //in a further mode, if it is used(a x-lathe added), spin and wait.
        buf_block_t *temp = has_it(pos);
        if (temp != nullptr) return temp;
        if (free.count) 
    }

    
};

#endif