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

struct args {
    struct sockaddr_in sockAddr;
    int sock, proxysock;
    char destip[100];  
    char destport[100]; 
};

static void *bridge_http_response(void *p){
    pthread_detach(pthread_self());

    char inbuf[BUFFSIZE], obuf[BUFFSIZE], buff[BUFFSIZE];
    struct args *pCliInfo = (struct args *)p;

    // responseをサーバーから取得する
    struct sockaddr_in servSockAddr;
    int servSock = 0, s;
        
    printf("connect to %s, port=%s.\n",
        pCliInfo->destip, pCliInfo->destport
        );

    servSockAddr.sin_family = AF_INET;
    servSockAddr.sin_port = htons(atoi(pCliInfo->destport));
    if ((servSock = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
        perror("Failed to create server socket.\n");
        exit(EXIT_FAILURE);
    }
    printf("Server socket created.\n");

    s = inet_pton(AF_INET, pCliInfo->destip, &servSockAddr.sin_addr);
    if(connect(servSock, (struct sockaddr *)&servSockAddr, sizeof(servSockAddr)) < 0) {
        perror("Server connection failed to establish.\n");
        exit(EXIT_FAILURE);
    }
    printf("server socket connected.\n\n");

    memset(inbuf, 0, sizeof(inbuf));
    int n = recv(pCliInfo->sock, inbuf, sizeof(inbuf), 0);
    printf("%s", inbuf);
    if(n >= 0){
        send(servSock, inbuf, (int)strlen(inbuf), 0);
        
        memset(obuf, 0, sizeof(obuf));
        int n = read(servSock, obuf, sizeof(obuf));

        send(pCliInfo->sock, obuf, (int)strlen(obuf), 0);
    }

    close(pCliInfo->sock);
    pthread_exit((void *)0);
}

int main(int argc, char *argv[]){
    int proxySock = 0, cliSock = 0;
    unsigned int len;
    char destip[100], destport[100], proxyport[100], inbuf[BUFFSIZE], obuf[BUFFSIZE], buff[BUFFSIZE];
    struct sockaddr_in proxySockAddr, cliSockAddr;

    if(argc < 4){
        printf("Please specify (dest address, dest port, proxy port).\n");
        exit(EXIT_FAILURE); 
    }

    strcpy(destip, argv[1]);
    printf("Dest address: %s\n", destip);

    strcpy(destport, argv[2]);
    printf("Dest port number: %s\n", destport);

    strcpy(proxyport, argv[3]);
    printf("Proxy port number: %s\n", proxyport);

    if((proxySock = socket(AF_INET, SOCK_STREAM, 0)) < 0) {  
          perror("Failed to create proxy socket");
          exit(EXIT_FAILURE); 
        }
    printf("Proxy crated.\n");

    proxySockAddr.sin_family = AF_INET;
    proxySockAddr.sin_port = htons(atoi(proxyport)); //ポート番号を指定
    proxySockAddr.sin_addr.s_addr = INADDR_ANY;

    // socketをbindする  
    if((bind(proxySock, (struct sockaddr*)&proxySockAddr, sizeof(proxySockAddr))) < 0) {  
        perror("bind() failed");
        exit(EXIT_FAILURE);  
    }  
    // listenする
    if((listen(proxySock, SOMAXCONN)) < 0) {  
        perror("listen() failed");  
        exit(EXIT_FAILURE);
    }  

    printf("waiting for connection..\n\n");

    while(1){
        struct args cliInfo;
        cliInfo.proxysock = proxySock;
        strcpy(cliInfo.destip, destip);
        strcpy(cliInfo.destport, destport);

        len = sizeof(cliInfo.sockAddr);
        if((cliInfo.sock = accept(proxySock, (struct sockaddr *) &cliInfo.sockAddr, &len)) < 0){
            perror("accept() failed");
            exit(EXIT_FAILURE);
        }
        
        pthread_t tid;
        pthread_create(&tid, NULL, &bridge_http_response, (void *) &cliInfo);
    }
}