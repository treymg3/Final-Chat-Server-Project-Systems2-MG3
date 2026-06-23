#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <pthread.h>
#include <arpa/inet.h>
#include <errno.h>
#include <signal.h>

#include "client_handler.h"
#include "client_list.h"
#include "mpmc_queue.h"
#include "worker.h"

#define PORT 8082
#define BACKLOG 16
#define NUM_WORKERS 4

static int create_server_socket(int port)
{
    int server_fd;
    struct sockaddr_in address;
    int opt = 1;

    server_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (server_fd < 0) {
        perror("socket failed");
        exit(EXIT_FAILURE);
    }

    if (setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt)) < 0) {
        perror("setsockopt failed");
        close(server_fd);
        exit(EXIT_FAILURE);
    }

    memset(&address, 0, sizeof(address));
    address.sin_family = AF_INET;
    address.sin_addr.s_addr = INADDR_ANY;
    address.sin_port = htons(port);

    if (bind(server_fd, (struct sockaddr *)&address, sizeof(address)) < 0) {
        perror("bind failed");
        close(server_fd);
        exit(EXIT_FAILURE);
    }

    if (listen(server_fd, BACKLOG) < 0) {
        perror("listen failed");
        close(server_fd);
        exit(EXIT_FAILURE);
    }

    return server_fd;
}

int main(void)
{
    signal(SIGPIPE, SIG_IGN);
    int server_fd;
    client_list_t clients;
    mpmc_queue_t queue;
    pthread_t workers[NUM_WORKERS];
    worker_arg_t worker_arg;

    client_list_init(&clients);
    mpmc_queue_init(&queue);

    worker_arg.clients = &clients;
    worker_arg.queue = &queue;

    for (int i = 0; i < NUM_WORKERS; i++) {
        if (pthread_create(&workers[i], NULL, broadcaster_thread, &worker_arg) != 0) {
            perror("pthread_create worker failed");
            exit(EXIT_FAILURE);
        }
    }

    server_fd = create_server_socket(PORT);
    printf("Chat server listening on port %d...\n", PORT);

    while (1) {
        struct sockaddr_in client_addr;
        socklen_t client_len = sizeof(client_addr);

        int client_fd = accept(server_fd, (struct sockaddr *)&client_addr, &client_len);
        if (client_fd < 0) {
            if (errno == EINTR) {
                continue;
            }
            perror("accept failed");
            continue;
        }

        printf("New client connected: fd=%d\n", client_fd);

        client_handler_arg_t *handler_arg = malloc(sizeof(client_handler_arg_t));
        if (handler_arg == NULL) {
            perror("malloc failed");
            close(client_fd);
            continue;
        }

        handler_arg->client_fd = client_fd;
        handler_arg->clients = &clients;
        handler_arg->queue = &queue;

        pthread_t tid;
        if (pthread_create(&tid, NULL, client_handler_thread, handler_arg) != 0) {
            perror("pthread_create client handler failed");
            close(client_fd);
            free(handler_arg);
            continue;
        }

        pthread_detach(tid);
    }

    close(server_fd);
    mpmc_queue_destroy(&queue);
    client_list_destroy(&clients);

    return 0;
}
