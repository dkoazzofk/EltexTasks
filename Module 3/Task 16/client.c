#include <arpa/inet.h>
#include <errno.h>
#include <netinet/in.h>
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>

int sockfd;

int connection(int sockfd, struct sockaddr_in servaddr){
    if((sockfd = socket(AF_INET, SOCK_STREAM, 0)) < 0){
        perror("socket");
        exit(EXIT_FAILURE);
    }
    memset(&servaddr, 0, sizeof(servaddr));
    servaddr.sin_family = AF_INET;
    servaddr.sin_port = htons(1510);
    if(inet_pton(AF_INET, "127.0.0.1", &servaddr.sin_addr) < 1){
        perror("wrong IP");
        close(sockfd);
        exit(EXIT_FAILURE);
    }
    if(connect(sockfd, (struct sockaddr *)&servaddr, sizeof(servaddr)) < 0){
        perror("connect");
        close(sockfd);
        exit(EXIT_FAILURE);
    }
}

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

void calculate(){
    pthread_t tid;

    if(pthread_create(&tid, NULL, recvMessage, NULL) < 0){
        perror("pthread_create");
        close(sockfd);
        exit(EXIT_FAILURE);
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
    
}

long long getSize(FILE* f) {
    fseek(f, 0, SEEK_END);
    long long size = ftell(f);
    fseek(f, 0, SEEK_SET);
    return size;
}

void sendFile(){
    char path[100];
    puts("Enter file path");
    scanf("%s", path);
    FILE* f;
    if((f = fopen(path, "rb")) == NULL){
        perror("fopen");
        close(sockfd);
        exit(EXIT_FAILURE);
    }
    if(send(sockfd, path, strlen(path) + 1, 0) < 0){
        perror("send");
        close(sockfd);
        exit(EXIT_FAILURE);
    }
    long long fileSize = getSize(f);
    printf("File size = %lld\n", fileSize);
    int size = 50000;
    char* fileData = malloc(size);
    int sizeData;
    int sendBytes = 0;
    long long totalSend = 0;

    while(1){
        sizeData = fread(fileData, 1, size, f);
        if (sizeData == 0)
            break;
            
        if ((sendBytes = send(sockfd, fileData, sizeData, 0)) < 0) {
            perror("send");
            close(sockfd);
            exit(EXIT_FAILURE);
        }
        totalSend += sendBytes;
    }
}

int main(int argc, char *argv[]){
    pthread_t tid;
    struct sockaddr_in servaddr;
    connection(sockfd, servaddr);
    int choice;
    printf("Choose operation:\n 1 - Calculator\n 2 - Send File\n");
    scanf("%d", &choice);
    if(send(sockfd, &choice, sizeof(choice), 0) < 0){
        perror("send");
        close(sockfd);
        exit(EXIT_FAILURE);
    }
    switch (choice)
    {
    case 1:
        calculate();
        break;
    case 2:
        sendFile();
        break;
    default:
        close(sockfd);
        break;
    }
    exit(EXIT_SUCCESS);
}