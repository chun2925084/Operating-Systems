#include "./lib/hw_malloc.h"
#include "hw4_mm_test.h"

int main(int argc, char *argv[])
{
    int num = 0;
    char option[100];
    void *mem;
    while(scanf("%s", option)!=EOF) {
        if(!strcmp(option, "alloc")) {
            if(scanf("%d", &num))
                hw_malloc(num);
        } else if(!strcmp(option, "free")) {
            if(scanf("%p", &mem))
                hw_free(mem);
        } else if(!strcmp(option, "print")) {
            if(scanf(" bin[%d]", &num)) {
                //printf("Print the bin[i]\n");
                print_Bin(num);
            } else if(scanf("%s",option)) {
                if(!strcmp(option, "mmap_alloc_list")) {
                    //printf("Printf mmap_list\n");
                    print_mmap_list();
                }
            }
        } else {
            printf("Error Input\n");
        }
    }
    return 0;
}
