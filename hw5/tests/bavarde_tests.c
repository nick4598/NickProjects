/*#include "thread_counter.h"
#include "csapp.h"
THREAD_COUNTER *tc;
void* incrementFunction();
void* decrementFunction();
void* waitForZero();

int main(int argc, char* argv[]) {
    pthread_t thread1,thread2, thread3;
    tc = tcnt_init();
    pthread_create(&thread1, NULL, incrementFunction, NULL);
    pthread_create(&thread2, NULL, decrementFunction, NULL);
    pthread_create(&thread3, NULL, waitForZero, NULL);
    pthread_join(thread1, NULL);
    pthread_join(thread2, NULL);
    pthread_join(thread3, NULL);
    printf("Done\n");
}

void* incrementFunction()
{
    for (int i = 0; i < 1000; i++)
    {
        tcnt_incr(tc);
    }
    return NULL;
}
void* decrementFunction()
{
    for (int i = 0; i < 1000; i++)
    {
        tcnt_decr(tc);
    }
    return NULL;
}
void* waitForZero()
{
    tcnt_wait_for_zero(tc);
    return NULL;
}
*/