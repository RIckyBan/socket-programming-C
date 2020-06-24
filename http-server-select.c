
###################
 Work In Progress
###################

#include <stdio.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/select.h>
#include <sys/time.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <stdlib.h>
#include <errno.h>

#define BUFFSIZE 2048
#define LISTENQ 5


int main(int argc, char *argv[]){
    fd_set fds, readfds;
    int maxfd, numCli, servSock, cliSock;
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

    int c_sock[FD_SETSIZE]; // デフォルトは1024
    struct sockaddr_in c_sockAddr[FD_SETSIZE]; // 配列を用意

    for(int i = 0; i < FD_SETSIZE; i++){
        c_sock[i] = -1;
        struct sockaddr_in tmp;
        c_sockAddr[i] = tmp;
    }

    FD_ZERO(&readfds);
    FD_SET(servSock, &readfds); // listenソケットを監視

    struct timeval waitval;

    waitval.tv_sec = 2;
    waitval.tv_usec = 500;

    if(listen(servSock, LISTENQ) < 0){
        perror("listen() failed");
        exit(EXIT_FAILURE);
    }

    maxfd = 1;
    numCli = 0;

    while(1){
        memcpy(&fds, &readfds, sizeof(readfds));

        int n = select(maxfd, &fds, NULL, NULL, &waitval);

        if(FD_ISSET(servSock, &fds)) {
            unsigned int len;
            len = sizeof(cliSockAddr);
            cliSock = accept(servSock, (struct sockaddr *) &cliSockAddr, &len);

            if(cliSock < 0){
                perror("accept() failed");
                exit(EXIT_FAILURE);
            }

            FD_SET(cliSock, &fds); // acceptソケットを監視
            c_sock[numCli] = cliSock;
            c_sockAddr[numCli] = cliSockAddr;
            numCli++;
            maxfd++;
        }

        for(int i = 0; i < maxfd+1; ++i){
            if(FD_ISSET(c_sock[i], &fds)){
                char inbuf[BUFFSIZE], obuf[BUFFSIZE], buff[BUFFSIZE];
                memset(inbuf, 0, sizeof(inbuf));
                recv(c_sock[i], inbuf, sizeof(inbuf), 0);
                printf("%s", inbuf);

                memset(obuf, 0, sizeof(obuf));
                snprintf(obuf, sizeof(obuf),
                    "HTTP/1.0 200 OK\r\n"
                    "Content-Type: text/html\r\n"
                    "\r\n"
                    "<h1>Hello</h1>\r\n"
                );

                send(c_sock[i], obuf, (int)strlen(obuf), 0);

                printf("connected from %s, port=%d.\n",
                    (char *)inet_ntop(AF_INET, &c_sockAddr[i].sin_addr,
                    buff, sizeof(buff)),
                    ntohs(c_sockAddr[i].sin_port)
                    );

                close(c_sock[i]);
            }
        }
    }
}