/*
 * Roll No: MT25086
 * File: MT25086_Part_A1_Client.c
 * Description: Standard Client Implementation (Used for A1, compatible with others)
 */

#include "MT25086_Part_A_Common.h"

volatile int keep_running = 1;

void *client_thread(void *arg) {
    ClientThreadArgs *args = (ClientThreadArgs *)arg;
    int sock = 0;
    struct sockaddr_in serv_addr;
    char *buffer = malloc(args->msg_size);
    long long total_bytes = 0;
    
    if ((sock = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
        printf("\n Socket creation error \n");
        return NULL;
    }

    serv_addr.sin_family = AF_INET;
    serv_addr.sin_port = htons(args->port);

    if (inet_pton(AF_INET, args->server_ip, &serv_addr.sin_addr) <= 0) {
        printf("\nInvalid address/ Address not supported \n");
        return NULL;
    }

    if (connect(sock, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0) {
        printf("\nConnection Failed \n");
        return NULL;
    }

    struct timespec start, end;
    clock_gettime(CLOCK_MONOTONIC, &start);

    while (keep_running) {
        int valread = recv(sock, buffer, args->msg_size, MSG_WAITALL);
        if (valread <= 0) break;
        total_bytes += valread;
    }

    clock_gettime(CLOCK_MONOTONIC, &end);
    long long duration = time_diff_us(start, end);
    double seconds = duration / 1000000.0;
    double throughput = (total_bytes * 8) / (seconds * 1e9); // Gbps

    // CSV Output: ThreadID, Throughput(Gbps), Latency(us - approx per msg)
    // printf("%d,%.4f,%.2f\n", args->thread_id, throughput, (double)duration/(total_bytes/args->msg_size));
    
    close(sock);
    free(buffer);
    return (void*)(size_t)total_bytes;
}

int main(int argc, char *argv[]) {
    if (argc != 5) {
        fprintf(stderr, "Usage: %s <server_ip> <msg_size> <num_threads> <duration_sec>\n", argv[0]);
        return 1;
    }

    char *server_ip = argv[1];
    size_t msg_size = atoi(argv[2]);
    int num_threads = atoi(argv[3]);
    int duration = atoi(argv[4]);

    pthread_t threads[num_threads];
    ClientThreadArgs args[num_threads];

    for (int i = 0; i < num_threads; i++) {
        args[i].thread_id = i;
        args[i].server_ip = server_ip;
        args[i].port = PORT;
        args[i].msg_size = msg_size;
        args[i].duration = duration;
        pthread_create(&threads[i], NULL, client_thread, &args[i]);
    }

    sleep(duration);
    keep_running = 0;

    long long total_bytes_all = 0;
    for (int i = 0; i < num_threads; i++) {
        void *retval;
        pthread_join(threads[i], &retval);
        total_bytes_all += (size_t)retval;
    }
    
    // Print aggregate throughput for the script to capture
    double total_throughput = (total_bytes_all * 8.0) / (duration * 1e9);
    printf("TOTAL_THROUGHPUT: %.6f\n", total_throughput);

    return 0;
}