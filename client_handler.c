#include "client_handler.h"            /* Include the client handler declarations. */
#include "message.h"                   /* Include the chat_message_t struct definition. */
#include <stdio.h>                     /* Include standard I/O for printf and perror. */
#include <stdlib.h>                    /* Include standard library for malloc and free. */
#include <string.h>                    /* Include string helpers for memset, strncmp, and snprintf. */
#include <unistd.h>                    /* Include close for closing client sockets. */
#include <sys/socket.h>                /* Include socket functions such as recv. */

void *client_handler_thread(void *arg) {                         /* Start the thread function for one connected client. */
    client_handler_arg_t *handler_arg = (client_handler_arg_t *)arg; /* Cast the generic thread argument to the correct type. */
    int client_fd = handler_arg->client_fd;                       /* Copy this client's socket fd into a local variable. */
    client_list_t *clients = handler_arg->clients;                /* Copy the shared client list pointer into a local variable. */
    mpmc_queue_t *queue = handler_arg->queue;                     /* Copy the shared queue pointer into a local variable. */
    free(handler_arg);                                            /* Free the argument struct allocated by chat_server.c. */

    add_client(clients, client_fd);                               /* Add this client fd to the shared connected-client list. */

    char buffer[MAX_MSG_LEN];                                     /* Create a stack buffer for text received from this client. */

    while (1) {                                                   /* Keep receiving messages until the client disconnects. */
        memset(buffer, 0, sizeof(buffer));                        /* Clear the buffer before reading a new message. */

        ssize_t bytes_received = recv(client_fd, buffer, sizeof(buffer) - 1, 0); /* Receive bytes from this client's socket. */

        if (bytes_received == 0) {                                /* Check whether the client closed the connection normally. */
            printf("Client fd=%d disconnected.\n", client_fd);    /* Print a server-side disconnect message. */
            break;                                                /* Leave the loop because this client is done. */
        }                                                        /* End normal-disconnect check. */

        if (bytes_received < 0) {                                 /* Check whether recv failed with an error. */
            perror("recv failed");                               /* Print the system error message for recv. */
            break;                                                /* Leave the loop because this client cannot be read safely. */
        }                                                        /* End recv-error check. */

        buffer[bytes_received] = '\0';                            /* Add a null terminator so buffer is a valid C string. */

        if (strncmp(buffer, "/quit", 5) == 0) {                  /* Check whether the client typed the quit command. */
            break;                                                /* Leave the loop to disconnect this client. */
        }                                                        /* End quit-command check. */

        chat_message_t *msg = malloc(sizeof(chat_message_t));     /* Allocate a message object on the heap for the worker thread. */
        if (msg == NULL) {                                        /* Check whether malloc failed. */
            perror("malloc chat_message_t");                     /* Print the system error message for malloc. */
            break;                                                /* Leave because we cannot safely create the message. */
        }                                                        /* End malloc failure check. */

        msg->sender_fd = client_fd;                               /* Store which client sent this message. */
        snprintf(msg->text, sizeof(msg->text), "Client %d: %s", client_fd, buffer); /* Format the text clients will receive. */

        mpmc_queue_push(queue, msg);                              /* Producer step: push the message pointer into the shared queue. */
    }                                                            /* End receive loop. */

    remove_client(clients, client_fd);                            /* Remove this client fd from the shared client list. */
    close(client_fd);                                             /* Close this client's socket to release the OS resource. */

    return NULL;                                                  /* Return NULL because pthread thread functions return void pointers. */
}                                                                /* End client_handler_thread. */
