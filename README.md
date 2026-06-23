# Concurrent Mini Chat Server

This project implements a concurrent TCP chat server using threads, sockets, synchronization, and an MPMC queue.

## Files

- `chat_server.c` — main server program and socket setup
- `client_handler.c` / `client_handler.h` — client handler thread logic
- `client_list.c` / `client_list.h` — shared client list and broadcast functions
- `worker.c` / `worker.h` — broadcaster worker thread logic
- `message.h` — chat message structure
- `mpmc_queue.c` / `mpmc_queue.h` — placeholder for your PA4 MPMC queue
- `client.c` — provided simple client program
- `Makefile`

## Build

```bash
make
```

This should produce:

```bash
./chat_server
./chat_client
```

## Run

In one terminal, run the server:

```bash
./chat_server
```

In two or more other terminals, run clients:

```bash
./chat_client 127.0.0.1 8082
```

Type a message in one client and press Enter. The message should be broadcast to the other connected clients.

To quit a client, type:

```bash
/quit
```

or press `Ctrl+C`.

## Important

The provided `mpmc_queue.c` is only a placeholder. You must replace it with your PA4 MPMC queue implementation.

Your server should use the queue as follows:

- Client handler threads push messages into the queue.
- Broadcaster worker threads pop messages from the queue.
- Messages should be dynamically allocated before being pushed.
- Messages should be freed after being broadcast.

## Clean

```bash
make clean
```
