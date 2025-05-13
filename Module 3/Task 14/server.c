#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#define PORT 8080
#define BUFFER_SIZE 1024

int main(){
    int sockfd;
    char buffer[BUFFER_SIZE];

    struct sockaddr_in servaddr, clientaddr;

    if((sockfd = socket(AF_INET, SOCK_DGRAM, 0)) < 0){
        perror("socket create");
        exit(EXIT_FAILURE);
    }

    memset(&servaddr, 0, sizeof(servaddr));
    memset(&clientaddr, 0, sizeof(clientaddr));

    servaddr.sin_family = AF_INET;
    servaddr.sin_port = htons(PORT);
    servaddr.sin_addr.s_addr = INADDR_ANY;

    if(bind(sockfd, (const struct sockaddr *)&servaddr), sizeof(servaddr) < 0){
        perror("bind failed");
        exit(EXIT_FAILURE);
    }
    printf("Server started.\n");
    socklen_t len;

    len = sizeof(clientaddr);

    while(1){
        int n = recvfrom(sockfd, (char *)buffer, BUFFER_SIZE, MSG_WAITALL, (struct sockaddr *)&servaddr, &len);
        buffer[n] = '\0';
        printf("Server: %s", buffer);

        if(strcmp(buffer, "exit\n") == 0){
            printf("Server end chat.")
            break;
        }

        printf("Server: ");
        fgets(buffer, BUFFER_SIZE, stdin);
        sendto(sockfd, (const char *)buffer, strlen(buffer), MSG_CONFIRM, (const struct sockaddr *)&clientaddr, len);
    }
    close(sockfd);
    return 0;
}