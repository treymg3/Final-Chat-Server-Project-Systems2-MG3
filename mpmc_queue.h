#ifndef MPMC_QUEUE_H                 /* Start the include guard so this header is not included twice. */
#define MPMC_QUEUE_H                 /* Define the include guard name. */

#include <pthread.h>                 /* Include pthreads so the queue can use pthread_mutex_t. */
#include <semaphore.h>               /* Include semaphores so the queue can use sem_t. */

#define MPMC_QUEUE_CAPACITY 1000     /* Use the same default capacity idea from PA4: 1000 slots. */

typedef struct mpmc_queue {          /* Define the MPMC queue struct used by the chat server. */
    void **buffer;                   /* Store generic pointers; each item will be a chat_message_t pointer. */
    int capacity;                    /* Store the maximum number of pointers the queue can hold. */
    int front;                       /* Store the index where the next pop operation removes an item. */
    int rear;                        /* Store the index where the next push operation inserts an item. */
    int count;                       /* Store the current number of items in the queue for bookkeeping. */
    pthread_mutex_t mutex;           /* Protect shared queue fields from race conditions. */
    sem_t empty;                     /* Count empty slots so producers do not push into a full queue. */
    sem_t full;                      /* Count filled slots so consumers do not pop from an empty queue. */
} mpmc_queue_t;                      /* Name the struct type mpmc_queue_t. */

void mpmc_queue_init(mpmc_queue_t *q);        /* Set up the queue before any thread uses it. */
void mpmc_queue_push(mpmc_queue_t *q, void *item); /* Add one pointer item to the queue. */
void *mpmc_queue_pop(mpmc_queue_t *q);        /* Remove and return one pointer item from the queue. */
void mpmc_queue_destroy(mpmc_queue_t *q);     /* Clean up the queue memory and synchronization objects. */

#endif                               /* End the include guard. */
