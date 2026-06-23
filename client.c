#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <pthread.h>
#include <arpa/inet.h>
#include <sys/socket.h>

#define MAX_MSG_LEN 1024

static volatile int running = 1;

typedef struct {
    int sockfd;
} client_arg_t;

static void *receive_thread(void *arg)
{
    client_arg_t *client_arg = (client_arg_t *)arg;
    int sockfd = client_arg->sockfd;
    char buffer[MAX_MSG_LEN];

    while (running) {
        memset(buffer, 0, sizeof(buffer));
        ssize_t n = recv(sockfd, buffer, sizeof(buffer) - 1, 0);

        if (n <= 0) {
            printf("\nDisconnected from server.\n");
            running = 0;
            break;
        }

        printf("%s", buffer);
        fflush(stdout);
    }

    return NULL;
}

static int connect_to_server(const char *server_ip, int port)
{
    int sockfd;
    struct sockaddr_in server_addr;

    sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd < 0) {
        perror("socket failed");
        exit(EXIT_FAILURE);
    }

    memset(&server_addr, 0, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(port);

    if (inet_pton(AF_INET, server_ip, &server_addr.sin_addr) <= 0) {
        perror("inet_pton failed");
        close(sockfd);
        exit(EXIT_FAILURE);
    }

    if (connect(sockfd, (struct sockaddr *)&server_addr, sizeof(server_addr)) < 0) {
        perror("connect failed");
        close(sockfd);
        exit(EXIT_FAILURE);
    }

    return sockfd;
}

int main(int argc, char *argv[])
{
    if (argc != 3) {
        fprintf(stderr, "Usage: %s <server_ip> <port>\n", argv[0]);
        fprintf(stderr, "Example: %s 127.0.0.1 8082\n", argv[0]);
        return EXIT_FAILURE;
    }

    const char *server_ip = argv[1];
    int port = atoi(argv[2]);

    int sockfd = connect_to_server(server_ip, port);
    printf("Connected to chat server. Type messages and press Enter.\n");
    printf("Type /quit to exit.\n");

    client_arg_t client_arg;
    client_arg.sockfd = sockfd;

    pthread_t recv_tid;
    if (pthread_create(&recv_tid, NULL, receive_thread, &client_arg) != 0) {
        perror("pthread_create failed");
        close(sockfd);
        return EXIT_FAILURE;
    }

    char line[MAX_MSG_LEN];
    while (running && fgets(line, sizeof(line), stdin) != NULL) {
        if (strncmp(line, "/quit", 5) == 0) {
            running = 0;
            break;
        }

        ssize_t n = send(sockfd, line, strlen(line), 0);
        if (n < 0) {
            perror("send failed");
            running = 0;
            break;
        }
    }

    close(sockfd);
    running = 0;
    pthread_cancel(recv_tid);
    pthread_join(recv_tid, NULL);

    return EXIT_SUCCESS;
}
