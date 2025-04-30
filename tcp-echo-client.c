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
#include <time.h>

//global variables
int nbytes;
int strt;
int incr;
int s;

//client_snd implementation
void *client_snd() {
    char buff[1024];  
    int next_value = strt;
    int remaining_bytes = nbytes;
    int index = 0;

    while (remaining_bytes > 0) {
        if (index < 1024) {
            buff[index++] = (char) next_value;
            remaining_bytes -= 1;
            next_value = (next_value + incr) % 256;
        } 
        else {
            if (send(s, buff, index, 0) < 0){
                perror("Failed to send data\n");
                return NULL;
            }
            memset(buff, 0, sizeof(buff));
            index = 0;
        }
    }

    // if there is any remaining data in the buffer, send it
    if (index > 0) {
        if (send(s, buff, index, 0) < 0){
            perror("Failed to send data\n");
            return NULL;
        }
    }
    shutdown(s, SHUT_WR);
    
    return NULL;
}

//client_rcv implementation
void *client_rcv() {
    char buff_check[1024];
    char buff_rcv[1024];
    int bytes_received = 0;
    int n;
    int next_value = strt;
    int index;

    while ((n = recv(s, buff_rcv, sizeof(buff_rcv), 0)) > 0) {
        bytes_received += n;
        index = 0;
        memset(buff_check, 0, sizeof(buff_check));
        while (index < n) {
            buff_check[index++] = (char) next_value;
            next_value = (next_value + incr) % 256;
        }

        if (memcmp(buff_check, buff_rcv, n) != 0) {
            perror("Buffer received does not match the expected buffer\n");
            exit(1);
        }
    }

    if (bytes_received != nbytes) {
        perror("Incorrect number of bytes received\n");
        exit(1);
    }

    return NULL;
}

int main (int argc, char * argv []){
    //check number of arguments
    if (argc != 4) {
        perror("Wrong number of arguments.\n");
        return 1;
    }

    char *host = argv[1];
    int port = atoi(argv[2]);
    int set[] = {1, 3, 5, 7, 11, 13, 17};
    struct sockaddr_in sin;
    pthread_t thread1, thread2;
    nbytes = atoi(argv[3]);

    if (strcmp(host, "localhost") == 0) {
        host = "127.0.0.1";
    }

    //check nbytes
    if(nbytes <= 0){
        perror("Invalid number of bytes\n");
        return 1;
    }
    //check port number
    if(port < 0 || port > 65535){
        perror("Invalid port number\n");
        return 1;
    }

    //initialize strt,incr
    srand(time(NULL));
    strt = rand() % 255;
    incr = set[rand() % 7];

    //create a global TCP socket (SOCK STREAM), s
    s = socket(AF_INET, SOCK_STREAM, 0);

    if (s < 0) {
        perror("Socket creation failed");
        return 1;
    }
    
    //connect the socket to the computer and port specified by the arguments
    memset(&sin,0,sizeof(sin));
    sin.sin_family = AF_INET;
    sin.sin_port = htons((unsigned short)port); 
    if (inet_pton(AF_INET, host, &sin.sin_addr) <= 0) {
        perror(" Address not supported");
        return 1;
    }
    
    if (connect(s, (struct sockaddr*)&sin, sizeof(sin)) < 0) {
        perror("Connection failed");
        return 1;
    }
    
    //create two threads to execute functions client snd and client rec
    pthread_create(&thread1,NULL,client_snd,NULL);
    pthread_create(&thread2,NULL,client_rcv,NULL);

    pthread_join(thread1, NULL);
    pthread_join(thread2, NULL);

    printf("Total bytes sent and received %d\n",nbytes);
  
    //exit
    close(s);

    return 0;
}