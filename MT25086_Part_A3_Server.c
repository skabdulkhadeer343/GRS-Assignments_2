/*
 * Roll No: MT25086
 * File: MT25086_Part_A3_Server.c
 * Description: Zero-Copy Server (MSG_ZEROCOPY)
 */

#include "MT25086_Part_A_Common.h"

#ifndef SO_ZEROCOPY
#define SO_ZEROCOPY 60
#endif
#ifndef MSG_ZEROCOPY
#define MSG_ZEROCOPY 0x4000000
#endif

void *handle_client(void *arg) {
    ServerThreadArgs *args = (ServerThreadArgs *)arg;
    int client_sock = args->sock_fd;
    size_t msg_size = args->msg_size;
    
    // Enable Zero Copy on this socket
    int one = 1;
    if (setsockopt(client_sock, SOL_SOCKET, SO_ZEROCOPY, &one, sizeof(one))) {
        perror("setsockopt zero copy failed");
        close(client_sock);
        return NULL;
    }

    Message msg;
    generate_data(&msg, msg_size);

    struct iovec iov[NUM_FIELDS];
    struct msghdr message_header = {0};

    for(int i=0; i<NUM_FIELDS; i++) {
        iov[i].iov_base = msg.fields[i];
        iov[i].iov_len = msg.field_size;
    }
    message_header.msg_iov = iov;
    message_header.msg_iovlen = NUM_FIELDS;

    // Buffer for reading error queue
    char cmsg_buf[CMSG_SPACE(sizeof(struct sock_extended_err))];
    struct msghdr msg_err;
    struct iovec iov_err;
    char err_buf[512]; // dummy

    while (1) {
        ssize_t sent = sendmsg(client_sock, &message_header, MSG_ZEROCOPY);
        if (sent == -1) {
            if (errno == ENOBUFS) {
                // Socket buffer full, wait and retry? or just continue processing completions
            } else if (errno != EAGAIN) {
                break; 
            }
        }
        if (sent <= 0 && errno != EAGAIN) break;

        // CRITICAL: We must reap the completion notifications from the error queue
        // Otherwise the socket runs out of optmem and stalls.
        // In a real app, we check if it is safe to reuse buffer. Here we just drain queue.
        memset(&msg_err, 0, sizeof(msg_err));
        iov_err.iov_base = err_buf;
        iov_err.iov_len = sizeof(err_buf);
        msg_err.msg_iov = &iov_err;
        msg_err.msg_iovlen = 1;
        msg_err.msg_control = cmsg_buf;
        msg_err.msg_controllen = sizeof(cmsg_buf);

        while (recvmsg(client_sock, &msg_err, MSG_ERRQUEUE | MSG_DONTWAIT) > 0);
    }

    free_data(&msg);
    close(client_sock);
    free(args);
    return NULL;
}

int main(int argc, char *argv[]) {
    // Boilerplate setup identical to A1/A2
    if (argc != 2) { fprintf(stderr, "Usage: %s <msg_size>\n", argv[0]); return 1; }
    size_t msg_size = atoi(argv[1]);
    int server_fd, new_socket;
    struct sockaddr_in address;
    int opt = 1;
    
    if ((server_fd = socket(AF_INET, SOCK_STREAM, 0)) == 0) exit(EXIT_FAILURE);
    setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR | SO_REUSEPORT, &opt, sizeof(opt));
    
    address.sin_family = AF_INET;
    address.sin_addr.s_addr = INADDR_ANY;
    address.sin_port = htons(PORT);
    bind(server_fd, (struct sockaddr *)&address, sizeof(address));
    listen(server_fd, 10);

    printf("A3 Server (Zero-Copy) listening on port %d\n", PORT);

    while (1) {
        socklen_t addrlen = sizeof(address);
        if ((new_socket = accept(server_fd, (struct sockaddr *)&address, &addrlen)) < 0) continue;
        pthread_t thread_id;
        ServerThreadArgs *args = malloc(sizeof(ServerThreadArgs));
        args->sock_fd = new_socket;
        args->msg_size = msg_size;
        pthread_create(&thread_id, NULL, handle_client, (void *)args);
        pthread_detach(thread_id);
    }
    return 0;
}