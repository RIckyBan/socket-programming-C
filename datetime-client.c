#include <stdio.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <stdlib.h>
#include <errno.h>

#define BUFFSIZE 2048

int main(int argc, char *argv[]){
    int INET_PROTOCL, sock, s, n;
    char destip[100], destport[100], buf[BUFFSIZE];

    if(argc < 2){
        printf("Please specify dest address.\n");
        return 0;
    }

    strcpy(destip, argv[1]);
    printf("Dest address: %s\n", destip);

    strcpy(destport, "13");
    printf("Dest port number: %s\n", destport);

    char *p = strrchr(destip, ':');

    if(p == NULL){
        struct sockaddr_in server;
        server.sin_family = AF_INET;
        server.sin_port = htons(atoi(destport));
        sock = socket(AF_INET, SOCK_STREAM, 0);

        if (sock < 0) {
            perror("Socket failed.\n");
            return 0;
        }

        s = inet_pton(AF_INET, destip, &server.sin_addr);

        connect(sock, (struct sockaddr *)&server,
                sizeof(server));
    }
    else {
        struct sockaddr_in6 server;
        server.sin6_family = AF_INET6;
        server.sin6_port = htons(atoi(destport));
        sock = socket(AF_INET6, SOCK_STREAM, 0);

        if (sock < 0) {
            perror("Socket failed.\n");
            return 0;
        }

        s = inet_pton(AF_INET6, destip, &server.sin6_addr);

        connect(sock, (struct sockaddr *)&server,
                sizeof(server));
    }

    memset(buf, 0, sizeof(buf));
    n = read(sock, buf, sizeof(buf));

    if(n < 0){
        perror("Error reading from socket.\n");
        return 0;
    }

    printf("\n%s\n", buf);
}

