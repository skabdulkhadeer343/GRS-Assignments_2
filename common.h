/*
 * Roll No: MT25086
 * File: MT25086_Part_A_Common.h
 * Description: Common definitions and helper functions for GRS PA02
 */

#ifndef COMMON_H
#define COMMON_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <time.h>
#include <pthread.h>
#include <sys/uio.h>
#include <linux/errqueue.h>
#include <netinet/tcp.h>

#define PORT 8080
#define NUM_FIELDS 8

// Structure for the message fields
typedef struct {
    char *fields[NUM_FIELDS];
    size_t field_size;
} Message;

// Arguments passed to client threads
typedef struct {
    int thread_id;
    char *server_ip;
    int port;
    size_t msg_size;
    int duration;
} ClientThreadArgs;

// Arguments passed to server threads
typedef struct {
    int sock_fd;
    size_t msg_size;
} ServerThreadArgs;

// Helper to generate random data
void generate_data(Message *msg, size_t total_size) {
    msg->field_size = total_size / NUM_FIELDS;
    for (int i = 0; i < NUM_FIELDS; i++) {
        msg->fields[i] = (char *)malloc(msg->field_size);
        if (!msg->fields[i]) {
            perror("Malloc failed");
            exit(EXIT_FAILURE);
        }
        memset(msg->fields[i], 'A' + i, msg->field_size); // Fill with dummy data
    }
}

// Helper to free data
void free_data(Message *msg) {
    for (int i = 0; i < NUM_FIELDS; i++) {
        free(msg->fields[i]);
    }
}

// Helper for time difference in microseconds
long long time_diff_us(struct timespec start, struct timespec end) {
    return (end.tv_sec - start.tv_sec) * 1000000LL + (end.tv_nsec - start.tv_nsec) / 1000;
}

#endif