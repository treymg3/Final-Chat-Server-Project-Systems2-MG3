#include "worker.h"                    /* Include the worker thread declarations. */
#include "message.h"                   /* Include chat_message_t so popped pointers can be cast correctly. */
#include <stdlib.h>                    /* Include free so the worker can release message memory. */

void *broadcaster_thread(void *arg) {                           /* Start the thread function for broadcaster workers. */
    worker_arg_t *worker_arg = (worker_arg_t *)arg;             /* Cast the generic thread argument to worker_arg_t. */
    client_list_t *clients = worker_arg->clients;               /* Copy the shared client list pointer into a local variable. */
    mpmc_queue_t *queue = worker_arg->queue;                    /* Copy the shared queue pointer into a local variable. */

    while (1) {                                                  /* Keep the worker running for the life of the server. */
        chat_message_t *msg = (chat_message_t *)mpmc_queue_pop(queue); /* Consumer step: pop one message pointer from the queue. */

        if (msg == NULL) {                                      /* Check whether the popped message pointer is invalid. */
            continue;                                           /* Skip this loop iteration if there is no valid message. */
        }                                                       /* End NULL message check. */

        broadcast_message(clients, msg);                        /* Send the message to all connected clients except the sender. */
        free(msg);                                              /* Free the heap message after it has been broadcast. */
    }                                                           /* End worker loop. */

    return NULL;                                                /* Return NULL even though this infinite loop normally never exits. */
}                                                               /* End broadcaster_thread. */
