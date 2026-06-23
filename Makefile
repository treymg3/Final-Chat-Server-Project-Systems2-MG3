CC = gcc
CFLAGS = -Wall -Wextra -O2 -pthread

SERVER_OBJS = chat_server.o client_handler.o client_list.o worker.o mpmc_queue.o
CLIENT_OBJS = client.o

all: chat_server chat_client

chat_server: $(SERVER_OBJS)
	$(CC) $(CFLAGS) -o chat_server $(SERVER_OBJS)

chat_client: $(CLIENT_OBJS)
	$(CC) $(CFLAGS) -o chat_client $(CLIENT_OBJS)

%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

clean:
	rm -f *.o chat_server chat_client

.PHONY: all clean
