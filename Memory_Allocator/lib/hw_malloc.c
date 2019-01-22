#include "hw_malloc.h"
#define HEAP_SIZE 64*1024

chunk_header *alloc;

void *hw_malloc(size_t bytes)
{
    //printf("hw_alloc\n");
    int alloc_size = bytes + 24;//require bytes + 24bytes
    int thresh = 32*1024;
    //heap_alloc or mmap_alloc
    if(alloc_size<thresh) { //heap alloc
        if(start_brk==NULL) {
            //printf("HEAP = NULL\n");
            HEAP_Initialize(alloc_size);
        } else {
            find_best_fit_split(alloc_size);
        }
        //printf("%p\n", alloc);
        printf("0x%012lx\n", (void*)alloc - get_start_sbrk() +24);
        return NULL;

    } else { //mmap_alloc

        //long int size = pow(2, log2(alloc_size)+1);
        //printf("%ld\n", size);
        alloc = mmap(NULL, alloc_size, PROT_READ | PROT_WRITE, MAP_SHARED | MAP_ANONYMOUS, -1, 0);
        printf("%p\n",(void*)alloc+24);
        add_mmap_list(alloc, alloc_size);
        return NULL;

    }
}

int hw_free(void *mem)
{
    //printf("hw_free\n");
    //printf("free = %p\n", mem);
    void* temp = mem - 24;
    long long int t = (long long int)start_brk_init;
    temp = temp +t;
    chunk_header* free = (chunk_header*)temp;
    if(free->mmap_flag == 2) { //from HEAP

        //printf("size of free = %f\n", log2(free->curr_size));
        add_bin_chunk(free, (int)log2(free->curr_size));
        printf("success\n");

    } else {

        free = (void*)free+24;
        if (free->mmap_flag == 1) { //from mmap
            dele_mmap_list(free);
            int test = munmap((void*)free, free->curr_size);
            if(test == 0) {
                printf("success\n");
            } else if (test == 1) {
                printf("fail\n");
            }
        }
    }
    return 0;
}

void *get_start_sbrk(void)
{
    //printf("get_start_sbrk\n");
    return start_brk_init;
}

void HEAP_Initialize(size_t bytes)
{

    start_brk = sbrk(HEAP_SIZE);
    start_brk_init = start_brk;
    //printf("start_brk = %p\n", start_brk);
    init_Bin();
    find_best_fit(bytes);

    return ;
}

void init_Bin()
{

    int i = 0;
    for(i=0; i<=10; i++) {
        Bin[i] = malloc(sizeof(chunk_header));//every bin should be a doubly linklist
        Bin[i]->next = Bin[i];
        Bin[i]->prev = Bin[i];
        Bin[i]->curr_size = sizeof(chunk_header);
        Bin[i]->allo_flag = 0;//not yet alloc
        Bin[i]->mmap_flag = 0;//not yet alloc
    }
}

void find_best_fit(size_t bytes)
{

    int i = 0;
    //void *temp = start_brk;

    size_t t = bytes*2;
    long int size = HEAP_SIZE;

    for(i=15; i>=5; i--) {
        if(size>t) {
            size = size/2;
            //printf("size = %ld\n", size);
            add_bin_chunk(start_brk + size, i);
        } else {
            //printf("the best fit\n");
            alloc= start_brk;
            //alloc->prev->next = alloc->next;
            //alloc->next->prev = alloc->prev;
            alloc->next = NULL;
            alloc->prev = NULL;
            alloc->curr_size = pow(2, i+1);
            alloc->allo_flag = 1;
            alloc->mmap_flag = 2;
            start_brk = start_brk + size;
            //printf("start = %p\n", start_brk);

        }
    }
}

void add_bin_chunk(void* start, int i)
{

    chunk_header* temp = Bin[i-5];
    chunk_header* new = (chunk_header*)start;
    //printf("new = %p\n", new);
    while(temp->next!=Bin[i-5]) {
        temp = temp->next;
    }
    temp->next = new;
    new->next = Bin[i-5];
    new->prev = temp;
    new->pre_size = temp->curr_size;
    new->curr_size = pow(2, i);
    new->allo_flag = 0;
    new->mmap_flag = 0;
    check_merge(start, i-5);
    //printf("Bin[%d]->next = %p\n", i, new);

}

void check_merge(void* start, int i)
{

    //printf("enter check merge\n");
    long int size = ((chunk_header*)start)->curr_size;
    void* temp1 = start+size;
    void* temp2 = start-size;
    chunk_header *find = Bin[i]->next;
    while(find!=Bin[i]) {

        if(find == temp1) {

            //printf("temp1 = %p\n", temp1);
            //printf("find = %p\n", find);
            //printf("go to merge\n");
            if(i!=10) {
                merge(temp1, start, i);//start<temp1
                return;
            }

        } else if (find == temp2) {

            //printf("temp2 = %p\n", temp2);
            //printf("find = %p\n", find);
            //printf("go to merge\n");
            if(i!=10) {
                merge(start, temp2, i);//start>temp2
                return;
            }
        }

        find = find->next;

    }

}

void merge(void* H, void* L, int i)
{

    delet_bin_chunk(H);
    delet_bin_chunk(L);
    add_bin_chunk(L, i+1+5);

}


void find_best_fit_split(size_t bytes)
{

    int i = 0;
    long int size = bytes*2;
    int fit = 0;
    int best_fit = 0;
    for(i=15; i>4; i--) {
        if(pow(2, i)<=size) {
            fit = i-5;
            best_fit = fit;
            break;
        }
    }

    void *first_chunk = Bin[fit]->next;
    while(first_chunk == Bin[fit]) {
        //printf("Bin[%d] is NULL\n", fit);
        fit = fit + 1;
        first_chunk = Bin[fit]->next;
    }
    chunk_header *temp = first_chunk;
    chunk_header *addr_low = first_chunk;
    /*find the lowest one*/
    while(temp->next!=Bin[fit]) { //not the last one
        if(temp<addr_low) {
            addr_low = temp;
            //printf("low_addr = %p\n", addr_low);
        }
        temp = temp->next;
    }
    delet_bin_chunk(addr_low);
    if(fit!=best_fit) {
        split(addr_low, fit, best_fit);
        alloc = addr_low;
        alloc->curr_size = pow(2, best_fit+5);
        alloc->pre_size = 0;
        alloc->allo_flag = 1;
        alloc->mmap_flag = 2;
    } else {
        alloc = addr_low;
        alloc->curr_size = pow(2, best_fit+5);
        alloc->pre_size = 0;
        alloc->allo_flag = 1;
        alloc->mmap_flag = 2;

    }

}

void delet_bin_chunk(chunk_header* del)
{

    del->next->prev = del->prev;
    del->prev->next = del->next;
    del->next = NULL;
    del->prev = NULL;
}

void split(void *chunk_split, int f, int B_f)
{

    int i = 0;
    long int size = pow(2, f+5);
    for(i=f; i>B_f; i--) {
        size = size/2;
        add_bin_chunk(chunk_split+size, i - 1 + 5);
    }

}

void print_Bin(int i)
{

    chunk_header *temp = Bin[i]->next;
    while(temp != Bin[i]) {

        printf("0x%012lx--------%ld\n", (void*)temp - get_start_sbrk(), temp->curr_size);
        temp = temp->next;
    }

}


void add_mmap_list(chunk_header* alloc, long int size)
{

    /*initial mmap_list*/
    if(mmap_head == NULL) {

        mmap_head = malloc(sizeof(chunk_header));//every bin should be a doubly linklist
        mmap_head->next = mmap_head;
        mmap_head->prev = mmap_head;
        mmap_head->mmap_flag = 0;
        mmap_head->allo_flag = 0;

    }
    chunk_header* temp = mmap_head->next;
    chunk_header* new = alloc;
    new->curr_size = size;
    new->mmap_flag = 1;
    new->allo_flag = 1;
    chunk_header* lowest = temp;
    while(temp->next!=mmap_head) {

        if(lowest->curr_size>=temp->curr_size) {

            lowest = temp;

        }
        temp = temp->next;

    }
    new->next = lowest->next;
    lowest->next->prev = new;
    lowest->next = new;
    new->prev = lowest;

}

void print_mmap_list()
{

    chunk_header* temp = mmap_head->next;
    while(temp!=mmap_head) {

        printf("%p--------%ld\n", (void*)temp, temp->curr_size);
        temp = temp->next;

    }

}

void dele_mmap_list(chunk_header* free)
{

    chunk_header* temp = mmap_head->next;
    while(temp!=mmap_head) {

        if(temp==free) {
            break;
        }

    }
    temp->prev->next = temp->next;
    temp->next->prev = temp->prev;
    temp->next = NULL;
    temp->prev = NULL;

}
