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
    int servSock, cliSock;
    unsigned int len;
    char inbuf[BUFFSIZE], obuf[BUFFSIZE], buff[BUFFSIZE];
    struct sockaddr_in servSockAddr, cliSockAddr;

    servSock = socket(AF_INET, SOCK_STREAM, 0);
    if(servSock < 0){
        perror("socket() failed");
        exit(EXIT_FAILURE);
    }

    servSockAddr.sin_family = AF_INET;
    servSockAddr.sin_port = htons(10024+12700); //ポート番号を指定
    servSockAddr.sin_addr.s_addr = INADDR_ANY;

    if(bind(servSock, (struct sockaddr *)&servSockAddr, sizeof(servSockAddr)) < 0){
        perror("bind() failed");
        exit(EXIT_FAILURE);
    }

    if(listen(servSock, 5) < 0){
        perror("listen() failed");
        exit(EXIT_FAILURE);
    }

    while(1){
        len = sizeof(cliSockAddr);
        cliSock = accept(servSock, (struct sockaddr *) &cliSockAddr, &len);

        if(cliSock < 0){
            perror("accept() failed");
            exit(EXIT_FAILURE);
        }

        memset(inbuf, 0, sizeof(inbuf));
        recv(cliSock, inbuf, sizeof(inbuf), 0);
        printf("%s", inbuf);

        memset(obuf, 0, sizeof(obuf));
        snprintf(obuf, sizeof(obuf),
            "HTTP/1.0 200 OK\r\n"
            "Content-Type: text/html\r\n"
            "\r\n"
            "<h1>Hello</h1>\r\n"
        );

        send(cliSock, obuf, (int)strlen(obuf), 0);

        printf("connected from %s, port=%d.\n",
            (char *)inet_ntop(AF_INET, &cliSockAddr.sin_addr,
            buff, sizeof(buff)),
            ntohs(cliSockAddr.sin_port)
            );

        close(cliSock);
    }
    exit(EXIT_FAILURE);  
}