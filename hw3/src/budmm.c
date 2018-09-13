/*
 * All functions you make for the assignment must be implemented in this file.
 * Do not submit your assignment with a main function in this file.
 * If you submit with a main function in this file, you will get a zero.
 */
#include <stdio.h>
#include <string.h>
#include <errno.h>
#include "debug.h"
#include "budmm.h"
#include "budprint.h"

/*
 * You should store the heads of your free lists in these variables.
 * Doing so will make it accessible via the extern statement in budmm.h
 * which will allow you to pass the address to sf_snapshot in a different file.
 */
extern bud_free_block free_list_heads[NUM_FREE_LIST];

bud_free_block *findLowestAddress(bud_free_block* head)
{
    bud_free_block * curr = head->next;
    bud_free_block * prev = head->prev;
    if (curr == prev)
    {
        head->next = head;
        head->prev = head;
        return curr;
    }
    bud_free_block * lowest = curr;
    bud_free_block * beforeLowest = curr->prev;
    bud_free_block * afterLowest = curr->next;
    //bud_listprint(3);
    while (curr != head)
    {
        //printf("findLowestAddress\n");
        if (curr < lowest)
        {

            lowest = curr;
            beforeLowest = lowest->prev;
            afterLowest = lowest->next;
        }
        curr = curr->next;
    }
    beforeLowest->next = afterLowest;
    afterLowest->prev = beforeLowest;

    return lowest;
}
void removeFromList(bud_free_block* bud)
{
    int order = bud->header.order;
    //printf("order = %d\n", order);
    bud_free_block sentinel = free_list_heads[order - ORDER_MIN];
    bud_free_block *curr = sentinel.next;
    bud_free_block *prev = sentinel.prev;
    if (curr == bud)
    {
        sentinel.next = &sentinel;
        sentinel.prev = &sentinel;
        curr->next = NULL;
        curr->prev = NULL;
    }
    while (curr != bud && curr != &sentinel);
    {
        curr = curr->next;
        prev = curr;
    }
    if (curr != &sentinel)
    {
        prev->next = curr->next;
    curr->next = NULL;
    curr->prev = NULL;
    }


}

void *bud_malloc(uint32_t rsize) {
    //inserting into a list would set the next field of the sentinel to be the inserted
    //node, and removing from a list would likewise modify the next field of the
    //sentinel
    //printf("\nmade it here\n");
    //printf("rsize = %d\n", rsize);
    if (rsize == 0)
    {
        errno = EINVAL;
        return NULL;
    }
    if (rsize > (MAX_BLOCK_SIZE - sizeof(bud_header)))
    {
        errno = EINVAL;
        return NULL;
    }
    int blockSize;
    for (int i = 0; i < NUM_FREE_LIST; i++)
    {
        blockSize = ORDER_MIN + i;
        blockSize = ORDER_TO_BLOCK_SIZE(blockSize);
        if (rsize == 32)
        {
            //printf("blockSize + sizeof(bud_header) = %lu\n", blockSize + sizeof(bud_header));
        }
        if (rsize <= blockSize - sizeof(bud_header))
            break;
    }

    if (rsize == 32)
    {
        //printf("blocksize = %d\n", blockSize);
    }

    //printf("blocksize = %d\n", blockSize);
    //printf("\nmade it here\n");
    int foundBlock = 0;
    int currentSize;
    //int order;
    bud_free_block *block = NULL;
    while (foundBlock == 0)
    {
       //tf("\nmade it here\n");
        for (int i = 0; i < NUM_FREE_LIST; i++)
        {
            //bud_free_block b = free_list_heads[i];
            currentSize = ORDER_TO_BLOCK_SIZE(ORDER_MIN + i);
            //printf("\ncurrentSize = %d\n", currentSize);
            if (free_list_heads[i].next == &free_list_heads[i])
            {
                //printf("\nmade it here\n");
                if (currentSize == MAX_BLOCK_SIZE)
                {
                    bud_free_block *bigBoyBlock = bud_sbrk();
                    if (bigBoyBlock == (void*)-1)
                    {
                        errno = ENOMEM;
                        return NULL;
                    }
                    bigBoyBlock->header.order = ORDER_MAX - 1;
                    bigBoyBlock->header.allocated = 0;
                    free_list_heads[i].next = bigBoyBlock;
                    bigBoyBlock->next = &free_list_heads[i];
                    free_list_heads[i].prev = bigBoyBlock;
                    bigBoyBlock->prev = &free_list_heads[i];
                    //printf("\nmade it here\n");

                }
            }
            else
            {
                //printf("\nmade it here\n");

                if (blockSize == currentSize)
                {

                    //printf("\nmade it here\n");
                    //printf("blocksize = %d\n", blockSize);

                    //block = free_list_heads[i].next;
                    foundBlock = 1;
                    block = findLowestAddress(&free_list_heads[i]);
                      // order = ORDER_MIN + i;
                    break;
                }
                else if (blockSize < currentSize)
                {
                    bud_free_block *b1 = findLowestAddress(&free_list_heads[i]);

                    //bud_free_block *b1 = free_list_heads[i].next;
                    //char *b3 = ((char*)free_list_heads[i].next + currentSize / 2);
                    char *b3 = ((char*)b1 + currentSize / 2);
                    bud_free_block *b2 = (bud_free_block*)b3;
                    b1->header.order = ORDER_MIN + i - 1;
                    b1->header.allocated = 0;
                    b2->header.order = ORDER_MIN + i - 1;

                    b2->header.allocated = 0;
                    //free_list_heads[i].next = b1->next;

                    if (free_list_heads[i-1].next == &free_list_heads[i-1])//need to set prev here
                    {
                        free_list_heads[i-1].prev = b1;
                        free_list_heads[i-1].next = b1;
                        free_list_heads[i-1].next = b2;
                        b2->next = b1;
                        b1->next = &free_list_heads[i-1];
                        b2->prev = &free_list_heads[i-1];
                        b1->prev = b2;

                    }
                    else
                    {
                        b1->next = free_list_heads[i-1].next;
                        free_list_heads[i-1].next = b1;
                        free_list_heads[i-1].next = b2;
                        b2->next = b1;
                        b1->prev = b2;
                        b2->prev = &free_list_heads[i-1];

                    }
                    break;


                }
            }
        }
    }
    block->header.rsize = rsize;
    block->header.allocated = 1;
    //printf("allocated bit = %u\n", block->header.allocated);
   // printf("rsize = %d\n", rsize);
   // printf("currentsize  = %d\n", currentSize);
    //printf("blocksize = %d\n", blockSize);
    //printf("rsize + header size = %lu\n", rsize + sizeof(bud_header));
    //printf("block header order = %d\n", block->header.order);
   // printf("size of void* = %lu\n", sizeof(void*));
    if (rsize + sizeof(bud_header) < currentSize)
    {
        block->header.padded = 1;
        //printf("block padded bit %d\n", block->header.padded);
    }
    else
    {
        block->header.padded = 0;
        //printf("block padded bit %d\n", block->header.padded);
    }


    return (((void*)block) + sizeof(bud_header));
}

void *bud_realloc(void *ptr, uint32_t rsize) {
    if (ptr == NULL && rsize == 0)
    {
        errno = EINVAL;
        return NULL;
    }
    if (rsize == 0)
    {
        bud_free(ptr);
        return NULL;
    }
    if (rsize > (MAX_BLOCK_SIZE - sizeof(bud_header)))
    {
        errno = EINVAL;
        return NULL;
    }
    if (ptr == NULL)
    {
        return bud_malloc(rsize);
    }
    if (ptr < bud_heap_start() || ptr > bud_heap_end())
    {
        abort();
    }

    bud_free_block *address = ptr - sizeof(bud_header);
    long unsigned answer = (uintptr_t)ptr & 0b111;
    if (answer != 0)
    {
        //printf("\nmade ith ere\n");
        abort();
        //printf("\nmade ith ere\n");
    }
    //printf("\nmade ith ere\n");
    bud_free_block *b = (bud_free_block*) address;
     //printf("\nfail here\n");
    uint64_t order = b->header.order;
    //printf("\nmade ith ere\n");
    //printf("order = %lu\n", order);
    if (order < ORDER_MIN || order > (ORDER_MAX - 1))
    {
         //printf("\nfail here\n");
        abort();
    }
    uint64_t allocated = b->header.allocated;
     //printf("\nfail here\n");
    if (allocated == 0)
    {
        abort();
    }
     //printf("\nfail here\n");
     //printf("\nfail here\n");
    uint64_t padded = b->header.padded;
    if (padded == 0)
    {
        if (b->header.rsize + sizeof(bud_header) != ORDER_TO_BLOCK_SIZE(order))
        {
            //printf("b header rsize = %d\n", b->header.rsize);
            //printf("orer = %lu\n", order);
            // printf("\nfail here\n");
            abort();
        }
    }
    else if (padded == 1)
    {
        if (b->header.rsize + sizeof(bud_header) == ORDER_TO_BLOCK_SIZE(order))
        {
            abort();
        }
    }
     //printf("\nfail here\n");
    uint64_t size = b->header.rsize;
    //printf("size = %lu\n", size);
    //printf("order = %d\n", b->header.order);
     //printf("\nfail here\n");
    if (size > ORDER_TO_BLOCK_SIZE(order))
    {
        abort();
    }
     //printf("\nfail here\n");
    int appropriateBlockSize;
    for (int i = 0; i < NUM_FREE_LIST; i++)
    {
        appropriateBlockSize = ORDER_MIN + i;
        appropriateBlockSize = ORDER_TO_BLOCK_SIZE(appropriateBlockSize);
        if (size <= appropriateBlockSize - sizeof(bud_header))
        {
            break;
        }
    }
     //printf("\nfail here\n");
    if (appropriateBlockSize != ORDER_TO_BLOCK_SIZE(order))
        abort();

    //printf("address of header = %p\n", address);
     //printf("\nfail here\n");
    int blockSize;
    for (int i = 0; i < NUM_FREE_LIST; i++)
    {
        blockSize = ORDER_MIN + i;
        blockSize = ORDER_TO_BLOCK_SIZE(blockSize);
        if (rsize <= blockSize - sizeof(bud_header))
            break;
    }
    //printf("address of header = %p\n", address);
     //printf("\nfail here\n");
    if (ORDER_TO_BLOCK_SIZE(order) == blockSize)
    {
        //printf("address of header = %p\n", address);
        return ptr;
    }
    else if (blockSize > ORDER_TO_BLOCK_SIZE(order))
    {
        //printf("address of header = %p\n", address);
        void* newBlock = bud_malloc(rsize);
        //printf("address of header = %p\n", address);
        memcpy(newBlock, address, ORDER_TO_BLOCK_SIZE(order));
       // printf("address of header = %p\n", address);

        bud_free(ptr);
        //printf("address of header = %p\n", address);
        return (void*)newBlock;

    }
    else if (blockSize < ORDER_TO_BLOCK_SIZE(order))
    {
         //printf("\nfail here\n");
        //printf("address of header = %p\n", address);
        int properBlockSize = 0;
        while (properBlockSize == 0)
        {
             //printf("\nfail here\n");
            //printf("address of header = %p\n", address);
            int currentSize = ORDER_TO_BLOCK_SIZE(order);
            //bud_free_block *b1 = findLowestAddress(&free_list_heads[order - ORDER_MIN]);

                    // printf("\nfail here\n");
                    //bud_free_block *b1 = free_list_heads[i].next;
                    //char *b3 = ((char*)free_list_heads[i].next + currentSize / 2);
                    //printf("address of header = %p\n", address);
                    char *b3 = ((char*)address + currentSize  / 2);
                    //printf("address of headerrr = %p\n", address);
                    bud_free_block *b2 = (bud_free_block*)b3;
                    order = order - 1;
                    //printf("address of header = %p\n", address);
                    address->header.order = order;
                    //b1->header.allocated = 0;
                    b2->header.order = order;

                    b2->header.allocated = 0;
                    //free_list_heads[i].next = b1->next;
                    //printf("address of header = %p\n", address);

                    if (free_list_heads[order - ORDER_MIN].next == &free_list_heads[order - ORDER_MIN])//need to set prev here
                    {
                        free_list_heads[order - ORDER_MIN].prev = b2;
                        //free_list_heads[i-1].next = b1;
                        free_list_heads[order - ORDER_MIN].next = b2;
                        b2->next = &free_list_heads[order - ORDER_MIN];
                        //b1->next = &free_list_heads[order - ORDER_MIN];
                        b2->prev = &free_list_heads[order - ORDER_MIN];
                        //b1->prev = b2;

                    }
                    else
                    {
                        b2->next = free_list_heads[order - ORDER_MIN].next;
                        //free_list_heads[i-1].next = b1;
                        free_list_heads[order - ORDER_MIN].next->prev = b2;
                        free_list_heads[order - ORDER_MIN].next = b2;
                        //b2->next = b1;
                        //b1->prev = b2;
                        b2->prev = &free_list_heads[order - ORDER_MIN];

                    }

                    if (ORDER_TO_BLOCK_SIZE(order) == blockSize)
                    {
                        properBlockSize = 1;
                        if (rsize + sizeof(bud_header) < ORDER_TO_BLOCK_SIZE(order))
                        {
                             address->header.padded = 1;
                             //printf("block padded bit %d\n", block->header.padded);
                        }
                        else
                        {
                             address->header.padded = 0;
                                //printf("block padded bit %d\n", block->header.padded);
                        }
                        address->header.rsize = rsize;
                        return (((void*)address) + sizeof(bud_header));
                    }
        }

    }
    return NULL;

}

void bud_free(void *ptr) {

    //printf("\nmade ith ere\n");
    if (ptr < bud_heap_start() || ptr > bud_heap_end())
    {
        abort();
    }
   // printf("\nmade ith ere\n");
    bud_free_block *address = ptr - sizeof(bud_header);
    long unsigned answer = (uintptr_t)ptr & 0b111;
    //printf("answer = %lu\n", answer);
    //printf("\nmade ith ere\n");
    //testPointer
    if (answer != 0)
    {
        //printf("\nmade ith ere\n");
        abort();
        //printf("\nmade ith ere\n");
    }
    //printf("\nmade ith ere\n");

    bud_free_block *b = (bud_free_block*) address;

    uint64_t order = b->header.order;
    //printf("\nmade ith ere\n");
    //printf("order = %lu\n", order);
    if (order < ORDER_MIN || order > (ORDER_MAX - 1))
    {
        abort();
    }
    uint64_t allocated = b->header.allocated;
    if (allocated == 0)
    {
        abort();
    }
    uint64_t padded = b->header.padded;
    if (padded == 0)
    {
        if (b->header.rsize + sizeof(bud_header) != ORDER_TO_BLOCK_SIZE(order))
        {
            abort();
        }
    }
    else if (padded == 1)
    {
        if (b->header.rsize + sizeof(bud_header) == ORDER_TO_BLOCK_SIZE(order))
        {
            abort();
        }
    }
    uint64_t rsize = b->header.rsize;
    if (rsize > ORDER_TO_BLOCK_SIZE(order))
    {
        abort();
    }
    //printf("\nmade ith ere\n");
    b->header.allocated = 0;
    //bud_free_block *bbefore = (bud_free_block*) address - ORDER_TO_BLOCK_SIZE(order) - sizeof(bud_header);
    //bud_free_block *bafter = (bud_free_block*) address + ORDER_TO_BLOCK_SIZE(order);
    int cantCoalesce = 0;
    int blockAlreadyAdded = 0;
    //int firstTime = 1;
    bud_free_block *leftMostAddress;
    //bud_listprint(3);

    while (cantCoalesce == 0)
    {

        void * addressOfBuddy = (void*)((uintptr_t)address ^ (ORDER_TO_BLOCK_SIZE(order)));
        addressOfBuddy = (void*)addressOfBuddy;
       // printf("addressOfBuddy = %p\n", addressOfBuddy);
        bud_free_block *buddy = (bud_free_block*) addressOfBuddy;
      //  address = buddy;

        if (buddy->next == NULL && buddy->prev == NULL)
        {
            if (free_list_heads[order - ORDER_MIN].next == &free_list_heads[order - ORDER_MIN])
            {
                    //printf("order = %lu\n", order);
                    address->header.order = order;
                    address->header.allocated = 0;
                    free_list_heads[order - ORDER_MIN].next = address;
                    address->next = &free_list_heads[order - ORDER_MIN];
                    free_list_heads[order - ORDER_MIN].prev = address;
                    address->prev = &free_list_heads[order - ORDER_MIN];
            }
            else
            {
                    address->next = free_list_heads[order - ORDER_MIN].next;
                        free_list_heads[order - ORDER_MIN].next = address;
                        address->prev = &free_list_heads[order-ORDER_MIN];
                        //free_list_heads[i-1].next = b2;
                        //b2->next = b1;
            }
            cantCoalesce = 1;
            continue;
        }

        //printf("buddy header allocated = %d\n", buddy->header.allocated);
        //printf("buddy header order = %d\n", buddy->header.order);
        //printf("\nmade it here\n");
        if (buddy->header.allocated == 0 && buddy->header.order == order)
        {
            if (order == ORDER_MAX - 1)
            {
                //printf("order = %lu\n", order);
                address->header.order = order;
                address->header.allocated = 0;
               address->next = free_list_heads[order - ORDER_MIN].next;
               free_list_heads[order - ORDER_MIN].next->prev = address;
                free_list_heads[order - ORDER_MIN].next = address;
                address->prev = &free_list_heads[order-ORDER_MIN];
                cantCoalesce = 1;
                break;
            }


           // printf("order = %lu\n", order);
            if (free_list_heads[order - ORDER_MIN].next != &free_list_heads[order - ORDER_MIN])
            {
                    //printf("\nmade it here\n");
                //printf("\nmade it here\n");
                bud_free_block *curr = free_list_heads[order - ORDER_MIN].next;
                bud_free_block *prev = free_list_heads[order - ORDER_MIN].prev;
                if (curr == prev)
                {
                    free_list_heads[order-ORDER_MIN].next = &free_list_heads[order-ORDER_MIN];
                    free_list_heads[order-ORDER_MIN].prev = &free_list_heads[order-ORDER_MIN];
                }
                else
                {
                    while (buddy != curr)
                    {
                        //if (curr == &free_list_heads[order - ORDER_MIN])
                        //{

                          //  couldntFind = 1;
                           // break;
                       // }
                        prev = curr;
                        curr = curr->next;
                    }

                        prev->next = curr->next;

                         curr->next->prev = prev;
                         buddy->next = NULL;
                         buddy->prev = NULL;

            //printf("\nmade it here\n");

                }
            }


             if (address < buddy)
             {
                //printf("address = %p\n", address);
                leftMostAddress = address;
                //printf("made it here\n");
                //removeFromList(buddy);

             }
             else
             {
                leftMostAddress = buddy;
             }
             order = order + 1;
             leftMostAddress->header.order = order;
             leftMostAddress->header.allocated = 0;
         //now i have to add this bigger block to free_lsit_heads;
            if (free_list_heads[order - ORDER_MIN].next == &free_list_heads[order - ORDER_MIN])
            {


                    //leftMostAddress->header.order = order;
                    //leftMostAddress->header.allocated = 0;
                if (leftMostAddress == buddy)
                {
                    free_list_heads[order - ORDER_MIN].next = leftMostAddress;
                    leftMostAddress->next = &free_list_heads[order - ORDER_MIN];
                    free_list_heads[order - ORDER_MIN].prev = leftMostAddress;
                    leftMostAddress->prev = &free_list_heads[order - ORDER_MIN];
                }


            }
            else
            {
                //free_list_heads[order-ORDER_MIN].next->prev = leftMostAddress;
                if (leftMostAddress == buddy)
                {
                    leftMostAddress->next = free_list_heads[order - ORDER_MIN].next;
                leftMostAddress->prev = &free_list_heads[order-ORDER_MIN];
                        free_list_heads[order - ORDER_MIN].next = leftMostAddress;
                }

                        //free_list_heads[i-1].next = b2;
                        //b2->next = b1;
            }
            //blockAlreadyAdded = blockAlreadyAdded ^ 1;

        //buddy->next = free_list_heads[order - ORDER_MIN].next
            //if we add the block and go back through and find its buddy cant be added then we
            //will probably readd the block SHIT
            address = leftMostAddress;
        }
        else //else buddy is allocated or not same order so we just add the buddyless block to the list
        {



            //printf("cantCoalesce\n");
            if (free_list_heads[order - ORDER_MIN].next == &free_list_heads[order - ORDER_MIN])
            {
                    //printf("order = %lu\n", order);
                    address->header.order = order;
                    address->header.allocated = 0;
                    free_list_heads[order - ORDER_MIN].next = address;
                    address->next = &free_list_heads[order - ORDER_MIN];
                    free_list_heads[order - ORDER_MIN].prev = address;
                    address->prev = &free_list_heads[order - ORDER_MIN];
            }
            else
            {
                blockAlreadyAdded = 0;
                bud_free_block *start = free_list_heads[order-ORDER_MIN].next;
                while (start != &free_list_heads[order-ORDER_MIN])
                {
                    if (start == address)
                    {
                        blockAlreadyAdded = 1;
                        break;
                    }
                    start = start->next;
                }
                if (blockAlreadyAdded == 0)
                {

                    address->next = free_list_heads[order - ORDER_MIN].next;
                    free_list_heads[order-ORDER_MIN].next->prev = address;
                        free_list_heads[order - ORDER_MIN].next = address;
                        //free_list_heads[i-1].next = b2;
                        //b2->next = b1;
                }
            }

            cantCoalesce = 1;

        }

    }




    return;
}
