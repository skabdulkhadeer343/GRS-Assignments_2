/*
 * Roll No: MT25086
 * File: MT25086_Part_A2_Server.c
 * Description: One-Copy Server (Scatter-Gather I/O)
 */

#include "MT25086_Part_A_Common.h"

void *handle_client(void *arg) {
    ServerThreadArgs *args = (ServerThreadArgs *)arg;
    int client_sock = args->sock_fd;
    size_t msg_size = args->msg_size;
    
    Message msg;
    generate_data(&msg, msg_size);

    // Prepare iovec for Scatter-Gather (Eliminates User Copy)
    struct iovec iov[NUM_FIELDS];
    struct msghdr message_header = {0};

    for(int i=0; i<NUM_FIELDS; i++) {
        iov[i].iov_base = msg.fields[i];
        iov[i].iov_len = msg.field_size;
    }

    message_header.msg_iov = iov;
    message_header.msg_iovlen = NUM_FIELDS;

    while (1) {
        ssize_t sent = sendmsg(client_sock, &message_header, 0);
        if (sent <= 0) break;
    }

    free_data(&msg);
    close(client_sock);
    free(args);
    return NULL;
}

// Main function is identical to A1, only logic inside handle_client changes.
// Retaining full main for modularity as requested.
int main(int argc, char *argv[]) {
    if (argc != 2) {
        fprintf(stderr, "Usage: %s <msg_size>\n", argv[0]);
        return 1;
    }
    size_t msg_size = atoi(argv[1]);
    int server_fd, new_socket;
    struct sockaddr_in address;
    int opt = 1;
    int addrlen = sizeof(address);

    if ((server_fd = socket(AF_INET, SOCK_STREAM, 0)) == 0) exit(EXIT_FAILURE);
    setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR | SO_REUSEPORT, &opt, sizeof(opt));
    
    address.sin_family = AF_INET;
    address.sin_addr.s_addr = INADDR_ANY;
    address.sin_port = htons(PORT);
    bind(server_fd, (struct sockaddr *)&address, sizeof(address));
    listen(server_fd, 10);

    printf("A2 Server (One-Copy) listening on port %d\n", PORT);

    while (1) {
        if ((new_socket = accept(server_fd, (struct sockaddr *)&address, (socklen_t *)&addrlen)) < 0) continue;
        pthread_t thread_id;
        ServerThreadArgs *args = malloc(sizeof(ServerThreadArgs));
        args->sock_fd = new_socket;
        args->msg_size = msg_size;
        pthread_create(&thread_id, NULL, handle_client, (void *)args);
        pthread_detach(thread_id);
    }
    return 0;
}