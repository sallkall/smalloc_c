#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/mman.h>
#include "smalloc.h"

/* Functions to print the datastructures used by smalloc */
extern void *mem;
extern struct block *allocated_list;
extern struct block *freelist;

/* Checks the increasing address order of list */
void check_order(struct block *list) {
    while (list->next != NULL) {
        if (list->addr > list->next->addr) {
            printf("[FAILED] freelist address order error found at [addr: %p]\n", list->addr);
        }
        list = list->next;
    }
    printf("[PASSED] freelist address order is correct!\n");
}

/* print the result of check_order */
void print_order_result(void) { 
    check_order(freelist); 
}

/* Prints each element of the list using the format string given below:*/
void print_list(struct block *list) {
    while (list != NULL) {
        printf("    [addr: %p, size: %d]\n", list->addr, list->size);
        printf("\n");
        list = list->next;
    }
}

void print_allocated(void) {
    print_list(allocated_list);
}

void print_free(void) {
    print_list(freelist);
}

/* write value size times to memory starting at ptr */
void write_to_mem(int size, char *ptr, char value) {
    int i = 0;
    for(i = 0; i < size; i++) {
        ptr[i] = value;
    }
}

/* Prints the contents of allocated memory. Each byte is printed as two
 * hexadecimal digits. */
void print_mem(void) {
    struct block *cur = allocated_list;
    
    while(cur != NULL) {
        printf("%p: size = %d\n", cur->addr, cur->size);
        
        /* print 16 bytes per line */
        int i, j;
        for(i = 0; i < cur->size / 8; i++){
            if(i % 2 == 0){
                printf("%5d:  ", i * 8);
            } else {
                printf("  ");
            }
            for(j = 0; j < 8; j++) {
                printf("%02x ", *((char *)cur->addr + ((i*8) + j)));
            }
            if((i+1) % 2 == 0 && ((i+1) * 8) != cur->size){
                printf("\n");
            }
        }
        printf("\n");
        cur =cur->next;
    }
    printf("\n");
}
