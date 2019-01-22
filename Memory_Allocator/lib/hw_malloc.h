#ifndef HW_MALLOC_H
#define HW_MALLOC_H
#include<string.h>
#include<stdlib.h>
#include<stdio.h>
#include<unistd.h>
#include<math.h>
#include<sys/mman.h>

struct node {
    int data;
    struct node *next;
    struct node *prev;
    long int pre_size;
    long int curr_size;
    int allo_flag;//0 for free, 1 for alloc
    int mmap_flag;//0 for not yet alloc, 1 for mmap, 2 for heap
};

typedef struct node chunk_header;

void *hw_malloc(size_t bytes);
int hw_free(void *mem);
void *get_start_sbrk(void);
void HEAP_Initialize(size_t bytes);
void init_Bin();
void find_best_fit(size_t bytes);
void add_bin_chunk(void* start, int i);
void find_best_fit_split(size_t bytes);
void delet_bin_chunk(chunk_header* del);
void split(void *chunk_split, int f, int B_f);
void print_Bin(int i);
void check_merge(void* start, int i);
void merge(void* H, void* L, int i);
void add_mmap_list(chunk_header* alloc, long int size);
void print_mmap_list();
void dele_mmap_list(chunk_header* free);





chunk_header* Bin[11];
chunk_header* mmap_head;
void *start_brk;
void *start_brk_init;

#endif
