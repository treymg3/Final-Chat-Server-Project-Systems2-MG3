#include "mpmc_queue.h"              /* Include the chat server queue declarations. */

#include <stdio.h>                   /* Include standard I/O so perror and fprintf can print errors. */
#include <stdlib.h>                  /* Include standard library so malloc, free, and exit are available. */

/*
 * This queue is adapted from PA4 queue.h and queue.c.
 * PA4 stored int values in int *buffer.
 * This chat server stores message pointers in void **buffer.
 */

void mpmc_queue_init(mpmc_queue_t *q) {                           /* Start the function that initializes the queue. */
    if (q == NULL) {                                              /* Check whether the caller passed a bad queue pointer. */
        fprintf(stderr, "mpmc_queue_init: queue pointer is NULL\n"); /* Print a clear error message for debugging. */
        exit(EXIT_FAILURE);                                       /* Stop the program because the queue is required. */
    }                                                            /* End the NULL pointer check. */

    q->capacity = MPMC_QUEUE_CAPACITY;                            /* Store the fixed queue capacity from the header. */
    q->front = 0;                                                 /* Start popping from index 0. */
    q->rear = 0;                                                  /* Start pushing at index 0. */
    q->count = 0;                                                 /* The queue starts empty, so the count is 0. */

    q->buffer = malloc((size_t)q->capacity * sizeof(void *));     /* Allocate an array that stores generic pointers. */
    if (q->buffer == NULL) {                                      /* Check whether malloc failed. */
        perror("malloc queue buffer");                           /* Print the system error for the allocation failure. */
        exit(EXIT_FAILURE);                                       /* Stop because the queue cannot work without memory. */
    }                                                            /* End malloc failure check. */

    for (int i = 0; i < q->capacity; i++) {                       /* Loop through every queue slot once. */
        q->buffer[i] = NULL;                                      /* Initialize each pointer slot to NULL for safety. */
    }                                                            /* End buffer initialization loop. */

    if (pthread_mutex_init(&q->mutex, NULL) != 0) {               /* Initialize the mutex and check whether it failed. */
        perror("pthread_mutex_init");                            /* Print a system error message for mutex failure. */
        free(q->buffer);                                          /* Free the buffer because setup cannot continue. */
        q->buffer = NULL;                                         /* Avoid leaving a pointer to freed memory. */
        exit(EXIT_FAILURE);                                       /* Stop because the queue cannot be safely shared. */
    }                                                            /* End mutex initialization check. */

    if (sem_init(&q->empty, 0, (unsigned int)q->capacity) != 0) {  /* Initialize empty slots to capacity because queue starts empty. */
        perror("sem_init empty");                                /* Print a system error message for semaphore failure. */
        pthread_mutex_destroy(&q->mutex);                         /* Destroy the mutex because setup failed. */
        free(q->buffer);                                          /* Free the allocated pointer buffer. */
        q->buffer = NULL;                                         /* Avoid leaving a dangling pointer. */
        exit(EXIT_FAILURE);                                       /* Stop because the queue cannot synchronize producers. */
    }                                                            /* End empty semaphore initialization check. */

    if (sem_init(&q->full, 0, 0) != 0) {                          /* Initialize full slots to zero because no items are present yet. */
        perror("sem_init full");                                 /* Print a system error message for semaphore failure. */
        sem_destroy(&q->empty);                                   /* Destroy the empty semaphore that was already initialized. */
        pthread_mutex_destroy(&q->mutex);                         /* Destroy the mutex that was already initialized. */
        free(q->buffer);                                          /* Free the allocated pointer buffer. */
        q->buffer = NULL;                                         /* Avoid leaving a dangling pointer. */
        exit(EXIT_FAILURE);                                       /* Stop because the queue cannot synchronize consumers. */
    }                                                            /* End full semaphore initialization check. */
}                                                                /* End mpmc_queue_init. */

void mpmc_queue_push(mpmc_queue_t *q, void *item) {               /* Start the producer function that inserts one pointer. */
    if (q == NULL || item == NULL) {                              /* Check for invalid queue or invalid item pointer. */
        return;                                                   /* Do nothing because there is nothing safe to push. */
    }                                                            /* End invalid input check. */

    sem_wait(&q->empty);                                          /* Wait until at least one empty slot is available. */
    pthread_mutex_lock(&q->mutex);                                /* Lock the queue before changing shared fields. */

    q->buffer[q->rear] = item;                                    /* Store the new pointer at the rear index. */
    q->rear = (q->rear + 1) % q->capacity;                        /* Move rear forward and wrap to 0 at the end. */
    q->count++;                                                   /* Increase the count because one item was inserted. */

    pthread_mutex_unlock(&q->mutex);                              /* Unlock the queue so another thread can use it. */
    sem_post(&q->full);                                           /* Signal that one more full slot is available to consumers. */
}                                                                /* End mpmc_queue_push. */

void *mpmc_queue_pop(mpmc_queue_t *q) {                           /* Start the consumer function that removes one pointer. */
    void *item;                                                   /* Create a local variable to hold the removed pointer. */

    if (q == NULL) {                                              /* Check for an invalid queue pointer. */
        return NULL;                                              /* Return NULL because there is no queue to pop from. */
    }                                                            /* End invalid queue check. */

    sem_wait(&q->full);                                           /* Wait until at least one full slot is available. */
    pthread_mutex_lock(&q->mutex);                                /* Lock the queue before changing shared fields. */

    item = q->buffer[q->front];                                   /* Read the pointer stored at the front index. */
    q->buffer[q->front] = NULL;                                   /* Clear the slot so it no longer holds an old pointer. */
    q->front = (q->front + 1) % q->capacity;                      /* Move front forward and wrap to 0 at the end. */
    q->count--;                                                   /* Decrease the count because one item was removed. */

    pthread_mutex_unlock(&q->mutex);                              /* Unlock the queue so another thread can use it. */
    sem_post(&q->empty);                                          /* Signal that one more empty slot is available to producers. */

    return item;                                                  /* Return the removed pointer to the worker thread. */
}                                                                /* End mpmc_queue_pop. */

void mpmc_queue_destroy(mpmc_queue_t *q) {                        /* Start the function that cleans up queue resources. */
    if (q == NULL) {                                              /* Check whether the caller passed a NULL queue pointer. */
        return;                                                   /* Do nothing because there is no queue to destroy. */
    }                                                            /* End NULL queue check. */

    sem_destroy(&q->empty);                                       /* Destroy the empty-slot semaphore. */
    sem_destroy(&q->full);                                        /* Destroy the full-slot semaphore. */
    pthread_mutex_destroy(&q->mutex);                             /* Destroy the queue mutex. */
    free(q->buffer);                                              /* Free the dynamically allocated pointer array. */
    q->buffer = NULL;                                             /* Set buffer to NULL so it does not point to freed memory. */
    q->capacity = 0;                                              /* Reset capacity because the queue is no longer active. */
    q->front = 0;                                                 /* Reset front index for safety. */
    q->rear = 0;                                                  /* Reset rear index for safety. */
    q->count = 0;                                                 /* Reset count because the queue is empty/destroyed. */
}                                                                /* End mpmc_queue_destroy. */
