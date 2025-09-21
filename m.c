#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <pthread.h>
#include <time.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#define MAX_THREADS 10000
#define PACKET_SIZE 489
#define AUTHOR "Ritik"

typedef struct {
    int thread_id;
    char* target_ip;
    int port;
    int duration;
    int packet_count;
} thread_data_t;

// Function prototypes
void display_banner();
void show_usage(char* program_name);
void* network_diagnostic_thread(void* threadarg);
void simulate_ping_test(const char* ip, int port, int thread_id);
int isValidIP(const char* ip);

// Banner display
void display_banner() {
    printf("===============================================\n");
    printf("        Ritik's Network Diagnostic Tool\n");
    printf("===============================================\n");
    printf("Author: %s\n", AUTHOR);
    printf("Purpose: Educational network testing tool\n");
    printf("Warning: For authorized testing only\n");
    printf("===============================================\n\n");
}

// Show usage information
void show_usage(char* program_name) {
    printf("Usage: %s <IP> <PORT> <DURATION> <THREADS>\n", program_name);
    printf("Example: %s 192.168.1.1 80 10 5\n\n", program_name);
    printf("Parameters:\n");
    printf("  IP        - Target IP address\n");
    printf("  PORT      - Port number (1-65535)\n");
    printf("  DURATION  - Test duration in seconds\n");
    printf("  THREADS   - Number of threads (1-%d)\n\n", MAX_THREADS);
}

// Validate IP address format
int isValidIP(const char* ip) {
    struct sockaddr_in sa;
    return inet_pton(AF_INET, ip, &(sa.sin_addr)) != 0;
}

// Simulate network diagnostic
void simulate_ping_test(const char* ip, int port, int thread_id) {
    const char* actions[] = {
        "Testing network latency",
        "Checking packet loss",
        "Measuring jitter",
        "Analyzing route",
        "Testing bandwidth"
    };
    
    int action_count = sizeof(actions) / sizeof(actions[0]);
    int action_index = rand() % action_count;
    
    printf("[Thread %d] %s to %s:%d\n", 
           thread_id, actions[action_index], ip, port);
}

// Thread function for network diagnostics
void* network_diagnostic_thread(void* threadarg) {
    thread_data_t* data = (thread_data_t*) threadarg;
    time_t start_time = time(NULL);
    
    printf("[Thread %d] Starting diagnostic on %s:%d for %d seconds\n",
           data->thread_id, data->target_ip, data->port, data->duration);
    
    while(time(NULL) - start_time < data->duration) {
        simulate_ping_test(data->target_ip, data->port, data->thread_id);
        sleep(1 + (rand() % 2));
    }
    
    printf("[Thread %d] Diagnostic completed for %s:%d\n",
           data->thread_id, data->target_ip, data->port);
    
    pthread_exit(NULL);
}

int main(int argc, char* argv[]) {
    if (argc != 5) {
        show_usage(argv[0]);
        return 1;
    }
    
    display_banner();
    
    // Parse arguments
    char* target_ip = argv[1];
    int port = atoi(argv[2]);
    int duration = atoi(argv[3]);
    int num_threads = atoi(argv[4]);
    
    // Validate inputs
    if (!isValidIP(target_ip)) {
        printf("Error: Invalid IP address format\n");
        return 1;
    }
    
    if (port < 1 || port > 65535) {
        printf("Error: Port must be between 1-65535\n");
        return 1;
    }
    
    if (duration <= 0) {
        printf("Error: Duration must be positive\n");
        return 1;
    }
    
    if (num_threads <= 0 || num_threads > MAX_THREADS) {
        printf("Error: Threads must be between 1-%d\n", MAX_THREADS);
        return 1;
    }
    
    printf("Starting network diagnostic with parameters:\n");
    printf("  Target: %s:%d\n", target_ip, port);
    printf("  Duration: %d seconds\n", duration);
    printf("  Threads: %d\n\n", num_threads);
    
    // Initialize random seed
    srand(time(NULL));
    
    // Create threads
    pthread_t threads[MAX_THREADS];
    thread_data_t thread_data[MAX_THREADS];
    
    for (int i = 0; i < num_threads; i++) {
        thread_data[i].thread_id = i;
        thread_data[i].target_ip = target_ip;
        thread_data[i].port = port;
        thread_data[i].duration = duration;
        thread_data[i].packet_count = 0;
        
        if (pthread_create(&threads[i], NULL, network_diagnostic_thread, (void*)&thread_data[i])) {
            printf("Error creating thread %d\n", i);
            return 1;
        }
    }
    
    // Wait for all threads to complete
    for (int i = 0; i < num_threads; i++) {
        pthread_join(threads[i], NULL);
    }
    
    printf("\nNetwork diagnostic completed successfully.\n");
    printf("This tool is for educational purposes only.\n");
    printf("Always ensure you have permission before testing any network.\n");
    
    return 0;
}
