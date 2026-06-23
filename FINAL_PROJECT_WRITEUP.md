# Final Project
# Concurrent Mini Chat Server with MPMC Queue

**100 points**  
**Assigned:** May 25, 2026
**Due:** Jun 8, 2026 at 11:59pm

## Objective

The goal of this final project is to integrate several systems programming concepts into one working application. You will build a concurrent TCP chat server in C that accepts multiple clients, receives messages from clients, places messages into a shared MPMC queue, and uses worker threads to broadcast messages to all connected clients.

This project connects the following concepts:

- Network programming using sockets
- Threads and concurrency
- Synchronization using mutexes
- Producer-consumer design
- Integration of your MPMC queue from Programming Assignment 4

You are provided with starter code for the server, a simple client program, a Makefile, and a README. Your main task is to complete the missing server logic and integrate your PA4 MPMC queue.

## Computing Resource

Please ssh into Linux CDM:

- `ssh -J <username>@sshjump.depaul.edu <username>@cdmlinux.cdm.depaul.edu`

or use your own x86 Linux machine.

Your code must compile and run in a Linux environment.

## Project Description

You will implement a concurrent chat server. Multiple clients should be able to connect to the server at the same time. When one client sends a message, the server should broadcast that message to the other connected clients.

The server should use the following design:

```text
Client handler threads receive messages from clients.
These threads act as producers.

Messages are inserted into a shared MPMC queue.

Broadcaster worker threads remove messages from the queue.
These threads act as consumers.

The broadcaster workers send each message to the connected clients.
```

This design decouples network receive operations from message broadcasting. The MPMC queue acts as the synchronization point between producer threads and consumer threads.

## Provided Files

The starter code includes the following files:

- `chat_server.c` — main server program and socket setup
- `client_handler.c` — client-handling thread logic
- `client_handler.h`
- `client_list.c` — shared client list functions
- `client_list.h`
- `worker.c` — broadcaster worker thread logic
- `worker.h`
- `message.h` — message structure
- `mpmc_queue.c` — placeholder queue file to replace with your PA4 queue
- `mpmc_queue.h` — queue interface
- `client.c` — simple provided chat client
- `Makefile`
- `README.md`

## Instructions

### 1. Review the Starter Code

Begin by reviewing the provided files. Understand how the server is organized across multiple files.

You should identify:

- Where the server socket is created
- Where clients are accepted
- Where client handler threads are created
- Where broadcaster worker threads are created
- Where the shared client list is stored
- Where the MPMC queue should be initialized, pushed to, and popped from

### 2. Integrate Your PA4 MPMC Queue

You must integrate your MPMC queue from Programming Assignment 4.

Your queue should support at least the following operations:

```c
void mpmc_queue_init(mpmc_queue_t *q);
void mpmc_queue_push(mpmc_queue_t *q, void *item);
void *mpmc_queue_pop(mpmc_queue_t *q);
void mpmc_queue_destroy(mpmc_queue_t *q);
```

The queue should store pointers to dynamically allocated `chat_message_t` objects.

Each message should be allocated when a client sends a message and freed after the message has been broadcast.

### 3. Implement the Shared Client List

The server must maintain a shared list of connected clients.

You must implement:

```c
void add_client(client_list_t *list, int client_fd);
void remove_client(client_list_t *list, int client_fd);
void broadcast_message(client_list_t *list, chat_message_t *msg);
```

The client list is shared by multiple threads, so it must be protected using a mutex.

You should think carefully about the following:

- What happens if one thread is broadcasting while another thread removes a client?
- What happens if two clients connect or disconnect at the same time?
- Why does the client list require synchronization?

### 4. Implement Client Handler Threads

Each connected client should be handled by a client handler thread.

The client handler thread should:

1. Add the client to the shared client list.
2. Repeatedly receive messages from the client using `recv()`.
3. Allocate a `chat_message_t` object.
4. Store the sender socket and message text in the message object.
5. Push the message into the MPMC queue.
6. Remove the client when the client disconnects.
7. Close the client socket.

Client handler threads are the **producer threads** in this project.

### 5. Implement Broadcaster Worker Threads

The server should create one or more broadcaster worker threads.

Each broadcaster worker should:

1. Pop a message from the MPMC queue.
2. Broadcast the message to the connected clients.
3. Free the message after it has been processed.

Broadcaster worker threads are the **consumer threads** in this project.

### 6. Test the Server and Client

## How to Compile and Run

Compile the project:

```bash
make
#Run the server in one terminal:
./chat_server
#Run the first client in another terminal:
./chat_client 127.0.0.1 8082
#Run the second client in a third terminal:
./chat_client 127.0.0.1 8082

Then type messages in one client terminal and verify that the other clients receive the messages.

You should also test client disconnects by closing a client with `Ctrl+C`.

How to Run the Server:

For the video demo, run the server in one terminal:

./chat_server

Open two other terminals and run the clients:

./chat_client 127.0.0.1 8082

When a client sends a message, the server should broadcast the message to all clients except the client that originated the message.

## Code Walkthrough Video

Instead of a written analysis report, you must submit a short video explaining your project.

Your video should be **up to 5 minutes total**:

- **3–4 minutes:** code walkthrough
- **1 minute:** working demo

If your video does not clearly explain your C code, you will receive a **zero for the assignment**. The goal is to demonstrate that you understand the code you submitted.

Your video should address the following:

1. **Overall Design**
   - Explain the architecture of your chat server.
   - Identify the producer threads and consumer threads.
   - Explain why an MPMC queue is appropriate for this project.

2. **MPMC Queue Integration**
   - Show where your PA4 MPMC queue is initialized.
   - Show where messages are pushed into the queue.
   - Show where messages are popped from the queue.
   - Explain what data type your queue stores.
   - Explain how memory is allocated and freed for messages.

3. **Synchronization**
   - Show the shared client list.
   - Explain how you protect the client list using mutexes.
   - Explain what could go wrong if the client list were not synchronized.

4. **Networking**
   - Explain how each client connection is handled.
   - Explain how your server handles client disconnects.

## Submission Requirements

Please submit the following in a zip to D2L:

- All C source files
- All header files
- Makefile
- README with instructions to run the code (already provided, but please update as necessary)

Please submit the video file by directly uploading it to D2L or Panopto or Youtube with a link (and permissions) to access.

At minimum, your submission should include to receive a grade:

- `chat_server.c`
- `client_handler.c`
- `client_handler.h`
- `client_list.c`
- `client_list.h`
- `worker.c`
- `worker.h`
- `message.h`
- `mpmc_queue.c`
- `mpmc_queue.h`
- `client.c`
- `Makefile`
- `README.md`
- Code walkthrough video

## Grading Criteria

- C code correctness and functionality – 40 points
- MPMC queue integration – 20 points
- Code walkthrough video – 40 points

## Notes

This project focuses on correctness, synchronization, and integration of systems concepts. You are not required to perform performance measurements for this project.

However, you should still think carefully about concurrency. Your server should avoid data races, should not crash when clients disconnect, and should correctly use the MPMC queue to decouple network receive threads from broadcaster worker threads.
