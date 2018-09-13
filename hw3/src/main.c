#include <stdio.h>
#include <errno.h>
#include <string.h>
#include "budmm.h"
#include "budprint.h"
//void* testPointer;
int main(int argc, char const *argv[]) {
    int i;

    bud_mem_init();
    //bud_listprint(3);
    char* ptr1 = (char*)bud_malloc(10); // 64
   // bud_listprint(3);
    //bud_listprint(0);
    //testPointer = ptr1;
    ptr1[0] = 'a';
    ptr1[1] = 'b';
    ptr1[2] = 'c';
    ptr1[3] = 'd';
    ptr1[4] = 'e';
    ptr1[5] = 'f';
    ptr1[6] = '\0';
    printf("ptr1 string: %s\n", ptr1);
    //bud_listprint(3);
    int* ptr2 = bud_malloc(sizeof(int) * 100); // 512
    for(i = 0; i < 100; i++)
        ptr2[i] = i;

    void* ptr3 = bud_malloc(3000); // 4192
    printf("ptr3: %p\n", ptr3);
    //bud_listprint(3);
    //bud_listprint(0);
    //printf("ptr1: %p\n", ptr1);
    bud_free(ptr1);
    //bud_listprint(3);
    //bud_listprint(3);
    //printf("Made it here\n");
    //bud_listprint(3);
    ptr2 = bud_realloc(ptr2, 124); // 128
    //printf("Made it here\n");
    //bud_listprint(3);
    ptr1 = bud_malloc(200); // 256 this line fails
    //2^5 = 32, 2^6 = 64, 2^7 = 128, 2^8 = 256
    //printf("Made it here\n");
    ptr1 = bud_realloc(ptr1, 100); // 128

    // intentional error (errno = EINVAL)
    //printf("Made it here\n");
    ptr3 = bud_malloc(20000);
    printf("errno: %d (%s)\n", errno, strerror(errno));

    bud_mem_fini();

    return EXIT_SUCCESS;
}
