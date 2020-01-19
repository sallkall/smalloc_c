#include "smalloc.h"
#include <stdio.h>
#include <stdlib.h>
#include <sys/mman.h>
#include <sys/types.h>
#include <unistd.h>

#define SIZE 72

#define ALLOC 2

/* Interesting test case for smalloc and sfree.
 *
 * SIZE = 64    3 blocks sized 14, 20, 30, bytes each ==> 14+20+30 = 64 (at a galance this seems like full allocation)
 * But since memory is aligned on an 8-byte boundary, it will end up asking for more bytes:
 * 14 -> 16
 * 20 -> 24
 * 30 -> 32
 * which is an additional 8 byte, so we need SIZE = 64 + 8 = 72 in order to have enough memory to allocate
 *
 * Then checks to see if smalloc will not allocate more memory when all has been allocated
 *
 * Also checks that sfree considers the case of returning a block with the largest address to the very end of freelist
 *
 * Checks if freelist is by increasing address order
 *
 * Lastly, will call smalloc to allocate one block after memory had been freed
 *
 */

int main(void) {
    mem_init(SIZE);
    char *ptrs[3];
    ptrs[0] = smalloc(14);
    ptrs[1] = smalloc(20);
    ptrs[2] = smalloc(30);

    write_to_mem(14, ptrs[0], 0);
    write_to_mem(20, ptrs[1], 1);
    write_to_mem(30, ptrs[2], 2);

    printf("List of allocated blocks:\n");
    print_allocated();
    printf("List of free blocks:\n"); // should only have 2 bytes left
    print_free();
    printf("Contents of allocated memory:\n"); // should have 3 blocks
    print_mem();

    // Still have 2 bytes of memory left, tries to allocate for 2 bytes. This should fail due to the 8-byte boundary
    // rule in smalloc:
    void *full_attempt = smalloc(ALLOC);
    printf("Attempt to allocate %d btyes of memory, result = %d (0 if prevented; 1 if allocated) \n", ALLOC,
           full_attempt != NULL);

    // Free blocks with middle address first, then free smallest address, and lastly free block with largest address:
    printf("(1): freeing middle block in allocated_list %p - result = %d\n", ptrs[1], sfree(ptrs[1]));
    printf("(2): freeing top block in allocated list    %p - result = %d\n", ptrs[0], sfree(ptrs[0]));
    // Free block with the largest address to the very end of freelist (this is a special case that requires additional
    // logic in smalloc)
    printf("(3): freeing bottom block in allocated list %p - result = %d\n", ptrs[2], sfree(ptrs[2]));

    printf("\nList of allocated blocks:\n");
    print_allocated();
    printf("List of free blocks:\n");
    print_free();
    printf("Contents of allocated memory:\n");
    print_mem();

    // Traversing over freelist to see blocks are by increasing address order:
    printf("\nChecking freelist increasing address order after using sfree...\n");
    print_order_result(); // this needs a helper func in testhelpers

    // Now that all blocks are freed, try to allocate again - should word:
    void *free_attempt = smalloc(ALLOC);
    printf("\nAttempt to allocate %d btyes of memory, result = %d (0 if prevented; 1 if allocated) \n", ALLOC,
           free_attempt != NULL);

    printf("\nList of allocated blocks:\n");
    print_allocated();
    printf("List of free blocks:\n");
    print_free();
    printf("Contents of allocated memory:\n");
    print_mem();

    mem_clean();
    return 0;
}