#include <stdio.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <stdlib.h>
#include <pthread.h>
#include <errno.h>

#define BUFFSIZE 2048
#define LISTENQ 5

struct args {
    struct sockaddr_in sockAddr;
    int sock;
};

static void *send_http_response(void *p){
    pthread_detach(pthread_self());

    char inbuf[BUFFSIZE], obuf[BUFFSIZE], buff[BUFFSIZE];
    struct args *pCliInfo = (struct args *)p;

    memset(inbuf, 0, sizeof(inbuf));
    recv(pCliInfo->sock, inbuf, sizeof(inbuf), 0);
    printf("%s", inbuf);

    memset(obuf, 0, sizeof(obuf));
    snprintf(obuf, sizeof(obuf),
        "HTTP/1.0 200 OK\r\n"
        "Content-Type: text/html\r\n"
        "\r\n"
        "<h1>Hello</h1>\r\n"
    );

    send(pCliInfo->sock, obuf, (int)strlen(obuf), 0);

    printf("connected from %s, port=%d.\n", 
        (char *)inet_ntop(AF_INET, &pCliInfo->sockAddr.sin_addr, 
        buff, sizeof(buff)),
        ntohs(pCliInfo->sockAddr.sin_port)
        );

    close((int)pCliInfo->sock);
    pthread_exit((void *)0);
}

int main(int argc, char *argv[]){
    int servSock;
    struct sockaddr_in servSockAddr;

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

    if(listen(servSock, LISTENQ) < 0){
        perror("listen() failed");
        exit(EXIT_FAILURE);
    }

    while(1){
        struct args cliInfo;
        unsigned int len;

        len = sizeof(cliInfo.sockAddr);
        cliInfo.sock = accept(servSock, (struct sockaddr *) &cliInfo.sockAddr, &len);
        
        if(cliInfo.sock < 0){
            perror("accept() failed");
            exit(EXIT_FAILURE);
        }
        
        pthread_t tid;
        pthread_create(&tid, NULL, &send_http_response, (void *) &cliInfo);
    }
}