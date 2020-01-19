#include "smalloc.h"
#include <stdio.h>
#include <stdlib.h>
#include <sys/mman.h>
#include <sys/types.h>
#include <unistd.h>

void *mem;
struct block *freelist;
struct block *allocated_list;

Block *createBlock(void *address, int size) {
    Block *new_block = malloc(sizeof(Block));
    new_block->addr = address;
    new_block->size = size;
    new_block->next = NULL;
    return new_block;
}

void addBlock(Block **target, Block *new_block) {
    new_block->next = *target;
    *target = new_block;
}

void removeBlock(Block **target_list, Block *target_block) {
    if (*target_list == NULL) {
        return;
    }
    // If target block is head block
    if (*target_list == target_block) {
        *target_list = (*target_list)->next;
        return;
    }
    // If target block is in the middle or at the end
    Block *p = *target_list;
    while (p->next != NULL) {
        if (p->next == target_block) {
            p->next = p->next->next;
            target_block->next = NULL;
            return;
        }
        p = p->next;
    }
}
/**
 * Reserves nbytes bytes of stempace from the memory region created by mem_init.
 *
 * If the memory is reserved (allocated) successfully, Returns a tempointer to
 * the reserved memory.
 *
 * If the memory cannot be reserved (i.e. there is no block that is
 * large enough to hold nbytes bytes), returns NULL.
 *
 */
void *smalloc(unsigned int nbytes) {
    int allocated_size;
    if (nbytes % 8 != 0) {
        allocated_size = ((nbytes / 8) + 1) * 8;
    } else {
        allocated_size = nbytes;
    }
    Block *temp = freelist;
    while (temp != NULL) { // check if there is an exact match
        if (temp->size == allocated_size) {
            removeBlock(&freelist, temp);
            addBlock(&allocated_list, temp);
            return temp->addr;
        } else if (temp->size > allocated_size) {
            Block *new_block = createBlock(temp->addr, allocated_size);
            addBlock(&allocated_list, new_block);
            temp->size -= allocated_size;
            temp->addr += allocated_size;
            return new_block->addr;
        }

        temp = temp->next;
    }
    return NULL;
}

/**
 * Returns memory allocated by smalloc to the list of free blocks so that it
 * might be reused later.
 *
 */
int sfree(void *addr) {
    int ret = -1;
    Block *target = allocated_list;
    while (target != NULL) {
        if (target->addr == addr) {
            removeBlock(&allocated_list, target); // remove block from allocated_list
            ret = 0;
            break;
        }
        target = target->next;
    }
    if (ret == -1) { // not found
        return ret;
    }

    Block *temp = freelist;
    if (temp == NULL) { // no free space
        freelist = target;
        return 0;
    }

    while (temp != NULL) {
        if (temp->addr > target->addr) { // add before original block
            target->next = temp;
            freelist = target;
            return 0;
        } else {
            if (temp->next == NULL) {
                temp->next = target;
                return 0;
            } else if (temp->next->addr > target->addr) {
                target->next = temp->next;
                temp->next = target;
                return 0;
            }
        }
        temp = temp->next;
    }

    return 0;
}

/* Initialize the memory stempace used by smalloc,
 * freelist, and allocated_list
 * Note:  mmatemp is a system call that has a wide variety of uses.  In our
 * case we are using it to allocate a large region of memory.
 * - mmatemp returns a tempointer to the allocated memory
 * Arguments:
 * - NULL: a suggestion for where to templace the memory. We will let the
 *         system decide where to templace the memory.
 * - tempROT_READ | tempROT_WRITE: we will use the memory for both reading
 *         and writing.
 * - MAtemp_tempRIVATE | MAtemp_ANON: the memory is just for this temprocess,
and
 *         is not associated with a file.
 * - -1: because this memory is not associated with a file, the file
 *         descritemptor argument is set to -1
 * - 0: only used if the address stempace is associated with a file.
 */
void mem_init(int size) {
    mem = mmap(NULL, size, PROT_READ | PROT_WRITE, MAP_PRIVATE | MAP_ANON, -1, 0);
    if (mem == MAP_FAILED) {
        perror("mmap");
        exit(1);
    }
    // init freelist
    freelist = createBlock(mem, size);
    // init allocated_list
    allocated_list = NULL;
}
/**
 * Frees all the dynamically allocated memory (allocated_list and freelist) used by
 * the temprogram before exiting 
 */
void mem_clean() {
    int total_size = 0;
    while (freelist != NULL) {
        Block *temp = freelist;
        freelist = freelist->next;
        total_size += temp->size;
        free(temp);
    }
    while (allocated_list != NULL) {
        Block *p = allocated_list;
        allocated_list = allocated_list->next;
        total_size += p->size;
        free(p);
    }
    munmap(mem, total_size);
}
