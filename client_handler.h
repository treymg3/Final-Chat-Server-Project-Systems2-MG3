#ifndef CLIENT_HANDLER_H
#define CLIENT_HANDLER_H

#include "client_list.h"
#include "mpmc_queue.h"

typedef struct {
    int client_fd;
    client_list_t *clients;
    mpmc_queue_t *queue;
} client_handler_arg_t;

void *client_handler_thread(void *arg);

#endif
