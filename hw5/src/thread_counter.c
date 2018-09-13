/*
 * A thread counter keeps a count of the number of threads that are
 * currently running.  Each time a thread starts, it increments the
 * thread count.  Each time a thread is about to exit, it decrements
 * the thread count.  A thread counter also provides a function that
 * can be called by a thread that wishes to wait for the thread count
 * to drop to zero.  Such a function is useful, for example, in order
 * to achieve clean termination of a multi-threaded application:
 * when termination is desired, the "main" thread takes some action
 * to cause the other threads to terminate and then it waits for the
 * thread count to drop to zero before exiting the program.
 */
#include <semaphore.h>
#include "thread_counter.h"
#include "debug.h"
#include "csapp.h"
struct thread_counter
{
    int counter;
    sem_t semaphore;
    pthread_mutex_t mutex;
};



/*
 * Initialize a new thread counter.
 */
THREAD_COUNTER *tcnt_init()
{
    THREAD_COUNTER* tc = malloc(sizeof(THREAD_COUNTER));
    sem_init(&(tc->semaphore), 0, 0);
    tc->counter = 0;
    pthread_mutex_init(&(tc->mutex), NULL);
    debug("Initialized thread");
    return tc;
}
/*
 * Finalize a thread counter.
 */
void tcnt_fini(THREAD_COUNTER *tc)
{
    sem_destroy(&(tc->semaphore));
    pthread_mutex_destroy(&(tc->mutex));
    free(tc);
}

/*
 * Increment a thread counter.
 */
void tcnt_incr(THREAD_COUNTER *tc)
{
    pthread_mutex_lock(&(tc->mutex));
    debug("tc before increment value now %d", tc->counter);
    tc->counter++;
    debug("tc after increment value now %d", tc->counter);
    pthread_mutex_unlock(&(tc->mutex));
}

/*
 * Decrement a thread counter, alerting anybody waiting
 * if the thread count has dropped to zero.
 */
void tcnt_decr(THREAD_COUNTER *tc)
{
    pthread_mutex_lock(&(tc->mutex));
    debug("tc before decrement called value now %d", tc->counter);
    tc->counter--;
    debug("tc after decrement called value now %d", tc->counter);
    if (tc->counter == 0)
    {
        V(&(tc->semaphore));
    }
    pthread_mutex_unlock(&(tc->mutex));
}

/*
 * A thread calling this function will block in the call until
 * the thread count has reached zero, at which point the
 * function will return.
 */
void tcnt_wait_for_zero(THREAD_COUNTER *tc)
{

    if (tc->counter == 0)
    {
        return;
    }
    debug("Waiting for zero");

    P(&(tc->semaphore));

    //V(tc->(&semaphore));
}