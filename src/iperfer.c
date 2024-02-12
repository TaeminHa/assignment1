#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <unistd.h>

#include <netdb.h>


#define PORT_MAX (1<<16)-1 // 65535
#define BUFFER_SIZE 1000
#define MAX_CLIENT 10

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
    struct sockaddr_in sin;
    char buffer[BUFFER_SIZE];

    /* 1. Create a TCP/IP socket with `socket` system call */
    // zero out struct
    bzero((char *)&sin, sizeof(sin));

    // set the protocol, address, and port
    sin.sin_family = AF_INET;
    sin.sin_addr.s_addr = INADDR_ANY;
    sin.sin_port = htons(port);

    // create socket 
    int server_fd;
    if ((server_fd = socket(PF_INET, SOCK_STREAM, 0)) < 0) {
        perror("simplex-talk: socket");
        exit(1);
    }

    /* 2. `bind` socket to the given port number */
    if ((bind(server_fd, (struct sockaddr *)&sin, sizeof(sin))) < 0) {
        perror("simplex-talk: bind");
        exit(1);
    }

    /* 3. `listen` for TCP connections */
    listen(server_fd, MAX_CLIENT);

    int client_fd, addr_len;
    /* 4. Wait for the client connection with `accept` system call */
    if ((client_fd = accept(server_fd, (struct sockaddr *)&sin, &addr_len)) < 0) {
        perror("simplex-talk: accept");
        exit(1);
    }

    /* 5. After the connection is established, receive data in chunks of 1000 bytes */
    int bytes_received = 0;
    double start = get_time();
    int buf_len;

    while (buf_len = recv(client_fd, buffer, sizeof(buffer), 0)){
        bytes_received += buf_len;
    }
    
    close(client_fd);

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
    struct sockaddr_in sin;
    char buffer[BUFFER_SIZE];

    /* 1. Create a TCP/IP socket with socket system call */
    /* translate host name into peer's IP address */
    struct hostent *hp;
    hp = gethostbyname(addr);
    if (!hp) {
        fprintf(stderr, "simplex-talk: unknown host: %s\n", addr);
        exit(1);
    }

    /* build address data structure */
    bzero((char *)&sin, sizeof(sin));
    sin.sin_family = AF_INET;
    bcopy(hp->h_addr, (char *)&sin.sin_addr, hp->h_length);
    sin.sin_port = htons(port);

    /* active open */
    int client_fd;
    if ((client_fd = socket(PF_INET, SOCK_STREAM, 0)) < 0) {
        perror("simplex-talk: socket");
        exit(1);
    }

    /* 2. `connect` to the server specified by arguments (`addr`, `port`) */
    if (connect(client_fd, (struct sockaddr *)&sin, sizeof(sin)) < 0)
    {
        perror("simplex-talk: connect");
        close(client_fd);
        exit(1);
    }

    /* 3. Send data to the connected server in chunks of 1000bytes */
    int bytes_sent = 0;
    double end = get_time() + duration;

    char msg[BUFFER_SIZE];

    while (get_time() < end) {
        bytes_sent += send(client_fd, msg, BUFFER_SIZE, 0);
    }

    /* 4. Close the connection after `duration` seconds */
    close(client_fd);

    /* 5. When the connection is closed, the program should print out the elapsed time, */
    /*    the total number of bytes sent (in kilobytes), and the rate */ 
    /*    at which the program sent data (in Mbps) */
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
        if (server_tcp_port < 0 || server_tcp_port > PORT_MAX) {
            fprintf(stderr, "TCP port is not within the valid range\n");
            exit(EXIT_FAILURE);
        }

        /* 2. Check the duration is a positive number */
        if (duration < 0) {
            fprintf(stderr, "Duration must be a positive number\n");
            exit(EXIT_FAILURE);
        }

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
        if (server_tcp_port < 0 || server_tcp_port > PORT_MAX) {
            fprintf(stderr, "TCP port is not within the valid range\n");
            exit(EXIT_FAILURE);
        }
        
        printf("Server mode, Port = %d\n", server_tcp_port);
        handle_server(server_tcp_port);
    }

    return 0;
}