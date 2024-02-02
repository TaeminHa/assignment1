#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <unistd.h>

#define PORT_MAX (1<<16)-1 // 65535
#define BUFFER_SIZE 1000
#define MAX_CLIENT 10

// TODO : Not sure if these should be global
struct sockaddr_in server_addr, client_addr;

/* get_time function */
/* Input: None */
/* Output: current time in seconds */
/* (double data type and ns precision) */
double
get_time(void) {
    struct timespec now;
    clock_gettime(CLOCK_REALTIME, &now);
    return now.tv_sec + (now.tv_nsec * 1e-9);
}

void
handle_server(int port) {
    /* TODO: Implement server mode operation here */
    // https://man7.org/linux/man-pages/man2/socket.2.html - man page with examples
    /* 1. Create a TCP/IP socket with `socket` system call */
    // AF_INET6 = IPv6 protocol; SOCK_STREAM = TCP, 0 = single protocol per socket
    int server_fd = socket(AF_INET6, SOCK_STREAM, 0);
    if (server_fd == -1) return;
    
    // create socket
    server_addr.sin_family = AF_INET;
    // IP address = INADDR_ANY (kernel chooses)
    server_addr.sin_addr.s_addr = htonl(INADDR_ANY);
    // Port number = parameter
    server_addr.sin_port = htons(port);

    /* 2. `bind` socket to the given port number */
    bind(server_fd, (struct sockaddr*)&server_addr, sizeof(server_addr));
    /* 3. `listen` for TCP connections */
    listen(server_fd, MAX_CLIENT);
    /* 4. Wait for the client connection with `accept` system call */
    socklen_t client_length = sizeof(client_addr);
    // client_addr and client_length are resultant parameter
    int client_fd = accept(server_fd,  (struct sockaddr*)&client_addr, &client_length);
    /* 5. After the connection is established, received data in chunks of 1000 bytes */
    int bytes_received = 0;
    double start = get_time();

    // TODO : How does the server know when connection closes
    // like what goes into the while (???)
    char* msg;
    memset(msg, 0, BUFFER_SIZE);
    while (recv(server_fd, msg, BUFFER_SIZE, 0) >= 0) {
        // do sth
        bytes_received += BUFFER_SIZE;
    }

    /* 6. When the connection is closed, the program should print out the elapsed time, */
    /*    the total number of bytes received (in kilobytes), and the rate */ 
    /*    at which the program received data (in Mbps) */
    double duration = get_time() - start;
    printf("%f kilobytes received in %f seconds at the rate of %f Mbps", bytes_received/1000.0, duration, ((bytes_received/1e6) / duration));

    return;
}

void
handle_client(const char *addr, int port, int duration) {
    /* TODO: Implement client mode operation here */
    /* 1. Create a TCP/IP socket with socket system call */
    int client_fd = socket(AF_INET6, SOCK_STREAM, 0);
    if (client_fd == -1) return;
    
    // create socket
    client_addr.sin_family = AF_INET;
    // IP address = INADDR_ANY (kernel chooses)
    client_addr.sin_addr.s_addr = htonl(INADDR_ANY);
    // Port number = parameter
    client_addr.sin_port = htons(port);

    /* 2. `connect` to the server specified by arguments (`addr`, `port`) */
    int connect_status = connect(client_fd, (struct sockaddr *) &server_addr, sizeof(server_addr));
    if (connect_status == -1) return;
    /* 3. Send data to the connected server in chunks of 1000bytes */
    // segfault bc we need to malloc first i think
    char* msg;
    memset(msg, 0, BUFFER_SIZE);

    int bytes_sent = 0;
    double end = get_time() + duration;
    while (get_time() < end) {
        send(client_fd, msg, BUFFER_SIZE, 0);
        bytes_sent += BUFFER_SIZE;
    }
    /* 4. Close the connection after `duration` seconds */
    close(client_fd);
    /* 5. When the connection is closed, the program should print out the elapsed time, */
    /*    the total number of bytes sent (in kilobytes), and the rate */ 
    /*    at which the program sent data (in Mbps) */
    //?????
    printf("%f kilobytes sent in %f seconds at the rate of %f Mbps", bytes_sent/1000.0, duration * 1.0, ((bytes_sent/1e6) / duration));
    return;
}

int
main(int argc, char *argv[]) {
    /* argument parsing */
    int mode = 0, server_tcp_port = 0, duration = 0;
    char *server_host_ipaddr = NULL;

    int opt;
    while ((opt = getopt(argc, argv, "csh:p:t:")) != -1) {
        switch (opt) {
            case 'c':
                mode = 1;
                break;
            case 's':
                mode = 2;
                break;
            case 'h':
                server_host_ipaddr = optarg;
                break;
            case 'p':
                server_tcp_port = atoi(optarg);
                break;
            case 't':
                duration = atoi(optarg);
                break;
            default:
                fprintf(stderr, "Usage: %s -c -h <server_host_ipaddr> -p <server_tcp_port> -t <duration_in_sec>\n", argv[0]);
                exit(EXIT_FAILURE);
        }
    }

    if (mode == 0) {
        fprintf(stderr, "Please specify either -c (client mode) or -s (server mode).\n");
        exit(EXIT_FAILURE);
    }

    if (mode == 1) {
        if (server_host_ipaddr == NULL || duration == 0 || server_tcp_port == 0) {
            fprintf(stderr, "Client mode requires -h, -p, and -t options.\n");
            exit(EXIT_FAILURE);
        }

        /* TODO: Implement argument check here */
        /* 1. Check server_tcp_port is within the port number range */
        /* 2. Check the duration is a positive number */

        printf("Client mode: Server IP = %s, Port = %d, Time Window = %d\n", server_host_ipaddr, server_tcp_port, duration);
        handle_client (server_host_ipaddr, server_tcp_port, duration);

    } else if (mode == 2) {
        // Server mode logic goes here
        if (server_tcp_port == 0) {
            fprintf(stderr, "Client mode requires -p option.\n");
            exit(EXIT_FAILURE);
        }

        /* TODO: Implement argument check here */
        /* Check server_tcp_port is within the port number range */
        
        printf("Server mode, Port = %d\n", server_tcp_port);
        handle_server(server_tcp_port);
    }

    return 0;
}