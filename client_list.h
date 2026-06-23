#ifndef CLIENT_LIST_H
#define CLIENT_LIST_H

#include <pthread.h>
#include "message.h"

#define MAX_CLIENTS 128

typedef struct {
    int client_fds[MAX_CLIENTS];
    int count;
    pthread_mutex_t mutex;
} client_list_t;

void client_list_init(client_list_t *list);
void client_list_destroy(client_list_t *list);

void add_client(client_list_t *list, int client_fd);
void remove_client(client_list_t *list, int client_fd);
void broadcast_message(client_list_t *list, chat_message_t *msg);

#endif
