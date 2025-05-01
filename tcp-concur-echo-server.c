#include <stdio.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <ctype.h>
#include <arpa/inet.h>
#include <pthread.h>

//tcp-echo implementation
void *tcp_echo(void *arg) {
    int s = *(int *)arg;
    int byte_count = 0;
    int n;
    char buff[4096];

    while (1) {
        if ((n = recv(s, buff, sizeof(buff), 0)) < 0) {
            perror("Failed to receive data\n");
            return NULL;
        }

        if (n == 0) { 
            printf("Total bytes sent %d\n",byte_count);
            shutdown(s,SHUT_RDWR);
            close(s);
            pthread_exit(NULL);
            return NULL;
        }

        else {
            if (send(s, buff, n, 0) < 0) {
                perror("Failed to send data\n");
                return NULL;
            }
            byte_count += n;
        }
    }
    return NULL;
}

int main(int argc, char *argv[]) {
    int port = atoi(argv[1]);
    struct sockaddr_in sin;
    struct sockaddr_in client_addr;
    int s1, s2;
    socklen_t client_len = sizeof(client_addr);

    // check port number
    if (port < 0 || port > 65535) {
        perror("Invalid port number\n");
        return 1;
    }

    // create a TCP socket
    s1 = socket(AF_INET, SOCK_STREAM, 0);
    if (s1 < 0) {
        perror("Socket creation failed");
        return 1;
    }

    // bind the socket to INADDR_ANY and the specified port
    memset(&sin, 0, sizeof(sin));
    sin.sin_family = AF_INET;
    sin.sin_addr.s_addr = INADDR_ANY;
    sin.sin_port = htons((unsigned short)port);

    if (bind(s1, (struct sockaddr *)&sin, sizeof(sin)) < 0) {
        perror("Can't bind to the port");
        close(s1);
        return 1;
    }

    if (listen(s1, 10) < 0) { 
        perror("Listen failed");
        close(s1);
        return 1;
    }


    // accept incoming connections in separate threads
    while (1) {
        s2 = accept(s1, (struct sockaddr *)&client_addr, &client_len);

        int *client_socket = malloc(sizeof(int));
        *client_socket = s2;

        pthread_t thread;
        if (pthread_create(&thread, NULL, tcp_echo, client_socket) != 0) {
            perror("Thread creation failed");
            free(client_socket); 
            close(s2);
        } else {
            pthread_detach(thread);
        }
    }

    close(s1);
    return 0;
}