#ifndef SJTU_ALLOC_HPP
#define SJTU_ALLOC_HPP
#include <stdio.h>
#include "exceptions.h"
using point = long;
template<size_t node_size = 65536>
class ALLOC{
    struct node{
        node            *next, *prior;
        point           loc[2];
		size_t          size;
        // int type;
        node(int l1 = 0, int l2 = 0, node *p = nullptr, node *n = nullptr):next(n), prior(p){
            loc[0] = l1, loc[1] = l2;size = l2 - l1;
        }
    };
    node                *head;
    point               file_end;
    const int           new_node_num = 8;
    void remove(node *n){
        if (n->prior != nullptr) n->prior->next = n->next;
        else head = n->next;
        if (n->next != nullptr) n->next->prior = n->prior;
    }
    //alloc new_node_num nodes sized node_size at the end of the file
    void new_mem(node *n){
        node *tmp = n;
        tmp->next = new node(file_end, file_end + node_size, tmp);
        tmp = tmp->next;
        for (int i = 1;i < new_node_num;i++){
            tmp->next = new node(tmp->loc[1], tmp->loc[1] + node_size, tmp);
            tmp = tmp->next;
        }
        file_end += node_size * new_node_num;
    }
    void clear_node(node *n){
		/*
        if (n != nullptr){
            clear_node(n->next);
            delete n;
        }
		*/
		while (n != nullptr) {
			node *temp = n;
			n = n->next;
			delete temp;
		}
    }
public:
    ALLOC():head(nullptr),file_end(){}
    ~ALLOC(){
        clear_node(head);
    }
    void refill(const char *filename){
        file_end = 0;
		clear_node(head);
        head = new node;
        new_mem(head);
        head = head->next;
        delete head->prior;
        head->prior = nullptr;
        save(filename);
    }
    void init(const char *filename){
        FILE *file;
        file = fopen(filename, "rb");
        if (!file) {
            refill(filename);
            return;
        }
        if (!fread(&file_end, sizeof(point), 1, file)){
            refill(filename);
            return;
        }
        node *temp = head = new node;
        while(!feof(file)){
            fread(temp->loc, sizeof(point), 2, file);
            // printf("%d %d ",temp->loc[0], temp->loc[1]);
            temp->next = new node;
            temp->next->prior = temp;
            temp = temp->next;
        }
        temp = temp->prior;delete temp->next;temp->next = nullptr;
        temp = temp->prior;delete temp->next;temp->next = nullptr;
        fclose(file);
    }
    void save(const char *filename){
        FILE *file;
        file = fopen(filename, "wb");
        if (!file) throw(runtime_error());
        fwrite(&file_end, sizeof(point), 1, file);
        node *temp = head;
        while(temp != nullptr){
            fwrite(temp->loc, sizeof(point), 2, file);
            // printf("%d %d ",temp->loc[0], temp->loc[1]);
            temp = temp->next;
        }
        fclose(file);
    }
    point alloc(size_t s){
        node *temp = head;
        point p;
        if (s > node_size){
            p = file_end;
            file_end += s;
            return p;
        }
        if (temp == nullptr){
            temp = new node;
            new_mem(temp);
            temp = head = temp->next;
            delete temp->prior;
        }
        while(temp->next != nullptr && temp->size <= s) temp = temp->next;
        if (temp->size <= s){
            new_mem(temp);
			temp = temp->next;
            p = temp->loc[0];
            temp->loc[0] += s;
			temp->size -=s;
        }
        else{
            p = temp->loc[0];
            temp->loc[0] += s;
			temp->size -= s;
            if (!(temp->size)) remove(temp);
        }
        return p;
    }
    void free(point p, size_t s){
        node *temp = head;
        for (;temp != nullptr;temp = temp->next){
            if (temp->loc[1] >= p + s){
                if (temp->loc[0] == p + s) {temp->loc[0] = p;temp->size += s;}
                else{
                    node *n = new node(p, p + s, temp->prior, temp);
                    if (temp->prior == nullptr) head = n;
                    else {
                        if (temp->prior->loc[1] == p) {
                            temp->prior->loc[1] = p + s;
							temp->prior->size += s;
                            delete n;
                            return;
                        }
                        else temp->prior->next = n;
                    }
                    temp->prior = n;
                }
            break;
            }
        }
    }
};
#endif
