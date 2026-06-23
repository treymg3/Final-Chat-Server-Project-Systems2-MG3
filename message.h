#ifndef MESSAGE_H
#define MESSAGE_H

#define MAX_MSG_LEN 1024

typedef struct {
    int sender_fd;
    char text[MAX_MSG_LEN];
} chat_message_t;

#endif
