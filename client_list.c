#include "client_list.h"               /* Include the client list declarations. */
#include <errno.h>                     /* Include errno so failed send errors can be explained. */
#include <stdio.h>                     /* Include standard I/O for printf and fprintf. */
#include <string.h>                    /* Include string helpers for strlen and strerror. */
#include <unistd.h>                    /* Include close so disconnected sockets can be closed. */
#include <sys/socket.h>                /* Include socket functions such as send. */

void client_list_init(client_list_t *list) {             /* Start the function that initializes the client list. */
    list->count = 0;                                     /* Start with zero connected clients. */
    pthread_mutex_init(&list->mutex, NULL);              /* Initialize the mutex that protects the shared list. */
}                                                        /* End client_list_init. */

void client_list_destroy(client_list_t *list) {          /* Start the function that destroys the client list. */
    pthread_mutex_destroy(&list->mutex);                 /* Destroy the mutex when the server is finished. */
}                                                        /* End client_list_destroy. */

void add_client(client_list_t *list, int client_fd) {    /* Start the function that adds one connected client. */
    pthread_mutex_lock(&list->mutex);                    /* Lock the list before reading or changing shared data. */

    if (list->count >= MAX_CLIENTS) {                    /* Check whether the client array is already full. */
        pthread_mutex_unlock(&list->mutex);              /* Unlock before leaving because we are done with the list. */
        fprintf(stderr, "Client list full. Closing fd=%d\n", client_fd); /* Print why this client is being closed. */
        close(client_fd);                                /* Close the rejected client socket. */
        return;                                          /* Leave because the client could not be added. */
    }                                                    /* End full-list check. */

    list->client_fds[list->count] = client_fd;           /* Store the new client fd in the next open array slot. */
    list->count++;                                       /* Increase the number of connected clients. */

    printf("Client added: fd=%d, total clients=%d\n", client_fd, list->count); /* Print a helpful server message. */

    pthread_mutex_unlock(&list->mutex);                  /* Unlock so other threads can use the client list. */
}                                                        /* End add_client. */

void remove_client(client_list_t *list, int client_fd) { /* Start the function that removes one disconnected client. */
    pthread_mutex_lock(&list->mutex);                    /* Lock the list before searching or changing it. */

    for (int i = 0; i < list->count; i++) {              /* Loop through the currently connected clients. */
        if (list->client_fds[i] == client_fd) {          /* Check whether this slot holds the fd we need to remove. */
            list->client_fds[i] = list->client_fds[list->count - 1]; /* Replace removed fd with the last fd in the list. */
            list->count--;                               /* Decrease the connected client count. */
            printf("Client removed: fd=%d, total clients=%d\n", client_fd, list->count); /* Print a helpful server message. */
            break;                                       /* Stop searching because the client was removed. */
        }                                                /* End matching-client check. */
    }                                                    /* End search loop. */

    pthread_mutex_unlock(&list->mutex);                  /* Unlock after the shared list update is complete. */
}                                                        /* End remove_client. */

void broadcast_message(client_list_t *list, chat_message_t *msg) { /* Start the function that sends a message to clients. */
    pthread_mutex_lock(&list->mutex);                    /* Lock the list so it cannot change during broadcasting. */

    for (int i = 0; i < list->count; i++) {              /* Loop through each connected client fd. */
        int target_fd = list->client_fds[i];             /* Copy the current target socket fd into a local variable. */

        if (target_fd == msg->sender_fd) {               /* Check whether this target is the original sender. */
            continue;                                    /* Skip the sender so clients do not receive their own message. */
        }                                                /* End sender-skip check. */

        ssize_t sent = send(target_fd, msg->text, strlen(msg->text), MSG_NOSIGNAL); /* Send the message text to this client. */

        if (sent < 0) {                                  /* Check whether send failed. */
            fprintf(stderr, "send failed to fd=%d: %s\n", target_fd, strerror(errno)); /* Print which client failed and why. */
            close(target_fd);                            /* Close the socket because the client is probably disconnected. */
            list->client_fds[i] = list->client_fds[list->count - 1]; /* Replace failed fd with the last fd in the list. */
            list->count--;                               /* Decrease the connected client count. */
            i--;                                         /* Re-check this index because a new fd was swapped into it. */
        }                                                /* End send-failure handling. */
    }                                                    /* End broadcast loop. */

    pthread_mutex_unlock(&list->mutex);                  /* Unlock after broadcasting to the current client list. */
}                                                        /* End broadcast_message. */
