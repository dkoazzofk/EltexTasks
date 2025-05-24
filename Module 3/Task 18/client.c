#include <arpa/inet.h>
#include <errno.h>
#include <netdb.h>
#include <netinet/in.h>
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/time.h>
#include <sys/types.h>
#include <unistd.h>

int sockfd;
void* recvMessage(void* arg){
    char mess[1024];
    while(1){
        if(read(sockfd, mess, sizeof(mess)) < 1){
            perror("recv");
            close(sockfd);
            exit(EXIT_FAILURE);
        }
        printf("\nServer to Client: %s\n", mess);
    }
    pthread_close(EXIT_SUCCESS);
}

int main(int argc, char *argv[]){
    pthread_t tid;
    struct sockaddr_in servaddr;

    if((sockfd = socket(AF_INET, SOCK_STREAM, 0)) < 0){
        perror("socket");
        exit(EXIT_FAILURE);
    }
    memset(&servaddr, 0, sizeof(servaddr));
    servaddr.sin_family = AF_INET;
    servaddr.sin_port = htons(atoi(argv[2]));
    if(inet_pton(AF_INET, argv[1], &servaddr.sin_addr) < 1){
        perror("wrong IP");
        close(sockfd);
        exit(EXIT_FAILURE);
    }
    if(connect(sockfd,(struct sockaddr *)&servaddr, sizeof(servaddr)) < 0){
        perror("connect");
        close(sockfd);
        exit(EXIT_FAILURE);
    }
    if(pthread_create(&tid, NULL, recvMessage, NULL) < 0){
        perror("pthread_create");
        close(sockfd);
    }

    char message[1024];
    while(1){
        if(fgets(message, 1024, stdin) != NULL){
            if(message[strlen(message) - 1] == '\n'){
                message[strlen(message) - 1] == '\0';
            }
        }
        if(!strcmp(message, "q")){
            printf("Exit...");
            if(pthread_cancel(tid) != 0){
                perror("pthread_cancel");
                exit(EXIT_FAILURE);
            }
            break;
        }
        if(send(sockfd, message, strlen(message) + 1, 0) < 0){
            perror("send");
            exit(EXIT_FAILURE);
        }
    }
    if(pthread_join(tid, NULL) != 0){
        perror("pthread_join");
        exit(EXIT_FAILURE);
    }
    close(sockfd);
    exit(EXIT_SUCCESS);
}