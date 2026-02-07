/*
 * Roll No: MT25086
 * File: MT25086_Part_A1_Server.c
 * Description: Two-Copy Server Implementation
 */

#include "MT25086_Part_A_Common.h"

void *handle_client(void *arg) {
    ServerThreadArgs *args = (ServerThreadArgs *)arg;
    int client_sock = args->sock_fd;
    size_t msg_size = args->msg_size;
    
    // Prepare data
    Message msg;
    generate_data(&msg, msg_size);

    // Buffer for serialization (Two-Copy: Copy to user buffer first)
    char *send_buffer = (char *)malloc(msg_size);
    if (!send_buffer) {
        perror("Buffer malloc failed");
        close(client_sock);
        return NULL;
    }

    // Serialize: Copy 8 fields into one linear buffer
    size_t offset = 0;
    for (int i = 0; i < NUM_FIELDS; i++) {
        memcpy(send_buffer + offset, msg.fields[i], msg.field_size);
        offset += msg.field_size;
    }

    // Send loop
    while (1) {
        ssize_t sent = send(client_sock, send_buffer, msg_size, 0);
        if (sent <= 0) break; // Client disconnected
    }

    free(send_buffer);
    free_data(&msg);
    close(client_sock);
    free(args);
    return NULL;
}

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

    if ((server_fd = socket(AF_INET, SOCK_STREAM, 0)) == 0) {
        perror("Socket failed");
        exit(EXIT_FAILURE);
    }

    if (setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR | SO_REUSEPORT, &opt, sizeof(opt))) {
        perror("Setsockopt failed");
        exit(EXIT_FAILURE);
    }

    address.sin_family = AF_INET;
    address.sin_addr.s_addr = INADDR_ANY;
    address.sin_port = htons(PORT);

    if (bind(server_fd, (struct sockaddr *)&address, sizeof(address)) < 0) {
        perror("Bind failed");
        exit(EXIT_FAILURE);
    }

    if (listen(server_fd, 10) < 0) {
        perror("Listen failed");
        exit(EXIT_FAILURE);
    }

    printf("A1 Server (Two-Copy) listening on port %d with msg_size %lu\n", PORT, msg_size);

    while (1) {
        if ((new_socket = accept(server_fd, (struct sockaddr *)&address, (socklen_t *)&addrlen)) < 0) {
            perror("Accept failed");
            continue;
        }

        pthread_t thread_id;
        ServerThreadArgs *args = malloc(sizeof(ServerThreadArgs));
        args->sock_fd = new_socket;
        args->msg_size = msg_size;

        if (pthread_create(&thread_id, NULL, handle_client, (void *)args) != 0) {
            perror("Thread creation failed");
            free(args);
        } else {
            pthread_detach(thread_id);
        }
    }
    return 0;
}