#ifndef WORKER_H
#define WORKER_H

#include "client_list.h"
#include "mpmc_queue.h"

typedef struct {
    client_list_t *clients;
    mpmc_queue_t *queue;
} worker_arg_t;

void *broadcaster_thread(void *arg);

#endif
